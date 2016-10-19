#include "pch.h"
#include "MyLib.h"

//..............................................................................

enum Error
{
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
convertToUtf8 (
	const wchar_t* string,
	size_t length
	)
{
	char* p;
	int requiredLength;

	requiredLength = WideCharToMultiByte (CP_UTF8, 0, string, (int) length, NULL, 0, NULL, NULL);
	if (!requiredLength)
		return NULL;

	p = malloc (requiredLength + 1);
	p [requiredLength] = 0; // ensure zero-termination

	WideCharToMultiByte (CP_UTF8, 0, string, (int) length, p, requiredLength, NULL, NULL);
	return p;
}

#endif

//..............................................................................

#if (_JNC_OS_WIN)
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
	typedef
	int
	MainFunc ();

	int result;
	Error finalResult = Error_Success;

	const char* fileName = NULL;
	jnc_Module* module = NULL;
	jnc_Runtime* runtime = NULL;
	jnc_Namespace* nspace;
	jnc_Function* function;
	MainFunc* mc;
	int returnValue;

	printf ("Initializing...\n");

	if (argc < 2)
	{
		printf ("usage: 01_export <script.jnc>\n");
		finalResult = Error_CmdLine;
		goto exit;
	}

	jnc_initialize ();

#if (_JNC_OS_WIN)
	fileName = convertToUtf8 (argv [1], -1);
#else
	fileName = argv [1];
#endif

	module = jnc_Module_create ();
	jnc_Module_initialize (module, fileName, jnc_ModuleCompileFlag_StdFlags);
	jnc_Module_addStaticLib (module, jnc_StdLib_getLib ());
	jnc_Module_addStaticLib (module, MyLib_getLib ());

	printf ("Parsing '%s'...\n", fileName);

	result =
		jnc_Module_parseFile (module, fileName) &&
		jnc_Module_parseImports (module);

	if (!result)
	{
		printf ("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	printf ("Compiling...\n");

	result = jnc_Module_compile (module);
	if (!result)
	{
		printf ("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	printf ("JITting...\n");

	result = jnc_Module_jit (module);
	if (!result)
	{
		printf ("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	nspace = jnc_ModuleItem_getNamespace ((jnc_ModuleItem*) jnc_Module_getGlobalNamespace (module));
	function = jnc_Namespace_findFunction (nspace, "main");
	if (!function)
	{
		printf ("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Compile;
		goto exit;
	}

	printf ("Running...\n");

	runtime = jnc_Runtime_create ();
	result = jnc_Runtime_startup (runtime, module);
	if (!result)
	{
		printf ("%s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	mc = jnc_Function_getMachineCode (function);

	JNC_BEGIN_CALL_SITE (runtime)
	returnValue = mc ();
	JNC_END_CALL_SITE_EX (&result)

	if (!result)
	{
		printf ("Runtime error: %s\n", jnc_getLastErrorDescription_v ());
		finalResult = Error_Runtime;
		goto exit;
	}

	printf ("Shutting down...\n");

	jnc_Runtime_shutdown (runtime);

	printf ("Done.\n");

exit:
	if (runtime)
		jnc_Runtime_destroy (runtime);

	if (module)
		jnc_Module_destroy (module);

	if (fileName)
		free ((void*) fileName);

	return finalResult;
}

//..............................................................................
