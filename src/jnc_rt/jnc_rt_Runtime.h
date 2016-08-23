// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace rt {

//.............................................................................

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

protected:
	sys::Lock m_lock;
	ct::Module* m_module;
	State m_state;
	sys::NotificationEvent m_noThreadEvent;
	size_t m_tlsSize;
	sl::StdList <Tls> m_tlsList;

	size_t m_stackSizeLimit; // adjustable limits

	GcHeap m_gcHeap;

public:
	Runtime ();

	~Runtime ()
	{
		shutdown ();
	}

	ct::Module* 
	getModule ()
	{
		return m_module;
	}

	GcHeap*
	getGcHeap ()
	{
		return &m_gcHeap;
	}

	size_t 
	getStackSizeLimit ()
	{
		return m_stackSizeLimit;
	}

	bool
	setStackSizeLimit (size_t sizeLimit);

	bool 
	startup (ct::Module* module);
	
	void
	shutdown ();

	void 
	initializeThread (ExceptionRecoverySnapshot* ers);

	void 
	uninitializeThread (ExceptionRecoverySnapshot* ers);
	
	void
	checkStackOverflow ();

	SjljFrame*
	setSjljFrame (SjljFrame* frame);

	static
	void
	dynamicThrow ();
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
Tls*
getCurrentThreadTls ()
{
	return sys::getTlsPtrSlotValue <Tls> ();
}

JNC_INLINE
Runtime*
getCurrentThreadRuntime ()
{
	Tls* tls = getCurrentThreadTls ();
	return tls ? tls->m_runtime : NULL;
}

//.............................................................................

} // namespace rt
} // namespace jnc
