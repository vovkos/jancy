// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_TlsMgr.h"
#include "jnc_Value.h"
#include "jnc_ClassType.h"
#include "jnc_DataPtrType.h"

namespace jnc {

class CModule;

//.............................................................................

enum EJit
{
	EJit_Normal = 0,
	EJit_McJit,

	EJit__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetJitKindString (EJit JitKind);

//.............................................................................

enum ECreateObjectFlag
{
	ECreateObjectFlag_Prime     = 0x01,
	ECreateObjectFlag_Construct = 0x02,
	ECreateObjectFlag_Pin       = 0x04,
};

//.............................................................................

class CRuntime
{
	friend class CGcHeap;

protected:
	enum EGcState
	{
		EGcState_Idle = 0,
		EGcState_WaitSafePoint,
		EGcState_Mark,
		EGcState_Sweep,
	};

	struct TGcRoot
	{
		void* m_p;
		CType* m_pType;
	};

	struct TGcDestructGuard: rtl::TListLink
	{
		rtl::CArrayT <TIfaceHdr*>* m_pDestructArray;
	};

protected:
	CModule* m_pModule;
	EJit m_JitKind;
	llvm::ExecutionEngine* m_pLlvmExecutionEngine;
	rtl::CStringHashTableMapT <void*> m_FunctionMap;

	mt::CLock m_Lock;

	// gc-heap

	volatile EGcState m_GcState;

	size_t m_GcHeapLimit;
	size_t m_TotalGcAllocSize;
	size_t m_CurrentGcAllocSize;
	size_t m_PeriodGcAllocLimit;
	size_t m_PeriodGcAllocSize;

	mt::CNotificationEvent m_GcIdleEvent;
	mt::CNotificationEvent m_GcSafePointEvent;
	volatile intptr_t m_GcUnsafeThreadCount;
	rtl::CArrayT <TObjHdr*> m_GcObjectArray;
	rtl::CArrayT <TObjHdr*> m_GcMemBlockArray;

	rtl::CHashTableT <TIfaceHdr*, rtl::CHashIdT <TIfaceHdr*> > m_GcPinTable;
	rtl::CAuxListT <TGcDestructGuard> m_GcDestructGuardList;
	rtl::CArrayT <TGcRoot> m_StaticGcRootArray;
	rtl::CArrayT <TGcRoot> m_GcRootArray [2];
	size_t m_CurrentGcRootArrayIdx;

	// tls

	size_t m_StackLimit;
	size_t m_TlsSlot;
	size_t m_TlsSize;
	rtl::CAuxListT <TTlsHdr> m_TlsList;

public:
	CRuntime ();

	~CRuntime ()
	{
		Destroy ();
	}

	EJit
	GetJitKind ()
	{
		return m_JitKind;
	}

	bool
	Create (
		CModule* pModule,
		EJit JitKind,
		size_t HeapLimit = -1,
		size_t StackLimit = -1
		);

	void
	Destroy ();

	void
	MapFunction (
		llvm::Function* pLlvmFunction,
		void* pf
		);

	void*
	FindFunctionMapping (const char* pName)
	{
		rtl::CStringHashTableMapIteratorT <void*> It = m_FunctionMap.Find (pName);
		return It ? It->m_Value : NULL;
	}

	bool
	Startup ();

	void
	Shutdown ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	llvm::ExecutionEngine*
	GetLlvmExecutionEngine ()
	{
		return m_pLlvmExecutionEngine;
	}

	static
	void
	RuntimeError (
		int Error,
		void* pCodeAddr,
		void* pDataAddr
		);

	// gc heap

	void
	RunGc ();

	size_t
	GcMakeThreadSafe (); // return prev gc level

	void
	RestoreGcLevel (size_t PrevGcLevel);

	void
	GcEnter ();

	void
	GcLeave ();

	void
	GcPulse ();

	void*
	GcAllocate (
		CType* pType,
		size_t ElementCount = 1
		);

	void*
	GcTryAllocate (
		CType* pType,
		size_t ElementCount = 1
		);

	void
	AddGcRoot (
		void* p,
		CType* pType
		);

	// creating objects on gc heap

	TIfaceHdr*
	CreateObject (
		CClassType* pType,
		uint_t Flags =
			ECreateObjectFlag_Prime | 
			ECreateObjectFlag_Construct | 
			ECreateObjectFlag_Pin
		);

	void
	PinObject (TIfaceHdr* pObject);

	void
	UnpinObject (TIfaceHdr* pObject);

	// tls

	size_t
	GetTlsSlot ()
	{
		return m_TlsSlot;
	}

	TTlsHdr*
	GetTls ();

	TTlsHdr*
	CreateTls ();

	void
	DestroyTls (TTlsHdr* pTls);

protected:
	void
	WaitGcIdleAndLock ();

	void
	GcAddObject (
		TObjHdr* pObject,
		CClassType* pType
		);

	void
	RunGc_l ();

	void
	GcMarkCycle ();

	void
	GcIncrementUnsafeThreadCount ();

	void
	GcDecrementUnsafeThreadCount ();

	void
	MarkGcLocalHeapRoot (
		void* p,
		CType* pType
		);
};

//.............................................................................

typedef mt::CScopeTlsSlotT <CRuntime> CScopeThreadRuntime;

inline
CRuntime*
GetCurrentThreadRuntime ()
{
	return mt::GetTlsSlotValue <CRuntime> ();
}

//.............................................................................

enum ERuntimeError
{
	ERuntimeError_OutOfMemory,
	ERuntimeError_StackOverflow,
	ERuntimeError_ScopeMismatch,
	ERuntimeError_DataPtrOutOfRange,
	ERuntimeError_NullClassPtr,
	ERuntimeError_NullFunctionPtr,
	ERuntimeError_NullPropertyPtr,
	ERuntimeError_AbstractFunction,
};

//.............................................................................

} // namespace jnc
