//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "JncApp.h"
#include "CmdLine.h"
#include "version.h"

#define _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS

//..............................................................................

enum JncError {
	JncError_Success         = 0,
	JncError_InvalidCmdLine  = -1,
	JncError_IoFailure       = -2,
	JncError_CompileFailure  = -3,
	JncError_RunFailure      = -4,
};

//..............................................................................

void
printVersion() {
	printf(
		"Jancy v%d.%d.%d (%s%s)\n",
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_REVISION,
		AXL_CPU_STRING,
		AXL_DEBUG_SUFFIX
	);
}

void
printUsage() {
	printVersion();

	sl::String helpString = CmdLineSwitchTable::getHelpString();
	printf("Usage: jancy [<options>...] <source_file>\n%s", helpString.sz());
}

#if (_JNC_OS_WIN)
int
wmain(
	int argc,
	wchar_t* argv[]
)
#else
int
main(
	int argc,
	char* argv[]
)
#endif
{
#if _AXL_OS_POSIX
	setvbuf(stdout, NULL, _IOLBF, 1024);
#endif

	bool result;

	jnc::initialize();
	jnc::setErrorRouter(err::getErrorMgr());
	lex::registerParseErrorProvider();
	sys::registerExceptionErrorProvider();

	int seed = (int)sys::getTimestamp();
	srand(seed);

	// fprintf(stderr, "srand: 0x%x\n", seed);

	CmdLine cmdLine;
	CmdLineParser parser(&cmdLine);

#ifdef _JNCC_PRINT_USAGE_IF_NO_ARGUMENTS
	if (argc < 2) {
		printUsage();
		return 0;
	}
#endif

	result = parser.parse(argc, argv);
	if (!result) {
		fprintf(stderr, "error parsing command line: %s\n", err::getLastErrorDescription().sz());
		return JncError_InvalidCmdLine;
	}

	if (cmdLine.m_flags & JncFlag_Help) {
		printUsage();
	} else if (cmdLine.m_flags & JncFlag_Version) {
		printVersion();
	} else {
		JncApp app(&cmdLine);

		uint64_t time0;

		time0 = sys::getTimestamp();
		result = app.parse();
		printf("parse: %s\n", sys::Time(sys::getTimestamp() - time0).format("%s.%l").sz());

		if (!result) {
			fprintf(stderr, "%s\n", err::getLastErrorDescription().sz());
			return JncError_CompileFailure;
		}

		if (cmdLine.m_moduleConfig.m_compileFlags & jnc::ModuleCompileFlag_Documentation) {
			result = app.generateDocumentation();
			if (!result) {
				printf("%s\n", err::getLastErrorDescription().sz());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_Compile) {
			time0 = sys::getTimestamp();
			result = app.compile();
			printf("compile: %s\n", sys::Time(sys::getTimestamp() - time0).format("%s.%l").sz());

			if (!result) {
				fprintf(stderr, "%s\n", err::getLastErrorDescription().sz());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_LlvmIr) {
			time0 = sys::getTimestamp();
			app.printLlvmIr();
			printf("print LLVM IR: %s\n", sys::Time(sys::getTimestamp() - time0).format("%s.%l").sz());
		}

		if (cmdLine.m_flags & JncFlag_Jit) {
			time0 = sys::getTimestamp();
			result = app.jit();
			printf("JIT: %s\n", sys::Time(sys::getTimestamp() - time0).format("%s.%l").sz());

			if (!result) {
				fprintf(stderr, "%s\n", err::getLastErrorDescription().sz());
				return JncError_CompileFailure;
			}
		}

		if (cmdLine.m_flags & JncFlag_Run) {
			int returnValue;
			result = app.runFunction(&returnValue);
			if (!result) {
				printf("%s\n", err::getLastErrorDescription().sz());
				return JncError_RunFailure;
			}

			if (!(cmdLine.m_flags & JncFlag_PrintReturnValue))
				return returnValue;

			printf("'%s' returned: %d\n", cmdLine.m_functionName.sz(), returnValue);
		}
	}

	return JncError_Success;
}

//..............................................................................
