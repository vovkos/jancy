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

enum StdRuntimeLimit
{
#ifdef _DEBUG
	StdRuntimeLimit_GcPeriodSize = 0, // run gc on every allocation
#elif (_AXL_CPU == AXL_CPU_X86)
	StdRuntimeLimit_GcPeriodSize = 1 * 1024 * 1024, // 1MB gc period
#else
	StdRuntimeLimit_GcPeriodSize = 2 * 1024 * 1024, // 2MB gc period
#endif

#if (_AXL_CPU == AXL_CPU_X86)
	StdRuntimeLimit_StackSize = 2 * 1024 * 1024, // 2MB std stack 
#else
	StdRuntimeLimit_StackSize = 4 * 1024 * 1024, // 4MB std stack limit
#endif
};

//.............................................................................

enum CreateObjectFlag
{
	CreateObjectFlag_Prime     = 0x01,
	CreateObjectFlag_Construct = 0x02,
	CreateObjectFlag_Pin       = 0x04,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef 
void
StaticDestructor ();

typedef 
void
Destructor (jnc::IfaceHdr* iface);

//.............................................................................

class Runtime
{
	friend class GcHeap;

protected:
	enum GcState
	{
		GcState_Idle = 0,
		GcState_WaitSafePoint,
		GcState_Mark,
		GcState_Sweep,
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

	struct StaticDestruct: rtl::ListLink
	{
		union
		{
			StaticDestructor* m_staticDtor;
			Destructor* m_dtor;
		};

		IfaceHdr* m_iface;
	};

protected:
	mt::Lock m_lock;
	rtl::Array <Module*> m_moduleArray;

	// gc-heap

	volatile GcState m_gcState;

	size_t m_gcTotalAllocSize;
	size_t m_gcCurrentAllocSize;
	size_t m_gcCurrentPeriodSize;

	mt::NotificationEvent m_gcIdleEvent;
	mt::NotificationEvent m_gcSafePointEvent;
	volatile intptr_t m_gcUnsafeThreadCount;
	rtl::Array <ObjHdr*> m_gcObjectArray;
	rtl::Array <ObjHdr*> m_gcMemBlockArray;

	rtl::HashTable <IfaceHdr*, rtl::HashId <IfaceHdr*> > m_gcPinTable;
	rtl::AuxList <GcDestructGuard> m_gcDestructGuardList;
	rtl::Array <GcRoot> m_gcStaticRootArray;
	rtl::Array <GcRoot> m_gcRootArray [2];
	size_t m_gcCurrentRootArrayIdx;

	rtl::StdList <StaticDestruct> m_staticDestructList;

	// tls

	size_t m_tlsSlot;
	size_t m_tlsSize;
	rtl::AuxList <TlsHdr> m_tlsList;

public:
	// adjustable limits

	size_t m_stackSizeLimit;
	size_t m_gcPeriodSizeLimit;

public:
	Runtime ();

	~Runtime ()
	{
		destroy ();
	}

	bool
	create ();

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

	static
	void
	runtimeError (const err::Error& error)
	{
		err::setError (error);
		AXL_MT_LONG_JMP_THROW ();
	}

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

	void*
	gcAllocate (size_t size);

	void*
	gcTryAllocate (size_t size);

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
			CreateObjectFlag_Prime | 
			CreateObjectFlag_Construct | 
			CreateObjectFlag_Pin
		);

	void
	pinObject (IfaceHdr* object);

	void
	unpinObject (IfaceHdr* object);

	// static destruct

	void
	addStaticDestructor (StaticDestructor *dtor);

	void
	addDestructor (
		Destructor *dtor,
		jnc::IfaceHdr* iface
		);

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
