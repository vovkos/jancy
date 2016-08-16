#include "pch.h"
#include "MyLib.h"

//.............................................................................

enum Error
{
	Error_Success = 0,
	Error_CmdLine = -1,
	Error_Io      = -2,
	Error_Compile = -3,
	Error_Runtime = -4,
};

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

	printf ("Initializing...\n");
	
	if (argc < 2)
	{
		printf ("usage: 01_export <script.jnc>\n");
		return Error_CmdLine;
	}

	jnc::initialize ();

	lex::registerParseErrorProvider ();

	srand ((int) sys::getTimestamp ());

	sl::String fileName = argv [1];

	jnc::AutoModule module;

	module->initialize (fileName);
	module->addStaticLib (jnc::StdLib_getLib ());
	module->addStaticLib (MyLib_getLib ());

	printf ("Opening '%s'...\n", fileName.cc ());

	io::SimpleMappedFile file;
	result = file.open (fileName, io::FileFlag_ReadOnly);
	if (!result)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Io;
	}

	printf ("Parsing...\n");

	result = 
		module->parse (fileName, (const char*) file.p (), file.getMappingSize ()) &&
		module->parseImports ();

	if (!result)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Compile;
	}

	printf ("Compiling...\n");

	result = module->compile ();
	if (!result)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Compile;
	}

	printf ("JITting...\n");

	result = module->jit ();
	if (!result)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Compile;
	}

	jnc::Namespace* nspace = module->getGlobalNamespace ()->getNamespace ();
	jnc::Function* mainFunction = nspace->findFunction ("main");
	if (!mainFunction)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Compile;
	}

	printf ("Running...\n");

	jnc::AutoRuntime runtime;

	result = runtime->startup (module);
	if (!result)
	{
		printf ("%s\n", err::getLastErrorDescription ().cc ());
		return Error_Runtime;
	}

	Error finalResult = Error_Success;

	int returnValue;
	result = jnc::callFunction (runtime, mainFunction, &returnValue);
	if (!result)
	{
		printf ("Runtime error: %s\n", err::getLastErrorDescription ().cc ());
		finalResult = Error_Runtime;
	}

	printf ("Shutting down...\n");

	runtime->shutdown ();

	printf ("Done.\n");

	return finalResult;
}

//.............................................................................
