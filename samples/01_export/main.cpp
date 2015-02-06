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
	printf ("Initializing...\n");
	
	if (argc < 2)
	{
		printf ("usage: 01_export <script.jnc>\n");
		return Error_CmdLine;
	}

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	err::registerParseErrorProvider ();

	srand ((int) axl::g::getTimestamp ());
	
	rtl::String fileName = argv [1];

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
		typedef void ConstructorProc ();
		typedef int MainProc ();

		if (constructor)
			((ConstructorProc*) constructor->getMachineCode ()) ();

		int returnValue = ((MainProc*) mainFunction->getMachineCode ()) ();
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
