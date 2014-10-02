#include "pch.h"
#include "Jnc.h"
#include "Version.h"

#define _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS

//.............................................................................

void
printVersion (OutStream* outStream)
{
	outStream->printf (
		"Jancy (%s) v%d.%d.%d\n",
		_AXL_CPU_STRING,
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_REVISION
		);
}

void
printUsage (OutStream* outStream)
{
	printVersion (outStream);

	rtl::String helpString = CmdLineSwitchTable::getHelpString ();
	outStream->printf ("Usage: jancy [<options>...] <source_file>\n%s", helpString.cc ());
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
	bool result;

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::registerParseErrorProvider ();
	srand ((int) axl::g::getTimestamp ());

	FileOutStream stdOut;
	CmdLine cmdLine;
	CmdLineParser parser (&cmdLine);

#ifdef _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS
	if (argc < 2)
	{
		printUsage (&stdOut);
		return 0;
	}
#endif

	result = parser.parse (argc, argv);
	if (!result)
	{
		printf ("error parsing command line: %s\n", err::getError ()->getDescription ().cc ());
		return JncErrorKind_InvalidCmdLine;
	}

	Jnc jnc;
	return jnc.run (&cmdLine, &stdOut);
}

//.............................................................................
