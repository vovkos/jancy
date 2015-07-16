// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_McSnapshotClassType.h"
#include "jnc_RuntimeStructs.h"

namespace jnc {

//.............................................................................

enum MulticastFieldKind
{
	MulticastFieldKind_Lock,
	MulticastFieldKind_MaxCount,
	MulticastFieldKind_Count,
	MulticastFieldKind_PtrArray,
	MulticastFieldKind_HandleTable,

	MulticastFieldKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum MulticastMethodKind
{
	MulticastMethodKind_Clear,
	MulticastMethodKind_Set,
	MulticastMethodKind_Add,
	MulticastMethodKind_Remove,
	MulticastMethodKind_GetSnapshot,
	MulticastMethodKind_Call,
	MulticastMethodKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum MulticastMethodFlag
{
	MulticastMethodFlag_InaccessibleViaEventPtr = 0x010000,
};

//.............................................................................

class MulticastClassType: public ClassType
{
	friend class TypeMgr;

protected:
	FunctionPtrType* m_targetType;
	McSnapshotClassType* m_snapshotType;
	StructField* m_fieldArray [MulticastFieldKind__Count];
	Function* m_methodArray [MulticastMethodKind__Count];

	ClassPtrTypeTuple* m_eventClassPtrTypeTuple;

public:
	MulticastClassType ();

	FunctionPtrType*
	getTargetType ()
	{
		return m_targetType;
	}

	FunctionType*
	getFunctionType ()
	{
		return m_targetType->getTargetType ();
	}

	StructField*
	getField (MulticastFieldKind field)
	{
		ASSERT (field < MulticastFieldKind__Count);
		return m_fieldArray [field];
	}

	Function*
	getMethod (MulticastMethodKind method)
	{
		ASSERT (method < MulticastMethodKind__Count);
		return m_methodArray [method];
	}

	McSnapshotClassType*
	getSnapshotType ()
	{
		return m_snapshotType;
	}

	virtual
	bool
	compile ()
	{
		return
			ClassType::compile () &&
			compileCallMethod ();
	}

	virtual
	void
	markGcRoots (
		const void* p,
		GcHeap* gcHeap
		);

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	bool
	calcLayout ()
	{
		return
			ClassType::calcLayout () &&
			m_snapshotType->ensureLayout ();
	}

	bool
	compileCallMethod ();
};

//.............................................................................

// structures backing up multicast, e.g.:
// mutlicast OnFire ();

struct Multicast: IfaceHdr
{
	volatile intptr_t m_lock;
	size_t m_maxCount;
	size_t m_count;
	void* m_ptrArray; // array of function closure, weak or unsafe pointers
	void* m_handleTable;

	Function*
	getMethod (MulticastMethodKind method)
	{
		return ((MulticastClassType*) m_box->m_type)->getMethod (method);
	}

	void
	call ();

	void
	call (intptr_t a);

	void
	call (
		intptr_t a1,
		intptr_t a2
		);
};

//.............................................................................

} // namespace jnc {
