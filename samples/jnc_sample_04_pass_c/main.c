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
#include "script.jnc.c"
;

//..............................................................................

enum Error {
	Error_Success = 0,
	Error_CmdLine = -1,
	Error_Io      = -2,
	Error_Compile = -3,
	Error_Runtime = -4,
};

typedef enum Error Error;

//..............................................................................

#if (_JNC_OS_WIN)

const char*
convertToUtf8(
	const wchar_t* string,
	size_t length
) {
	char* p;
	int requiredLength;

	requiredLength = WideCharToMultiByte(CP_UTF8, 0, string, (int)length, NULL, 0, NULL, NULL);
	if (!requiredLength)
		return NULL;

	p = malloc(requiredLength + 1);
	p[requiredLength] = 0; // ensure zero-termination

	WideCharToMultiByte(CP_UTF8, 0, string, (int)length, p, requiredLength, NULL, NULL);
	return p;
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
	typedef
	void
	FooFunc(jnc_DataPtr);

	int result;
	Error finalResult = Error_Success;

	const char* fileName = NULL;
	jnc_Module* module = NULL;
	jnc_Runtime* runtime = NULL;
	jnc_GcHeap* gcHeap = NULL;
	jnc_Namespace* nspace;
	jnc_Function* function;
	jnc_DataPtr ptr;
	FooFunc* mc;
	char string[] = "This is a host string on stack";

	printf("Initializing...\n");
	jnc_initialize("jnc_sample_04_pass_c");

	module = jnc_Module_create();
	jnc_Module_initialize(module, "jnc_sample_04_pass_c", jnc_ModuleCompileFlag_StdFlags);
	jnc_Module_addStaticLib(module, jnc_StdLib_getLib());
	jnc_Module_require(module, jnc_ModuleItemKind_Function, "foo", 1);

	if (argc < 2) {
		printf("Parsing default script...\n");
		result = jnc_Module_parse(module, "script.jnc", g_script, sizeof(g_script) - 1);
	} else {

#if (_JNC_OS_WIN)
		fileName = convertToUtf8(argv[1], -1);
#else
		fileName = argv[1];
#endif
		printf("Parsing '%s'...\n", fileName);
		result = jnc_Module_parseFile(module, fileName);
	}

	result = result && jnc_Module_parseImports(module);
	if (!result) {
		printf("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	printf("Compiling & JITting...\n");

	result =
		jnc_Module_compile(module) &&
		jnc_Module_optimize(module, 2) &&
		jnc_Module_jit(module);

	if (!result) {
		printf("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	nspace = jnc_ModuleItem_getNamespace((jnc_ModuleItem*)jnc_Module_getGlobalNamespace(module));
	function = (jnc_Function*)jnc_Namespace_findItem(nspace, "foo").m_item;
	JNC_ASSERT(function && jnc_ModuleItem_getItemKind((jnc_ModuleItem*)function) == jnc_ModuleItemKind_Function);

	printf("Running...\n");

	runtime = jnc_Runtime_create();
	result = jnc_Runtime_startup(runtime, module);
	if (!result) {
		printf("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	gcHeap = jnc_Runtime_getGcHeap(runtime);
	mc = jnc_Function_getMachineCode(function);

	// normally, you want to invalidate foreign pointers
	// immediately upon exiting the call-site

	printf("Automatic foreign pointer invalidation...\n");

	JNC_BEGIN_CALL_SITE(runtime)
		ptr = jnc_GcHeap_createForeignBufferPtr(gcHeap, string, sizeof(string), 1);
		mc(ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Runtime error: %s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	// here, ptr cannot be accessed anymore

	JNC_BEGIN_CALL_SITE(runtime)
		mc(ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Expected error: %s\n", jnc_getLastErrorDescription_v ());
	} else {
		printf("Unexpected success accessing an invalidated pointer\n");
		finalResult = Error_Runtime;
		goto exit;
	}

	// sometimes, you may want to invalidate foreign pointers manually

	printf("Manual foreign pointer invalidation...\n");

	JNC_BEGIN_CALL_SITE(runtime)
		ptr = jnc_GcHeap_createForeignBufferPtr(gcHeap, string, sizeof(string), 0);
		mc(ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Runtime error: %s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	// we still can access ptr here

	JNC_BEGIN_CALL_SITE(runtime)
		mc(ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Runtime error: %s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	// invalidate manually

	jnc_GcHeap_invalidateDataPtr(gcHeap, ptr);

	// now, we can't access it

	JNC_BEGIN_CALL_SITE(runtime)
		mc(ptr);
	JNC_END_CALL_SITE_EX(&result)

	if (!result) {
		printf("Exprected error: %s\n", jnc_getLastErrorDescription_v ());
	} else {
		printf("Unexpected success accessing an invalidated pointer\n");
		finalResult = Error_Runtime;
		goto exit;
	}

	printf("Shutting down...\n");

	jnc_Runtime_shutdown(runtime);

	printf("Done.\n");

exit:
	if (runtime)
		jnc_Runtime_destroy(runtime);

	if (module)
		jnc_Module_destroy(module);

#if (_JNC_OS_WIN)
	if (fileName)
		free((void*)fileName);
#endif

	return finalResult;
}

//..............................................................................
