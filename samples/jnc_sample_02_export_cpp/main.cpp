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
#include "MyLib.h"

char g_script[] =
#include "script.jnc.cpp"
;

//..............................................................................

enum Error {
	Error_Success = 0,
	Error_CmdLine = -1,
	Error_Io      = -2,
	Error_Compile = -3,
	Error_Runtime = -4,
};

//..............................................................................

#if (_JNC_OS_WIN)

std::unique_ptr<char>
convertToUtf8(
	const wchar_t* string,
	size_t length = -1
) {
	int requiredLength = ::WideCharToMultiByte(CP_UTF8, 0, string, (int)length, NULL, 0, NULL, NULL);
	if (!requiredLength)
		return std::unique_ptr<char>();

	char* p = new char[requiredLength + 1];
	p[requiredLength] = 0; // ensure zero-termination

	::WideCharToMultiByte(CP_UTF8, 0, string, (int)length, p, requiredLength, NULL, NULL);
	return std::unique_ptr<char>(p);
}

#endif

//..............................................................................

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
	bool result;

	printf("Initializing...\n");
	jnc::initialize();
	jnc::AutoModule module;

	module->initialize("jnc_sample_02_export_cpp");
	module->addStaticLib(jnc::StdLib_getLib());
	module->addStaticLib(MyLib_getLib());
	module->require(jnc::ModuleItemKind_Function, "main");

	if (argc < 2) {
		printf("Parsing default script...\n");
		result = module->parse("script.jnc", g_script, sizeof(g_script) - 1);
	} else {
#if (_JNC_OS_WIN)
		std::unique_ptr<char> fileName_utf8 = convertToUtf8(argv[1]);
		const char* fileName = fileName_utf8.get();
#else
		const char* fileName = argv[1];
#endif
		printf("Parsing '%s'...\n", fileName);
		result = module->parseFile(fileName);
	}

	result = result && module->parseImports();
	if (!result) {
		printf("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	printf("Compiling & JITting...\n");

	result =
		module->compile() &&
		module->optimize() &&
		module->jit();

	if (!result) {
		printf("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Compile;
	}

	jnc::Namespace* nspace = module->getGlobalNamespace()->getNamespace();
	jnc::Function* mainFunction = (jnc::Function*)nspace->findItem("main").m_item;
	JNC_ASSERT(mainFunction && mainFunction->getItemKind() == jnc::ModuleItemKind_Function);

	printf("Running...\n");

	jnc::AutoRuntime runtime;

	result = runtime->startup(module);
	if (!result) {
		printf("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	Error finalResult = Error_Success;

	int returnValue;
	result = jnc::callFunction(runtime, mainFunction, &returnValue);
	if (!result) {
		printf("Runtime error: %s\n", jnc::getLastErrorDescription_v ());
		finalResult = Error_Runtime;
	}

	printf("Shutting down...\n");

	runtime->shutdown();

	printf("Done.\n");

	return finalResult;
}

//..............................................................................
