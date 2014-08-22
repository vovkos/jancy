#include "pch.h"
#include "Jnc.h"

//.............................................................................

int
CStdLib::Printf (
	const char* pFormat,
	...
	)
{
	AXL_VA_DECL (va, pFormat);

	CJnc* pJnc = CJnc::GetCurrentJnc ();
	ASSERT (pJnc);

	return pJnc->GetOutStream ()->Printf_va (pFormat, va);
}

//.............................................................................

CJnc* CJnc::m_pCurrentJnc = NULL;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
CJnc::Run (
	TCmdLine* pCmdLine,
	COutStream* pOutStream
	)
{
	bool Result;

	m_pCmdLine = pCmdLine;
	m_pOutStream = pOutStream;

	if (pCmdLine->m_Flags & EJncFlag_Help)
	{
		PrintUsage (pOutStream);
		return EJncError_Success;
	}

	if (pCmdLine->m_Flags & EJncFlag_Version)
	{
		PrintVersion (pOutStream);
		return EJncError_Success;
	}

	if (pCmdLine->m_Flags & EJncFlag_Server)
		return Server ();

	rtl::CArrayT <char> StdInBuffer;
	io::CMappedFile SrcFile;

	rtl::CString SrcName;
	const char* pSrc;
	size_t SrcSize;

	if (pCmdLine->m_Flags & EJncFlag_StdInSrc)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		int StdInFile = _fileno (stdin);
#endif
		for (;;)
		{
			char Buffer [1024];
#if (_AXL_ENV == AXL_ENV_WIN)
			int Result = _read (StdInFile, Buffer, sizeof (Buffer));
#else
			int Result = read (STDIN_FILENO, Buffer, sizeof (Buffer));
#endif
			if (Result <= 0)
				break;

			StdInBuffer.Append (Buffer, Result);
		}

		pSrc = StdInBuffer;
		SrcSize = StdInBuffer.GetCount ();
		SrcName = !m_pCmdLine->m_SrcNameOverride.IsEmpty () ?
			m_pCmdLine->m_SrcNameOverride :
			"stdin";
	}
	else if (!pCmdLine->m_SrcFileName.IsEmpty ())
	{
		Result = SrcFile.Open (pCmdLine->m_SrcFileName, io::EFileFlag_ReadOnly);
		if (!Result)
		{
			pOutStream->Printf (
				"cannot open '%s': %s\n",
				pCmdLine->m_SrcFileName.cc (), // thanks a lot gcc
				err::GetError ()->GetDescription ().cc ()
				);
			return EJncError_IoFailure;
		}

		pSrc = (const char*) SrcFile.View ();
		SrcSize = (size_t) SrcFile.GetSize ();
		SrcName = !m_pCmdLine->m_SrcNameOverride.IsEmpty () ?
			m_pCmdLine->m_SrcNameOverride :
			io::GetFullFilePath (pCmdLine->m_SrcFileName);
	}
	else
	{
		pOutStream->Printf ("missing input (required source-file-name or --stdin)\n");
		return EJncError_InvalidCmdLine;
	}

	if (pCmdLine->m_Flags & (EJncFlag_RunFunction | EJncFlag_Disassembly))
		pCmdLine->m_Flags |= EJncFlag_Jit;

	Result = Compile (SrcName, pSrc, SrcSize);
	if (!Result)
	{
		pOutStream->Printf ("%s\n", err::GetError ()->GetDescription ().cc ());
		return EJncError_CompileFailure;
	}

	if (pCmdLine->m_Flags & EJncFlag_LlvmIr)
		PrintLlvmIr ();

	if (m_pCmdLine->m_Flags & EJncFlag_Jit)
	{
		Result = Jit ();
		if (!Result)
		{
			pOutStream->Printf ("%s\n", err::GetError ()->GetDescription ().cc ());
			return EJncError_CompileFailure;
		}
	}

	if (pCmdLine->m_Flags & EJncFlag_Disassembly)
		PrintDisassembly ();

	if (pCmdLine->m_Flags & EJncFlag_RunFunction)
	{
		int ReturnValue;
		Result = RunFunction (&ReturnValue);
		if (!Result)
		{
			pOutStream->Printf ("%s\n", err::GetError ()->GetDescription ().cc ());
			return EJncError_RunFailure;
		}

		pOutStream->Printf ("'%s' returned (%d).\n", pCmdLine->m_FunctionName.cc (), ReturnValue);
	}

	return EJncError_Success;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CJnc::Compile (
	const char* pFileName,
	const char* pSource,
	size_t Length
	)
{
	bool Result;

	uint_t ModuleFlags = 0;
	if (m_pCmdLine->m_Flags & EJncFlag_DebugInfo)
		ModuleFlags |= jnc::EModuleFlag_DebugInfo;

	if (m_pCmdLine->m_Flags & EJncFlag_Jit_mc)
		ModuleFlags |= jnc::EModuleFlag_McJit;

	m_Module.Create ("jncc_module", ModuleFlags);

	jnc::CScopeThreadModule ScopeModule (&m_Module);

	Result =
		m_Module.Parse (pFileName, pSource, Length) &&
		m_Module.Compile ();

	if (!Result)
		return false;

	return true;
}

bool
CJnc::Jit ()
{
	return
		m_Module.CreateLlvmExecutionEngine () &&
		CStdLib::Export (&m_Module) &&
		m_Module.Jit () &&
		m_Runtime.Create (m_pCmdLine->m_GcHeapSize, m_pCmdLine->m_StackSize) &&
		m_Runtime.AddModule (&m_Module);
}

void
CJnc::PrintLlvmIr ()
{
	if (!(m_pCmdLine->m_Flags & EJncFlag_LlvmIr_c))
	{
		m_pOutStream->Printf ("%s", m_Module.GetLlvmIrString ().cc ());
		return;
	}

	uint_t CommentMdKind = m_Module.m_LlvmIrBuilder.GetCommentMdKind ();

	rtl::CIteratorT <jnc::CFunction> Function = m_Module.m_FunctionMgr.GetFunctionList ().GetHead ();
	for (; Function; Function++)
	{
		jnc::CFunctionType* pFunctionType = Function->GetType ();

		m_pOutStream->Printf ("%s %s %s %s\n",
			pFunctionType->GetReturnType ()->GetTypeString ().cc (),
			pFunctionType->GetCallConv ()->GetCallConvString (),
			Function->m_Tag.cc (),
			pFunctionType->GetArgString ().cc ()
			);

		llvm::Function* pLlvmFunction = Function->GetLlvmFunction ();
		llvm::Function::BasicBlockListType& BlockList = pLlvmFunction->getBasicBlockList ();
		llvm::Function::BasicBlockListType::iterator Block = BlockList.begin ();

		for (; Block != BlockList.end (); Block++)
		{
			std::string Name = Block->getName ();
			m_pOutStream->Printf ("%s:\n", Name.c_str ());

			llvm::BasicBlock::InstListType& InstList = Block->getInstList ();
			llvm::BasicBlock::InstListType::iterator Inst = InstList.begin ();
			for (; Inst != InstList.end (); Inst++)
			{
				std::string String;
				llvm::raw_string_ostream Stream (String);

				llvm::MDNode* pMdComment = Inst->getMetadata (CommentMdKind);
				if (pMdComment)
					Inst->setMetadata (CommentMdKind, NULL); // remove before print

				Inst->print (Stream);

				m_pOutStream->Printf ("%s\n", String.c_str ());

				if (pMdComment)
				{
					Inst->setMetadata (CommentMdKind, pMdComment); // restore
					llvm::MDString* pMdString = (llvm::MDString*) pMdComment->getOperand (0);
					m_pOutStream->Printf ("\n  ; %s\n", pMdString->getString ().data ());
				}
			}
		}

		m_pOutStream->Printf ("\n........................................\n\n");
	}
}

void
CJnc::PrintDisassembly ()
{
	jnc::CDisassembler Dasm;

	rtl::CIteratorT <jnc::CFunction> Function = m_Module.m_FunctionMgr.GetFunctionList ().GetHead ();
	for (; Function; Function++)
	{
		jnc::CFunctionType* pFunctionType = Function->GetType ();

		m_pOutStream->Printf (
			"%s %s %s %s\n",
			pFunctionType->GetReturnType ()->GetTypeString ().cc (),
			pFunctionType->GetCallConv ()->GetCallConvString (),
			Function->m_Tag.cc (),
			pFunctionType->GetArgString ().cc ()
			);

		void* pf = Function->GetMachineCode ();
		size_t Size = Function->GetMachineCodeSize ();

		if (pf)
		{
			rtl::CString s = Dasm.Disassemble (pf, Size);
			m_pOutStream->Printf ("\n%s", s.cc ());
		}

		m_pOutStream->Printf ("\n........................................\n\n");
	}
}

bool
CJnc::RunFunction (
	jnc::CFunction* pFunction,
	int* pReturnValue
	)
{
	typedef
	int
	FFunction ();

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		int ReturnValue = pf ();
		if (pReturnValue)
			*pReturnValue = ReturnValue;
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

bool
CJnc::RunFunction (int* pReturnValue)
{
	bool Result;

	jnc::CModuleItem* pFunctionItem = m_Module.m_NamespaceMgr.GetGlobalNamespace ()->FindItem (m_pCmdLine->m_FunctionName);
	if (!pFunctionItem || pFunctionItem->GetItemKind () != jnc::EModuleItem_Function)
	{
		err::SetFormatStringError ("'%s' is not found or not a function\n", m_pCmdLine->m_FunctionName.cc ());
		return false;
	}

	jnc::CFunction* pFunction = (jnc::CFunction*) pFunctionItem;

	jnc::CScopeThreadRuntime ScopeRuntime (&m_Runtime);

	m_Runtime.Startup ();

	jnc::CFunction* pConstructor = m_Module.GetConstructor ();
	if (pConstructor)
	{
		Result = RunFunction (pConstructor);
		if (!Result)
			return false;
	}

	Result = RunFunction (pFunction, pReturnValue);
	if (!Result)
		return false;

	jnc::CFunction* pDestructor = m_Module.GetDestructor ();
	if (pDestructor)
	{
		Result = RunFunction (pDestructor);
		if (!Result)
			return false;
	}

	m_Runtime.Shutdown ();

	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
CJnc::Server ()
{
	int Result;

	PrintVersion (m_pOutStream);

#if (_AXL_ENV == AXL_ENV_WIN)
	WSADATA WsaData;
	WSAStartup (0x0202, &WsaData);
#endif

	SOCKET ServerSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT (ServerSock != INVALID_SOCKET);

	sockaddr_in SockAddr = { 0 };
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons (m_pCmdLine->m_ServerPort);

	Result = bind (ServerSock, (sockaddr*) &SockAddr, sizeof (SockAddr));
	if (Result == -1)
	{
		m_pOutStream->Printf (
			"cannot open TCP port %d (%s)\n",
			m_pCmdLine->m_ServerPort,
			err::CError (getsockerror ()).GetDescription ().cc ()
			);

		return EJncError_IoFailure;
	}

	listen (ServerSock, 1);

	m_pOutStream->Printf ("listening on TCP port %d...\n", m_pCmdLine->m_ServerPort);

	for (;;)
	{
		socklen_t SockAddrSize = sizeof (SockAddr);

		SOCKET ConnSock = accept (ServerSock, (sockaddr*) &SockAddr, &SockAddrSize);
		m_pOutStream->Printf (
			"%s:%d: connection accepted\n",
			inet_ntoa (SockAddr.sin_addr),
			ntohs (SockAddr.sin_port)
			);

		Client (ConnSock, &SockAddr);
		closesocket (ConnSock);
	}

	closesocket (ServerSock);
	return 0;
}

int
CJnc::Client (
	SOCKET Socket,
	sockaddr_in* pSockAddr
	)
{
	int Result;

	rtl::CString CmdLineString;

	for (;;)
	{
		char Buffer [256];
		Result = recv (Socket, Buffer, sizeof (Buffer), 0);
		if (Result <= 0)
		{
			m_pOutStream->Printf (
				"%s:%d: premature connection close\n",
				inet_ntoa (pSockAddr->sin_addr),
				ntohs (pSockAddr->sin_port)
				);
			return EJncError_IoFailure;
		}

		char* p = strnchr (Buffer, Result, '\n');
		if (p)
		{
			*p = '\0';
			CmdLineString.Append (Buffer, p - Buffer);
			break;
		}

		CmdLineString.Append (Buffer, Result);
	}

	m_pOutStream->Printf (
		"%s:%d: cmdline: %s\n",
		inet_ntoa (pSockAddr->sin_addr),
		ntohs (pSockAddr->sin_port),
		CmdLineString.cc ()
		);

	CSocketOutStream SocketOut;
	SocketOut.m_Socket = Socket;

	TCmdLine CmdLine;
	CCmdLineParser Parser (&CmdLine);

	Result = Parser.Parse (CmdLineString, CmdLineString.GetLength ());
	if (!Result)
	{
		SocketOut.Printf ("error parsing command line: %s\n", err::GetError ()->GetDescription ().cc ());
		return EJncError_InvalidCmdLine;
	}

	if (CmdLine.m_Flags & EJncFlag_Server)
	{
		SocketOut.Printf ("recursive server request\n");
		return EJncError_InvalidCmdLine;
	}

	CJnc Jnc;
	return Jnc.Run (&CmdLine, &SocketOut);
}

//.............................................................................

