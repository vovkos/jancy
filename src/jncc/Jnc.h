#pragma once

#include "CmdLine.h"
#include "OutStream.h"

//.............................................................................

enum EJncError
{
	EJncError_Success = 0,
	EJncError_InvalidCmdLine,
	EJncError_IoFailure,
	EJncError_CompileFailure,
	EJncError_RunFailure,
};

//.............................................................................

void
PrintVersion (COutStream* pOutStream);

void
PrintUsage (COutStream* pOutStream);

//.............................................................................

class CStdLib: public jnc::CStdLib
{
public:
	JNC_API_BEGIN_LIB ()
		JNC_API_FUNCTION ("printf",  &Printf)
		JNC_API_LIB (jnc::CStdLib)
	JNC_API_END_LIB ()

	static
	int
	Printf (
		const char* pFormat,
		...
		);
};

//.............................................................................

class CJnc
{
protected:
	static CJnc* m_pCurrentJnc;

	TCmdLine* m_pCmdLine;
	COutStream* m_pOutStream;

	jnc::CModule m_Module;
	jnc::CRuntime m_Runtime;

public:
	CJnc ()
	{
		m_pCurrentJnc = this;
		m_pCmdLine = NULL;
		m_pOutStream = NULL;
	}

	static
	CJnc*
	GetCurrentJnc ()
	{
		return m_pCurrentJnc;
	}

	COutStream*
	GetOutStream ()
	{
		return m_pOutStream;
	}

	int
	Run (
		TCmdLine* pCmdLine,
		COutStream* pOutStream
		);

protected:
	bool
	Compile (
		const char* pFileName,
		const char* pSource,
		size_t Length
		);

	bool
	Jit ();

	void
	PrintLlvmIr ();

	void
	PrintDisassembly ();

	bool
	RunFunction (int* pReturnValue = NULL);

	bool
	RunFunction (
		jnc::CFunction* pFunction,
		int* pReturnValue = NULL
		);

	int
	Server ();

	int
	Client (
		SOCKET Socket,
		sockaddr_in* pSockAddr
		);
};

//.............................................................................
