#include "pch.h"
#include "Jnc.h"

//.............................................................................

int
StdLib::printf (
	const char* format,
	...
	)
{
	AXL_VA_DECL (va, format);

	Jnc* jnc = Jnc::getCurrentJnc ();
	ASSERT (jnc);

	return jnc->getOutStream ()->printf_va (format, va);
}

//.............................................................................

Jnc* Jnc::m_currentJnc = NULL;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
Jnc::run (
	CmdLine* cmdLine,
	OutStream* outStream
	)
{
	bool result;

	m_cmdLine = cmdLine;
	m_outStream = outStream;

	if (cmdLine->m_flags & JncFlag_Help)
	{
		printUsage (outStream);
		return JncErrorKind_Success;
	}

	if (cmdLine->m_flags & JncFlag_Version)
	{
		printVersion (outStream);
		return JncErrorKind_Success;
	}

	if (cmdLine->m_flags & JncFlag_Server)
		return server ();

	rtl::Array <char> stdInBuffer;
	io::SimpleMappedFile srcFile;

	rtl::String srcName;
	const char* src;
	size_t srcSize;

	if (cmdLine->m_flags & JncFlag_StdInSrc)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		int stdInFile = _fileno (stdin);
#endif
		for (;;)
		{
			char buffer [1024];
#if (_AXL_ENV == AXL_ENV_WIN)
			int result = _read (stdInFile, buffer, sizeof (buffer));
#else
			int result = read (STDIN_FILENO, buffer, sizeof (buffer));
#endif
			if (result <= 0)
				break;

			stdInBuffer.append (buffer, result);
		}

		src = stdInBuffer;
		srcSize = stdInBuffer.getCount ();
		srcName = !m_cmdLine->m_srcNameOverride.isEmpty () ?
			m_cmdLine->m_srcNameOverride :
			"stdin";
	}
	else if (!cmdLine->m_srcFileName.isEmpty ())
	{
		result = srcFile.open (cmdLine->m_srcFileName, io::FileFlag_ReadOnly);
		if (!result)
		{
			outStream->printf (
				"cannot open '%s': %s\n",
				cmdLine->m_srcFileName.cc (), // thanks a lot gcc
				err::getLastError ()->getDescription ().cc ()
				);
			return JncErrorKind_IoFailure;
		}

		src = (const char*) srcFile.p ();
		srcSize = (size_t) srcFile.getSize ();
		srcName = !m_cmdLine->m_srcNameOverride.isEmpty () ?
			m_cmdLine->m_srcNameOverride :
			io::getFullFilePath (cmdLine->m_srcFileName);
	}
	else
	{
		outStream->printf ("missing input (required source-file-name or --stdin)\n");
		return JncErrorKind_InvalidCmdLine;
	}

	if (cmdLine->m_flags & JncFlag_RunFunction)
		cmdLine->m_flags |= JncFlag_Jit;

	result = compile (srcName, src, srcSize);
	if (!result)
	{
		outStream->printf ("%s\n", err::getLastError ()->getDescription ().cc ());
		return JncErrorKind_CompileFailure;
	}

	if (cmdLine->m_flags & JncFlag_LlvmIr)
		printLlvmIr ();

	if (m_cmdLine->m_flags & JncFlag_Jit)
	{
		result = jit ();
		if (!result)
		{
			outStream->printf ("%s\n", err::getLastError ()->getDescription ().cc ());
			return JncErrorKind_CompileFailure;
		}
	}

	if (cmdLine->m_flags & JncFlag_RunFunction)
	{
		m_runtime.m_stackSizeLimit = cmdLine->m_stackSizeLimit;
		m_runtime.m_gcHeap.m_periodSizeLimit = cmdLine->m_gcPeriodSizeLimit;

		int returnValue;
		result = runFunction (&returnValue);
		if (!result)
		{
			outStream->printf ("%s\n", err::getLastError ()->getDescription ().cc ());
			return JncErrorKind_RunFailure;
		}

		outStream->printf ("'%s' returned (%d).\n", cmdLine->m_functionName.cc (), returnValue);
	}

	return JncErrorKind_Success;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Jnc::compile (
	const char* fileName,
	const char* source,
	size_t length
	)
{
	bool result;

	uint_t moduleFlags = 0;
	if (m_cmdLine->m_flags & JncFlag_DebugInfo)
		moduleFlags |= jnc::ModuleFlag_DebugInfo;

	if (m_cmdLine->m_flags & JncFlag_Jit_mc)
		moduleFlags |= jnc::ModuleFlag_McJit;

	m_module.create ("jncc_module", moduleFlags);

	result =
		m_module.parse (fileName, source, length) &&
		m_module.compile ();

	if (!result)
		return false;

	return true;
}

bool
Jnc::jit ()
{
	return
		m_module.createLlvmExecutionEngine () &&
		StdLib::mapFunctions (&m_module) &&
		m_module.jit ();
}

void
Jnc::printLlvmIr ()
{
	if (!(m_cmdLine->m_flags & JncFlag_LlvmIr_c))
	{
		m_outStream->printf ("%s", m_module.getLlvmIrString ().cc ());
		return;
	}

	rtl::Iterator <jnc::Function> function = m_module.m_functionMgr.getFunctionList ().getHead ();
	for (; function; function++)
	{
		jnc::FunctionType* functionType = function->getType ();

		m_outStream->printf ("%s %s %s %s\n",
			functionType->getReturnType ()->getTypeString ().cc (),
			functionType->getCallConv ()->getCallConvString (),
			function->m_tag.cc (),
			functionType->getArgString ().cc ()
			);

		llvm::Function* llvmFunction = function->getLlvmFunction ();
		llvm::Function::BasicBlockListType& blockList = llvmFunction->getBasicBlockList ();
		llvm::Function::BasicBlockListType::iterator block = blockList.begin ();

		for (; block != blockList.end (); block++)
		{
			std::string name = block->getName ();
			m_outStream->printf ("%s:\n", name.c_str ());

			llvm::BasicBlock::InstListType& instList = block->getInstList ();
			llvm::BasicBlock::InstListType::iterator inst = instList.begin ();
			for (; inst != instList.end (); inst++)
			{
				std::string string;
				llvm::raw_string_ostream stream (string);
				inst->print (stream);
				m_outStream->printf ("%s\n", string.c_str ());
			}
		}

		m_outStream->printf ("\n........................................\n\n");
	}
}

bool
Jnc::runFunction (int* returnValue)
{
	bool result;

	jnc::ModuleItem* functionItem = m_module.m_namespaceMgr.getGlobalNamespace ()->findItem (m_cmdLine->m_functionName);
	if (!functionItem || functionItem->getItemKind () != jnc::ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not found or not a function\n", m_cmdLine->m_functionName.cc ());
		return false;
	}

	jnc::Function* function = (jnc::Function*) functionItem;

	result = m_runtime.startup (&m_module);
	if (!result)
		return false;

	result = jnc::callFunction (&m_runtime, function, returnValue);
	if (!result)
		return false;

	m_runtime.shutdown ();

	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
Jnc::server ()
{
	int result;

	printVersion (m_outStream);

#if (_AXL_ENV == AXL_ENV_WIN)
	WSADATA wsaData;
	WSAStartup (0x0202, &wsaData);
#endif

	SOCKET serverSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT (serverSock != INVALID_SOCKET);

	sockaddr_in sockAddr = { 0 };
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons (m_cmdLine->m_serverPort);

	result = bind (serverSock, (sockaddr*) &sockAddr, sizeof (sockAddr));
	if (result == -1)
	{
		m_outStream->printf (
			"cannot open TCP port %d (%s)\n",
			m_cmdLine->m_serverPort,
			err::Error (getsockerror ()).getDescription ().cc ()
			);

		return JncErrorKind_IoFailure;
	}

	listen (serverSock, 1);

	m_outStream->printf ("listening on TCP port %d...\n", m_cmdLine->m_serverPort);

	for (;;)
	{
		socklen_t sockAddrSize = sizeof (sockAddr);

		SOCKET connSock = accept (serverSock, (sockaddr*) &sockAddr, &sockAddrSize);
		m_outStream->printf (
			"%s:%d: connection accepted\n",
			inet_ntoa (sockAddr.sin_addr),
			ntohs (sockAddr.sin_port)
			);

		client (connSock, &sockAddr);
		closesocket (connSock);
	}

	closesocket (serverSock);
	return 0;
}

int
Jnc::client (
	SOCKET socket,
	sockaddr_in* sockAddr
	)
{
	int result;

	rtl::String cmdLineString;

	for (;;)
	{
		char buffer [256];
		result = recv (socket, buffer, sizeof (buffer), 0);
		if (result <= 0)
		{
			m_outStream->printf (
				"%s:%d: premature connection close\n",
				inet_ntoa (sockAddr->sin_addr),
				ntohs (sockAddr->sin_port)
				);
			return JncErrorKind_IoFailure;
		}

		char* p = strnchr (buffer, result, '\n');
		if (p)
		{
			*p = '\0';
			cmdLineString.append (buffer, p - buffer);
			break;
		}

		cmdLineString.append (buffer, result);
	}

	m_outStream->printf (
		"%s:%d: cmdline: %s\n",
		inet_ntoa (sockAddr->sin_addr),
		ntohs (sockAddr->sin_port),
		cmdLineString.cc ()
		);

	SocketOutStream socketOut;
	socketOut.m_socket = socket;

	CmdLine cmdLine;
	CmdLineParser parser (&cmdLine);

	result = parser.parse (cmdLineString, cmdLineString.getLength ());
	if (!result)
	{
		socketOut.printf ("error parsing command line: %s\n", err::getLastError ()->getDescription ().cc ());
		return JncErrorKind_InvalidCmdLine;
	}

	if (cmdLine.m_flags & JncFlag_Server)
	{
		socketOut.printf ("recursive server request\n");
		return JncErrorKind_InvalidCmdLine;
	}

	Jnc jnc;
	return jnc.run (&cmdLine, &socketOut);
}

//.............................................................................

