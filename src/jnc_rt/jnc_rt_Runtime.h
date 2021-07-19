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

#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace rt {

//..............................................................................

class Runtime
{
	friend class GcHeap;

protected:
	enum State
	{
		State_Idle,
		State_Running,
		State_ShuttingDown,
	};

	enum Shutdown
	{
		Shutdown_IterationCount    = 6,
		Shutdown_WaitThreadTimeout = 500, // 3 sec total
	};

protected:
	sys::Lock m_lock;
	ct::Module* m_module;
	State m_state;
	sys::NotificationEvent m_noThreadEvent;
	size_t m_tlsSize;
	sl::List<Tls, GetTlsLink> m_tlsList;
	GcHeap m_gcHeap;

public:
	void* volatile m_userData;

public:
	Runtime();

	~Runtime()
	{
		shutdown();
	}

	ct::Module*
	getModule()
	{
		return m_module;
	}

	GcHeap*
	getGcHeap()
	{
		return &m_gcHeap;
	}

	bool
	isAborted()
	{
		return m_gcHeap.isAborted();
	}

	bool
	startup(ct::Module* module);

	void
	shutdown();

	void
	abort();

	void
	initializeCallSite(CallSite* callSite);

	void
	uninitializeCallSite(CallSite* callSite);

	SjljFrame*
	setSjljFrame(SjljFrame* frame);

	static
	void
	dynamicThrow();

#if (_JNC_OS_POSIX)
	static
	void
	saveSignalInfo(SjljFrame* sjljFrame);
#endif
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
Tls*
getCurrentThreadTls()
{
	CallSite* callSite = sys::getTlsPtrSlotValue<CallSite>();
	return callSite ? callSite->m_tls : NULL;
}

JNC_INLINE
Runtime*
getCurrentThreadRuntime()
{
	Tls* tls = getCurrentThreadTls();
	return tls ? tls->m_runtime : NULL;
}

//..............................................................................

} // namespace rt
} // namespace jnc
