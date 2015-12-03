#include "pch.h"
#include "CmdLine.h"
#include "version.h"

#define _PRINT_USAGE_IF_NO_ARGUMENTS

//.............................................................................

void
printVersion ()
{
	printf (
		"jncx-info (%s) v%d.%d.%d\n",
		_AXL_CPU_STRING,
		VERSION_MAJOR,
		VERSION_MINOR,
		VERSION_REVISION
		);
}

void
printUsage ()
{
	printVersion ();

	sl::String helpString = CmdLineSwitchTable::getHelpString ();
	printf ("Usage: jncx-info <options> <file.jncx>\n%s", helpString.cc ());
}

int
printSourceFile (
	const char* fileName,
	const char* sourceFileName
	)
{
	sys::DynamicLibrary dynamicLib;
	bool result = dynamicLib.open (fileName);
	if (!result)
	{
		printf ("cannot open '%s': %s\n", fileName, err::getLastErrorDescription ().cc ());
		return -1;
	}
		
	jnc::ext::ExtensionLibMainFunc* mainFunc = (jnc::ext::ExtensionLibMainFunc*) dynamicLib.getFunction (jnc::ext::g_extensionLibMainFuncName);
	jnc::ext::ExtensionLib* extensionLib = mainFunc ? mainFunc (jnc::ext::getStdExtensionLibHost ()) : NULL;
	if (!extensionLib)
	{
		printf ("cannot get extension lib in '%s'", fileName);
		return -1;
	}
	
	sl::StringSlice source = extensionLib->findSourceFileContents (sourceFileName);
	if (source.isEmpty ())
	{
		printf ("extension lib '%s' does not contain '%s'", fileName, sourceFileName);
		return -1;
	}


	fwrite (source, source.getLength (), 1, stdout);
	return 0;
}

int
printSourceFileList (const char* fileName)
{
	sys::DynamicLibrary dynamicLib;
	bool result = dynamicLib.open (fileName);
	if (!result)
	{
		printf ("cannot open '%s': %s\n", fileName, err::getLastErrorDescription ().cc ());
		return -1;
	}
		
	jnc::ext::ExtensionLibMainFunc* mainFunc = (jnc::ext::ExtensionLibMainFunc*) dynamicLib.getFunction (jnc::ext::g_extensionLibMainFuncName);
	jnc::ext::ExtensionLib* extensionLib = mainFunc ? mainFunc (jnc::ext::getStdExtensionLibHost ()) : NULL;
	if (!extensionLib)
	{
		printf ("cannot get extension lib in '%s'", fileName);
		return -1;
	}
	
	size_t count = extensionLib->getSourceFileCount ();
	for (size_t i = 0; i < count; i++)
	{
		const char* fileName = extensionLib->getSourceFileName (i);
		printf ("%s\n", fileName);
	}

	return 0;
}

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
	int result;

	CmdLine cmdLine;
	CmdLineParser parser (&cmdLine);

#ifdef _PRINT_USAGE_IF_NO_ARGUMENTS
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
		return -1;
	}

	result = 0;

	if (cmdLine.m_flags & CmdLineFlag_Help)
		printUsage ();
	else if (cmdLine.m_flags & CmdLineFlag_Version)
		printVersion ();
	else if (cmdLine.m_flags & CmdLineFlag_SourceFile)
		result = printSourceFile (cmdLine.m_fileName, cmdLine.m_sourceFileName);
	else 
		result = printSourceFileList (cmdLine.m_fileName);

	return result;
}

//.............................................................................
