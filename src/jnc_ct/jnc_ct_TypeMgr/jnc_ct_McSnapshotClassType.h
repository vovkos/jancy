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

class McSnapshotClassType: public ClassType {
	friend class TypeMgr;

protected:
	class CallMethod: public CompilableFunction {
	public:
		virtual
		bool
		compile() {
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
	getTargetType() {
		return m_targetType;
	}

	FunctionType*
	getFunctionType() {
		return m_targetType->getTargetType();
	}

	Field*
	getField(McSnapshotFieldKind field) {
		ASSERT(field < McSnapshotFieldKind__Count);
		return m_fieldArray[field];
	}

	Function*
	getMethod(McSnapshotMethodKind method) {
		ASSERT(method < McSnapshotMethodKind__Count);
		return m_methodArray[method];
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	bool
	compileCallMethod(Function* function);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
McSnapshotClassType::McSnapshotClassType() {
	m_classTypeKind = ClassTypeKind_McSnapshot;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_targetType = NULL;
	memset(m_fieldArray, 0, sizeof(m_fieldArray));
	memset(m_methodArray, 0, sizeof(m_methodArray));
}

//..............................................................................

} // namespace ct
} // namespace jnc
