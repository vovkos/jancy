#pragma once

#include "CmdLine.h"
#include "OutStream.h"

//.............................................................................

enum JncErrorKind
{
	JncErrorKind_Success = 0,
	JncErrorKind_InvalidCmdLine,
	JncErrorKind_IoFailure,
	JncErrorKind_CompileFailure,
	JncErrorKind_RunFailure,
};

//.............................................................................

void
printVersion (OutStream* outStream);

void
printUsage (OutStream* outStream);

//.............................................................................

class StdLib: public jnc::StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_FUNCTION ("printf",  &printf)
		JNC_LIB (jnc::StdLib)
	JNC_END_LIB ()

	static
	int
	printf (
		const char* format,
		...
		);
};

//.............................................................................

class Jnc
{
protected:
	static Jnc* m_currentJnc;

	CmdLine* m_cmdLine;
	OutStream* m_outStream;

	jnc::Module m_module;
	jnc::Runtime m_runtime;

public:
	Jnc ()
	{
		m_currentJnc = this;
		m_cmdLine = NULL;
		m_outStream = NULL;
	}

	static
	Jnc*
	getCurrentJnc ()
	{
		return m_currentJnc;
	}

	OutStream*
	getOutStream ()
	{
		return m_outStream;
	}

	int
	run (
		CmdLine* cmdLine,
		OutStream* outStream
		);

protected:
	bool
	compile (
		const char* fileName,
		const char* source,
		size_t length
		);

	bool
	jit ();

	void
	printLlvmIr ();

	void
	printDisassembly ();

	bool
	runFunction (int* returnValue = NULL);

	bool
	runFunction (
		jnc::Function* function,
		int* returnValue = NULL
		);

	int
	server ();

	int
	client (
		SOCKET socket,
		sockaddr_in* sockAddr
		);
};

//.............................................................................
