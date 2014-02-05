// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_McSnapshotClassType.h"

namespace jnc {

//.............................................................................

enum EMulticastField
{
	EMulticastField_Lock,
	EMulticastField_MaxCount,
	EMulticastField_Count,
	EMulticastField_PtrArray,
	EMulticastField_HandleTable,

	EMulticastField__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EMulticastMethod
{
	EMulticastMethod_Clear,
	EMulticastMethod_Set,
	EMulticastMethod_Add,
	EMulticastMethod_Remove,
	EMulticastMethod_GetSnapshot,
	EMulticastMethod_Call,
	EMulticastMethod__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EMulticastMethodFlag
{
	EMulticastMethodFlag_InaccessibleViaEventPtr = 0x010000,
};

//.............................................................................

class CMulticastClassType: public CClassType
{
	friend class CTypeMgr;

protected:
	CFunctionPtrType* m_pTargetType;
	CMcSnapshotClassType* m_pSnapshotType;
	CStructField* m_FieldArray [EMulticastField__Count];
	CFunction* m_MethodArray [EMulticastMethod__Count];

	TClassPtrTypeTuple* m_pEventClassPtrTypeTuple;

public:
	CMulticastClassType ();

	CFunctionPtrType* 
	GetTargetType ()
	{
		return m_pTargetType;
	}

	CFunctionType* 
	GetFunctionType ()
	{
		return m_pTargetType->GetTargetType ();
	}

	CStructField* 
	GetField (EMulticastField Field)
	{
		ASSERT (Field < EMulticastField__Count);
		return m_FieldArray [Field];
	}

	CFunction* 
	GetMethod (EMulticastMethod Method)
	{
		ASSERT (Method < EMulticastMethod__Count);
		return m_MethodArray [Method];
	}

	CMcSnapshotClassType*
	GetSnapshotType ()
	{
		return m_pSnapshotType;
	}

	virtual 
	bool
	Compile ()
	{
		return 
			CClassType::Compile () &&
			CompileCallMethod ();
	}

	virtual 
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		);	

protected:
	virtual 
	void
	PrepareTypeString ();

	virtual 
	bool
	CalcLayout ()
	{
		return
			CClassType::CalcLayout () &&
			m_pSnapshotType->EnsureLayout ();
	}

	bool
	CompileCallMethod ();
};

//.............................................................................

// structures backing up multicast, e.g.:
// mutlicast OnFire ();

struct TMulticast: TIfaceHdr
{
	volatile intptr_t m_Lock;
	size_t m_MaxCount;
	size_t m_Count;
	void* m_pPtrArray; // array of function closure, weak or unsafe pointers
	void* m_pHandleTable;
	
	CFunction* 
	GetMethod (EMulticastMethod Method)
	{
		return ((CMulticastClassType*) m_pObject->m_pType)->GetMethod (Method);
	}
	
	void
	Call ();

	void
	Call (intptr_t a);

	void
	Call (
		intptr_t a1,
		intptr_t a2
		);
};

//.............................................................................

} // namespace jnc {
