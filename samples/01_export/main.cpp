#include "pch.h"
#include "MyLib.h"

//.............................................................................

enum Error
{
	Error_Success = 0,
	Error_Io      = -1,
	Error_Compile = -2,
	Error_Runtime = -3,
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
	printf ("Initializing...\n");
	
	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();
	
	err::registerParseErrorProvider ();

#if (_AXL_ENV == AXL_ENV_WIN)
	rtl::String fileName = io::getDir (rtl::String (argv [0])) + "\\script.jnc";
#else
	rtl::String fileName = io::getDir (argv [0]) + "/script.jnc";
#endif
	
	printf ("Opening '%s'...\n", fileName.cc ());

	io::SimpleMappedFile file;
	bool result = file.open (fileName, io::FileFlag_ReadOnly);
	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Io;
	}

	printf ("Parsing...\n");

	jnc::Module module;
	jnc::ScopeThreadModule scopeModule (&module);

	module.create (fileName);

	result = module.parse (
		fileName,
		(const char*) file.p (),
		file.getSize ()
		);

	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Compile;
	}

	printf ("Compiling...\n");

	result = module.compile ();
	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Compile;
	}

	printf ("Mapping...\n");

	result =
		module.createLlvmExecutionEngine () &&
		MyLib::mapFunctions (&module);

	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Compile;
	}

	printf ("JITting...\n");

	result = module.jit ();
	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Compile;
	}

	jnc::Function* mainFunction = module.getFunctionByName ("main");
	if (!mainFunction)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Compile;
	}

	printf ("Running...\n");

	jnc::Runtime runtime;
	jnc::ScopeThreadRuntime scopeRuntime (&runtime);

	result = 
		runtime.create (16 * 1024, 16 * 1024) &&
		runtime.addModule (&module); // 16K gc heap, 16K stack
		runtime.startup ();

	if (!result)
	{
		printf ("%s\n", err::getError ()->getDescription ().cc ());
		return Error_Runtime;
	}

	jnc::Function* constructor = module.getConstructor ();
	jnc::Function* destructor = module.getDestructor ();

	if (destructor)
		runtime.addStaticDestructor ((jnc::StaticDestructor*) destructor->getMachineCode ());

	Error finalResult = Error_Success;

	try
	{
		typedef int Function ();

		if (constructor)
			((Function*) constructor->getMachineCode ()) ();

		int returnValue = ((Function*) mainFunction->getMachineCode ()) ();
		printf ("'main' returned (%d)\n", returnValue);
	}
	catch (err::Error error)
	{
		printf ("Runtime error: %s\n", error.getDescription ().cc ());
		finalResult = Error_Runtime;
	}
	catch (...)
	{
		printf ("Unexpected runtime exception\n");
		finalResult = Error_Runtime;
	}

	printf ("Shutting down...\n");

	runtime.shutdown ();

	printf ("Done.\n");

	return finalResult;
}

//.............................................................................
