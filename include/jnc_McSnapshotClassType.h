// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"
#include "jnc_FunctionPtrType.h"

namespace jnc {

//.............................................................................

enum McSnapshotFieldKind
{
	McSnapshotFieldKind_Count,
	McSnapshotFieldKind_PtrArray,

	McSnapshotFieldKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum McSnapshotMethodKind
{
	McSnapshotMethodKind_Call,

	McSnapshotMethodKind__Count,
};

//.............................................................................

class McSnapshotClassType: public ClassType
{
	friend class TypeMgr;

protected:
	FunctionPtrType* m_targetType;
	StructField* m_fieldArray [McSnapshotFieldKind__Count];
	Function* m_methodArray [McSnapshotMethodKind__Count];

public:
	McSnapshotClassType ();

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
	getField (McSnapshotFieldKind field)
	{
		ASSERT (field < McSnapshotFieldKind__Count);
		return m_fieldArray [field];
	}

	Function* 
	getMethod (McSnapshotMethodKind method)
	{
		ASSERT (method < McSnapshotMethodKind__Count);
		return m_methodArray [method];
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
		void* p,
		GcHeap* gcHeap
		);	

protected:
	virtual 
	void
	prepareTypeString ();

	bool
	compileCallMethod ();
};

//.............................................................................

} // namespace jnc {
