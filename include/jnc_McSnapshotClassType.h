// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"
#include "jnc_FunctionPtrType.h"

namespace jnc {

//.............................................................................

enum EMcSnapshotField
{
	EMcSnapshotField_Count,
	EMcSnapshotField_PtrArray,

	EMcSnapshotField__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EMcSnapshotMethod
{
	EMcSnapshotMethod_Call,

	EMcSnapshotMethod__Count,
};

//.............................................................................

class CMcSnapshotClassType: public CClassType
{
	friend class CTypeMgr;

protected:
	CFunctionPtrType* m_pTargetType;
	CStructField* m_FieldArray [EMcSnapshotField__Count];
	CFunction* m_MethodArray [EMcSnapshotMethod__Count];

public:
	CMcSnapshotClassType ();

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
	GetField (EMcSnapshotField Field)
	{
		ASSERT (Field < EMcSnapshotField__Count);
		return m_FieldArray [Field];
	}

	CFunction* 
	GetMethod (EMcSnapshotMethod Method)
	{
		ASSERT (Method < EMcSnapshotMethod__Count);
		return m_MethodArray [Method];
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

	bool
	CompileCallMethod ();
};

//.............................................................................

// multicast snapshot returns function pointer with this closure:

struct TMcSnapshot: TIfaceHdr
{
	size_t m_Count;
	void* m_pPtrArray; // array of function closure or unsafe pointers
};

//.............................................................................

} // namespace jnc {
