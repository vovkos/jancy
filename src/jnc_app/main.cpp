#include "pch.h"
#include "JncApp.h"
#include "CmdLine.h"
#include "version.h"

#define _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS

//.............................................................................

enum JncError
{
	JncError_Success         = 0,
	JncError_InvalidCmdLine  = -1,
	JncError_IoFailure       = -2,
	JncError_CompileFailure  = -3,
	JncError_RunFailure      = -4,
};

//.............................................................................

void
printVersion ()
{
	printf (
		"Jancy v%d.%d.%d (%s%s)\n",
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_REVISION,
		_AXL_CPU_STRING,
		_AXL_DEBUG_SUFFIX
		);
}

void
printUsage ()
{
	printVersion ();

	sl::String helpString = CmdLineSwitchTable::getHelpString ();
	printf ("Usage: jancy [<options>...] <source_file>\n%s", helpString.cc ());
}

//.............................................................................

#include <iostream>

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

	jnc::initialize ();

	srand ((int) sys::getTimestamp ());

	CmdLine cmdLine;
	CmdLineParser parser (&cmdLine);

#ifdef _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS
	if (argc < 2)
	{
		printUsage ();
		return 0;
	}
#endif

	result = parser.parse (argc, argv);
	if (!result)
	{
		printf ("error parsing command line: %s\n", err::getLastErrorDescription ().cc ());
		return JncError_InvalidCmdLine;
	}

	if (cmdLine.m_flags & JncFlag_Help)
	{
		printUsage ();
	}
	else if (cmdLine.m_flags & JncFlag_Version)
	{
		printVersion ();
	}
	else
	{
		JncApp app (&cmdLine);

		result = app.parse ();
		if (!result)
		{
			printf ("%s\n", err::getLastErrorDescription ().cc ());
			return JncError_CompileFailure;
		}

		if (cmdLine.m_flags & JncFlag_Documentation)
		{
			result = app.generateDocumentation ();
			if (!result)
			{
				printf ("%s\n", err::getLastErrorDescription ().cc ());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_Compile)
		{
			result = app.compile ();
			if (!result)
			{
				printf ("%s\n", err::getLastErrorDescription ().cc ());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_LlvmIr)
			app.printLlvmIr ();

		if (cmdLine.m_flags & JncFlag_Jit)
		{
			result = app.jit ();
			if (!result)
			{
				printf ("%s\n", err::getLastErrorDescription ().cc ());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_Run)
		{
			int returnValue;
			result = app.runFunction (&returnValue);
			if (!result)
			{
				printf ("%s\n", err::getLastErrorDescription ().cc ());
				return JncError_RunFailure;
			}

			if (!(cmdLine.m_flags & JncFlag_PrintReturnValue))
				return returnValue;

			printf ("'%s' returned: %d\n", cmdLine.m_functionName.cc (), returnValue);
		}
	}

	return JncError_Success;
}

//.............................................................................
