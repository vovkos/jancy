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

#pragma once

#include "jnc_Pch.h"

namespace jnc {
namespace rt {

//..............................................................................

#if (_JNC_OS_POSIX)

class ExceptionMgr
{
protected:
	struct sigaction m_prevSigActionTable [32]; // we will get a compile-time error if not enough

public:
	ExceptionMgr ()
	{
		memset (m_prevSigActionTable, 0, sizeof (m_prevSigActionTable));
	} 

	void
	install ();

protected:
	static
	void
	signalHandler (
		int signal,
		siginfo_t* signalInfo,
		void* context
		);

	static
	void
	signalHandler_SIGUSR (int signal)
	{
		// do nothing (we gc-handshake manually). but we still need a handler
	}

	void
	invokePrevSignalHandler (int signal);
};

#elif (_JNC_OS_WIN)

class ExceptionMgr
{
protected:
	PTOP_LEVEL_EXCEPTION_FILTER m_prevUnhandledExceptionFilter;

public:
	ExceptionMgr ()
	{
		m_prevUnhandledExceptionFilter = NULL;
	}

	void
	install ();

	static
	int
	handleSehException (
		uint_t code,
		EXCEPTION_POINTERS* exceptionPointers
		);

protected:
	static
	LONG 
	WINAPI 
	unhandledExceptionFilter (EXCEPTION_POINTERS* exceptionPointers);
};

#endif

//..............................................................................

} // namespace rt
} // namespace jnc
