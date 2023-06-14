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
#include "test.h"

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

	printf("Initializing...\n");

	if (argc < 2) {
		printf("usage: jnc_test_abi <script.jnc>\n");
		return -1;
	}

#if (_JNC_OS_WIN)
	sl::String sourceFileName = argv[1]; // utf16 -> utf8
#else
	const char* sourceFileName = argv[1];
#endif

	bool result;

	jnc::initialize();
	jnc::setErrorRouter(err::getErrorMgr());
	lex::registerParseErrorProvider();
	sys::registerExceptionErrorProvider();

	srand((int)sys::getTimestamp());

	jnc::AutoModule module;
	jnc::AutoRuntime runtime;

	module->initialize("jnc_module");
	module->addStaticLib(jnc::StdLib_getLib());
	module->addStaticLib(TestLib_getLib());

	static const char* requiredFuncSet[] = {
		"c2jnc.funcInt32",
		"c2jnc.funcInt64",
		"c2jnc.funcStruct32",
		"c2jnc.funcStruct64",
		"c2jnc.funcStruct128",
		"c2jnc.funcVariant",
		"c2jnc.funcFloat",
		"c2jnc.funcDouble",
		"jnc2c.testInt32",
		"jnc2c.testInt64",
		"jnc2c.testStruct32",
		"jnc2c.testStruct64",
		"jnc2c.testStruct128",
		"jnc2c.testVariant",
		"jnc2c.testPtr",
		"jnc2c.testFloat",
		"jnc2c.testDouble",
	};

	for (size_t i = 0; i < countof(requiredFuncSet); i++)
		module->require(jnc::ModuleItemKind_Function, requiredFuncSet[i]);

	printf("Parsing...\n");

	result =
		module->parseFile(sourceFileName) &&
		module->parseImports();

	if (!result) {
		printf("Error: %s\n", err::getLastErrorDescription().sz());
		return -1;
	}

	printf("Compiling & JITting...\n");

	result =
		module->compile() &&
		module->jit();

	if (!result) {
		printf("Error: %s\n", err::getLastErrorDescription().sz());
		return -1;
	}

	printf("Starting up runtime...\n");

	result = runtime->startup(module);
	if (!result) {
		printf("Error: %s\n", err::getLastErrorDescription().sz());
		return -1;
	}

	int retval = 0;

	JNC_BEGIN_CALL_SITE(runtime)
		c2jnc::testInt32(module);
		c2jnc::testInt64(module);
		c2jnc::testStruct32(module);
		c2jnc::testStruct64(module);
		c2jnc::testStruct128(module);
		c2jnc::testVariant(module);
		c2jnc::testFloat(module);
		c2jnc::testDouble(module);

		jnc2c::test(module, "jnc2c.testInt32");
		jnc2c::test(module, "jnc2c.testInt64");
		jnc2c::test(module, "jnc2c.testStruct32");
		jnc2c::test(module, "jnc2c.testStruct64");
		jnc2c::test(module, "jnc2c.testStruct128");
		jnc2c::test(module, "jnc2c.testVariant");
		jnc2c::test(module, "jnc2c.testPtr");
		jnc2c::test(module, "jnc2c.testFloat");
		jnc2c::test(module, "jnc2c.testDouble");
	JNC_CALL_SITE_CATCH()

	printf("Runtime exception caught: %s\n", err::getLastErrorDescription().sz());
	retval = -1;

	JNC_END_CALL_SITE()

	printf("Shutting down...\n");
	runtime->shutdown();

	printf("Done.\n");
	return retval;
}

//..............................................................................
