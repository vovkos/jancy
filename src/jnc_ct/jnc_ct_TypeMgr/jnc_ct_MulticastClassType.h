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

class MulticastClassType: public ClassType {
	friend class TypeMgr;

protected:
	class CallMethod: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
			return ((MulticastClassType*)m_parentNamespace)->compileCallMethod(this);
		}
	};

protected:
	FunctionPtrType* m_targetType;
	McSnapshotClassType* m_snapshotType;
	ClassPtrTypeTuple* m_eventClassPtrTypeTuple;

public:
	MulticastClassType();

	FunctionPtrType*
	getTargetType() const {
		return m_targetType;
	}

	FunctionType*
	getFunctionType() const {
		return m_targetType->getTargetType();
	}

	McSnapshotClassType*
	getSnapshotType() const {
		return m_snapshotType;
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	calcLayout();

	bool
	compileCallMethod(Function* function);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
MulticastClassType::MulticastClassType() {
	m_classTypeKind = ClassTypeKind_Multicast;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_targetType = NULL;
	m_snapshotType = NULL;
	m_eventClassPtrTypeTuple = NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
