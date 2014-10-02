// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_TlsMgr.h"
#include "jnc_Value.h"
#include "jnc_ClassType.h"
#include "jnc_DataPtrType.h"

namespace jnc {

class Module;

//.............................................................................

enum CreateObjectFlagKind
{
	CreateObjectFlagKind_Prime     = 0x01,
	CreateObjectFlagKind_Construct = 0x02,
	CreateObjectFlagKind_Pin       = 0x04,
};

//.............................................................................

class Runtime
{
	friend class GcHeap;

protected:
	enum GcStateKind
	{
		GcStateKind_Idle = 0,
		GcStateKind_WaitSafePoint,
		GcStateKind_Mark,
		GcStateKind_Sweep,
	};

	struct GcRoot
	{
		void* m_p;
		Type* m_type;
	};

	struct GcDestructGuard: rtl::ListLink
	{
		rtl::Array <IfaceHdr*>* m_destructArray;
	};

protected:
	mt::Lock m_lock;
	rtl::Array <Module*> m_moduleArray;

	// gc-heap

	volatile GcStateKind m_gcState;

	size_t m_gcHeapLimit;
	size_t m_totalGcAllocSize;
	size_t m_currentGcAllocSize;
	size_t m_periodGcAllocLimit;
	size_t m_periodGcAllocSize;

	mt::NotificationEvent m_gcIdleEvent;
	mt::NotificationEvent m_gcSafePointEvent;
	volatile intptr_t m_gcUnsafeThreadCount;
	rtl::Array <ObjHdr*> m_gcObjectArray;
	rtl::Array <ObjHdr*> m_gcMemBlockArray;

	rtl::HashTable <IfaceHdr*, rtl::HashId <IfaceHdr*> > m_gcPinTable;
	rtl::AuxList <GcDestructGuard> m_gcDestructGuardList;
	rtl::Array <GcRoot> m_staticGcRootArray;
	rtl::Array <GcRoot> m_gcRootArray [2];
	size_t m_currentGcRootArrayIdx;

	// tls

	size_t m_stackLimit;
	size_t m_tlsSlot;
	size_t m_tlsSize;
	rtl::AuxList <TlsHdr> m_tlsList;

public:
	Runtime ();

	~Runtime ()
	{
		destroy ();
	}

	bool
	create (
		size_t heapLimit = -1,
		size_t stackLimit = -1
		);

	rtl::Array <Module*> 
	getModuleArray ()
	{
		return m_moduleArray;
	}

	Module* 
	getFirstModule ()
	{
		return !m_moduleArray.isEmpty () ? m_moduleArray [0] : NULL;
	}

	bool
	addModule (Module* module);

	void
	destroy ();

	bool
	startup ();

	void
	shutdown ();

	static
	void
	runtimeError (
		int error,
		void* codeAddr,
		void* dataAddr
		);

	// gc heap

	void
	runGc ();

	size_t
	gcMakeThreadSafe (); // return prev gc level

	void
	restoreGcLevel (size_t prevGcLevel);

	void
	gcEnter ();

	void
	gcLeave ();

	void
	gcPulse ();

	void*
	gcAllocate (
		Type* type,
		size_t elementCount = 1
		);

	void*
	gcTryAllocate (
		Type* type,
		size_t elementCount = 1
		);

	void
	addGcRoot (
		void* p,
		Type* type
		);

	// creating objects on gc heap

	IfaceHdr*
	createObject (
		ClassType* type,
		uint_t flags =
			CreateObjectFlagKind_Prime | 
			CreateObjectFlagKind_Construct | 
			CreateObjectFlagKind_Pin
		);

	void
	pinObject (IfaceHdr* object);

	void
	unpinObject (IfaceHdr* object);

	// tls

	size_t
	getTlsSlot ()
	{
		return m_tlsSlot;
	}

	TlsHdr*
	getTls ();

	TlsHdr*
	createTls ();

	void
	destroyTls (TlsHdr* tls);

protected:
	void
	waitGcIdleAndLock ();

	void
	gcAddObject (
		ObjHdr* object,
		ClassType* type
		);

	void
	runGc_l ();

	void
	gcMarkCycle ();

	void
	gcIncrementUnsafeThreadCount ();

	void
	gcDecrementUnsafeThreadCount ();

	void
	markGcLocalHeapRoot (
		void* p,
		Type* type
		);
};

//.............................................................................

typedef mt::ScopeTlsSlot <Runtime> ScopeThreadRuntime;

inline
Runtime*
getCurrentThreadRuntime ()
{
	return mt::getTlsSlotValue <Runtime> ();
}

//.............................................................................

enum RuntimeErrorKind
{
	RuntimeErrorKind_OutOfMemory,
	RuntimeErrorKind_StackOverflow,
	RuntimeErrorKind_ScopeMismatch,
	RuntimeErrorKind_DataPtrOutOfRange,
	RuntimeErrorKind_NullClassPtr,
	RuntimeErrorKind_NullFunctionPtr,
	RuntimeErrorKind_NullPropertyPtr,
	RuntimeErrorKind_AbstractFunction,
};

//.............................................................................

} // namespace jnc
