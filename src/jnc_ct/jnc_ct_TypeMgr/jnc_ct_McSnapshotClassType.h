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

#include "jnc_ct_ClassType.h"
#include "jnc_ct_FunctionPtrType.h"

namespace jnc {
namespace ct {

//..............................................................................

class McSnapshotClassType: public ClassType
{
	friend class TypeMgr;

protected:
	class CallMethod: public CompilableFunction
	{
	public:
		virtual
		bool
		compile()
		{
			return ((McSnapshotClassType*)m_parentNamespace)->compileCallMethod(this);
		}
	};

protected:
	FunctionPtrType* m_targetType;
	Field* m_fieldArray[McSnapshotFieldKind__Count];
	Function* m_methodArray[McSnapshotMethodKind__Count];

public:
	McSnapshotClassType();

	FunctionPtrType*
	getTargetType()
	{
		return m_targetType;
	}

	FunctionType*
	getFunctionType()
	{
		return m_targetType->getTargetType();
	}

	Field*
	getField(McSnapshotFieldKind field)
	{
		ASSERT(field < McSnapshotFieldKind__Count);
		return m_fieldArray[field];
	}

	Function*
	getMethod(McSnapshotMethodKind method)
	{
		ASSERT(method < McSnapshotMethodKind__Count);
		return m_methodArray[method];
	}

protected:
	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareDoxyTypeString();

	bool
	compileCallMethod(Function* function);
};

//..............................................................................

} // namespace ct
} // namespace jnc
