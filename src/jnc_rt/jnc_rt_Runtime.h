// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_rt_GcHeap.h"

namespace jnc {
namespace rt {

//.............................................................................

enum RuntimeDef
{
#if (_AXL_PTR_SIZE == 8)
	RuntimeDef_StackSizeLimit    = 1 * 1024 * 1024, // 1MB std stack limit
	RuntimeDef_MinStackSizeLimit = 32 * 1024,       // 32KB min stack 
#else
	RuntimeDef_StackSizeLimit    = 512 * 1024,      // 512KB std stack 
	RuntimeDef_MinStackSizeLimit = 16 * 1024,       // 16KB min stack 
#endif
};

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

	static
	void
	dynamicThrow ();

	static
	void
	runtimeError (const err::Error& error)
	{
		err::setError (error);
		dynamicThrow ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Runtime*
getCurrentThreadRuntime ()
{
	return sys::getTlsSlotValue <Runtime> ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Tls*
getCurrentThreadTls ()
{
	return sys::getTlsSlotValue <Tls> ();
}

//.............................................................................

} // namespace rt
} // namespace jnc
