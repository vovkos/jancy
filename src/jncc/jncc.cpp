#include "pch.h"
#include "jncc.h"

#define _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS

//.............................................................................

void
PrintVersion (COutStream* pOutStream)
{
	pOutStream->Printf (
		"Jancy compiler (%s) v%d.%d.%d\n",
		_AXL_CPU_STRING,
		HIBYTE (HIWORD (EJnc_CompilerVersion)),
		LOBYTE (HIWORD (EJnc_CompilerVersion)),
		LOWORD (EJnc_CompilerVersion)
		);
}

void
PrintUsage (COutStream* pOutStream)
{
	PrintVersion (pOutStream);

	rtl::CString HelpString = CCmdLineSwitchTable::GetHelpString ();
	pOutStream->Printf ("Usage: jnc [<options>...] <source_file>\n%s", HelpString.cc ());
}

//.............................................................................

CJnc* CJnc::m_pCurrentJnc = NULL;

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

//.............................................................................

#if (_AXL_ENV == AXL_ENV_WIN)
int
wmain (
	int argc,
	wchar_t* argv []
	)
#else
int
main (
	int argc,
	char* argv []
	)
#endif
{
	bool Result;

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::CParseErrorProvider::Register ();
	srand ((int) axl::g::GetTimestamp ());

	CFileOutStream StdOut;
	TCmdLine CmdLine;
	CCmdLineParser Parser (&CmdLine);

#ifdef _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS
	if (argc < 2)
	{
		PrintUsage (&StdOut);
		return 0;
	}
#endif

	Result = Parser.Parse (argc, argv);
	if (!Result)
	{
		printf ("error parsing command line: %s\n", err::GetError ()->GetDescription ().cc ());
		return EJncError_InvalidCmdLine;
	}

	CJnc Jnc;
	return Jnc.Run (&CmdLine, &StdOut);
}

//.............................................................................
