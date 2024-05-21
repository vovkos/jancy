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
	module->initialize("jnc_sample_05_pass_cpp");
	module->addStaticLib(jnc::StdLib_getLib());
	module->require(jnc::ModuleItemKind_Function, "foo");

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
	jnc::Function* fooFunction = (jnc::Function*)nspace->findItem("foo").m_item;
	JNC_ASSERT(fooFunction && fooFunction->getItemKind() == jnc::ModuleItemKind_Function);

	printf("Running...\n");

	jnc::AutoRuntime runtime;
	jnc::GcHeap* gcHeap = runtime->getGcHeap();

	result = runtime->startup(module);
	if (!result) {
		printf("%s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	Error finalResult = Error_Success;
	jnc::DataPtr ptr;

	char string[] = "This is a host string on stack";

	// normally, you want to invalidate foreign pointers
	// immediately upon exiting the call-site

	printf("Automatic foreign pointer invalidation...\n");

	JNC_BEGIN_CALL_SITE(runtime)
		ptr = gcHeap->createForeignBufferPtr(string, sizeof(string));
		jnc::callVoidFunction(fooFunction, ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Runtime error: %s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	// here, ptr cannot be accessed anymore

	result = jnc::callVoidFunction(runtime, fooFunction, ptr);
	if (!result) {
		printf("Exprected error: %s\n", jnc::getLastErrorDescription_v ());
	} else {
		printf("Unexpected success accessing an invalidated pointer\n");
		return Error_Runtime;
	}

	jnc_setErrno(2);

	// sometimes, you may want to invalidate foreign pointers manually

	printf("Manual foreign pointer invalidation...\n");

	JNC_BEGIN_CALL_SITE(runtime)
		ptr = gcHeap->createForeignBufferPtr(string, sizeof(string), false); // isCallSiteLocal = false
		jnc::callVoidFunction(fooFunction, ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Runtime error: %s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	// we still can access ptr here

	result = jnc::callVoidFunction(runtime, fooFunction, ptr);
	if (!result) {
		printf("Runtime error: %s\n", jnc::getLastErrorDescription_v ());
		return Error_Runtime;
	}

	// invalidate manually

	gcHeap->invalidateDataPtr(ptr);

	// now, we can't access it

	result = jnc::callVoidFunction(runtime, fooFunction, ptr);
	if (!result) {
		printf("Expected error: %s\n", jnc::getLastErrorDescription_v ());
	} else {
		printf("Unexpected success accessing an invalidated pointer\n");
		return Error_Runtime;
	}

	printf("Shutting down...\n");

	runtime->shutdown();

	printf("Done.\n");

	return finalResult;
}

//..............................................................................
