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
#include "jnc_rt_ExceptionMgr.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace rt {

//..............................................................................

#if (_JNC_OS_POSIX)

void
ExceptionMgr::install ()
{
	int result;
	sigset_t signalMask;
	sigemptyset (&signalMask);
	sigaddset (&signalMask, SIGSEGV);
	sigaddset (&signalMask, SIGBUS);
	sigaddset (&signalMask, SIGFPE);

	struct sigaction sigAction = { 0 };
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_mask = signalMask;

	sigAction.sa_sigaction = signalHandler;
	result = sigaction (SIGSEGV, &sigAction, &m_prevSigActionTable [SIGSEGV]);
	ASSERT (result == 0);

	result = sigaction (SIGBUS, &sigAction, &m_prevSigActionTable [SIGBUS]);
	ASSERT (result == 0);

	result = sigaction (SIGFPE, &sigAction, &m_prevSigActionTable [SIGFPE]);
	ASSERT (result == 0);

	sigAction.sa_flags = 0;
	sigAction.sa_handler = signalHandler_SIGUSR;
	result = sigaction (SIGUSR1, &sigAction, &m_prevSigActionTable [SIGUSR1]);
	ASSERT (result == 0);
}

void
ExceptionMgr::signalHandler (
	int signal,
	siginfo_t* signalInfo,
	void* context
	)
{
	enum
	{
	#if (_JNC_OS_DARWIN)
		GcGuardPageHitSignal = SIGBUS
	#else
		GcGuardPageHitSignal = SIGSEGV
	#endif
	};

	// while POSIX does not require pthread_getspecific to be async-signal-safe, in practice it is

	Tls* tls = getCurrentThreadTls ();
	if (!tls)
	{
		sl::getSimpleSingleton <ExceptionMgr> ()->invokePrevSignalHandler (signal);
		return;
	}

	if (signal == GcGuardPageHitSignal)
	{
		GcHeap* gcHeap = tls->m_runtime->getGcHeap ();
		if (signalInfo->si_addr == gcHeap->getGuardPage ())
		{
			gcHeap->handleGuardPageHit (&tls->m_gcMutatorThread);
			return;
		}
	}

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	if (!tlsVariableTable->m_sjljFrame)
	{
		sl::getSimpleSingleton <ExceptionMgr> ()->invokePrevSignalHandler (signal);
	}
	else
	{
		AXL_TODO ("add extra variables to the SJLJ frame to store error information");
		longjmp (tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	}
}

void
ExceptionMgr::invokePrevSignalHandler (int signal)
{
	AXL_TODO ("invoking prev hanlder could be done faster without sigaction/raise/sigaction triple")

	int result;
	struct sigaction currentSigAction;

	result = sigaction (signal, &m_prevSigActionTable [signal], &currentSigAction);
	ASSERT (result == 0);

	raise (signal);

	result = sigaction (signal, &currentSigAction, NULL);
	ASSERT (result == 0);
}

#elif (_AXL_OS_WIN)

void
ExceptionMgr::install ()
{
	m_prevUnhandledExceptionFilter = ::SetUnhandledExceptionFilter (unhandledExceptionFilter);
}

int
ExceptionMgr::handleSehException (
	uint_t code,
	EXCEPTION_POINTERS* exceptionPointers
	)
{
	Tls* tls = getCurrentThreadTls ();
	if (!tls)
		return EXCEPTION_CONTINUE_SEARCH;

	GcHeap* gcHeap = tls->m_runtime->getGcHeap ();

	if (code == EXCEPTION_ACCESS_VIOLATION &&
		exceptionPointers->ExceptionRecord->ExceptionInformation [1] == (uintptr_t) gcHeap->getGuardPage ())
	{		
		gcHeap->handleGuardPageHit (&tls->m_gcMutatorThread);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	if (tlsVariableTable->m_sjljFrame)
	{
		AXL_TODO ("encode SEH information better");
		sys::win::setNtStatus (code);
		longjmp (tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

LONG 
WINAPI 
ExceptionMgr::unhandledExceptionFilter (EXCEPTION_POINTERS* exceptionPointers)
{
	int result = handleSehException (exceptionPointers->ExceptionRecord->ExceptionCode, exceptionPointers);
	if (result != EXCEPTION_CONTINUE_SEARCH)
		return result;

	ExceptionMgr* self = sl::getSimpleSingleton <ExceptionMgr> ();
	if (self->m_prevUnhandledExceptionFilter)
		result = self->m_prevUnhandledExceptionFilter (exceptionPointers);

	return result;
}

#endif

//..............................................................................

} // namespace rt
} // namespace jnc
