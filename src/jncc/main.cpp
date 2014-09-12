#include "pch.h"
#include "Jnc.h"
#include "Version.h"

#define _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS

//.............................................................................

void
PrintVersion (COutStream* pOutStream)
{
	pOutStream->Printf (
		"Jancy (%s) v%d.%d.%d\n",
		_AXL_CPU_STRING,
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_REVISION
		);
}

void
PrintUsage (COutStream* pOutStream)
{
	PrintVersion (pOutStream);

	rtl::CString HelpString = CCmdLineSwitchTable::GetHelpString ();
	pOutStream->Printf ("Usage: jancy [<options>...] <source_file>\n%s", HelpString.cc ());
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
