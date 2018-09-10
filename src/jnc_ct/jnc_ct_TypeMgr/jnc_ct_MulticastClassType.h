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

#include "jnc_ct_McSnapshotClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

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
	getTargetType () const
	{
		return m_targetType;
	}

	FunctionType*
	getFunctionType () const
	{
		return m_targetType->getTargetType ();
	}

	StructField*
	getField (MulticastFieldKind field) const
	{
		ASSERT (field < MulticastFieldKind__Count);
		return m_fieldArray [field];
	}

	Function*
	getMethod (MulticastMethodKind method) const
	{
		ASSERT (method < MulticastMethodKind__Count);
		return m_methodArray [method];
	}

	McSnapshotClassType*
	getSnapshotType () const
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

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareDoxyLinkedText ();

	virtual
	void
	prepareDoxyTypeString ();

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

//..............................................................................

} // namespace ct
} // namespace jnc
