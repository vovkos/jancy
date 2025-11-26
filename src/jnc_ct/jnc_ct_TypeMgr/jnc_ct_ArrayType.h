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

#include "jnc_ArrayType.h"
#include "jnc_ct_Type.h"
#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class ArrayType: public Type {
	friend class TypeMgr;
	friend class Parser;

protected:
	Type* m_elementType;
	Type* m_rootType;
	size_t m_elementCount;

public:
	ArrayType();

	Type*
	getElementType() {
		return m_elementType;
	}

	Type*
	getRootType();

	size_t
	getElementCount() {
		return m_elementCount;
	}

	static
	sl::String
	createSignature(
		Type* elementType,
		size_t elementCount
	) {
		return sl::formatString(
			"A%d%s",
			elementCount,
			elementType->getSignature().sz()
		);
	}

	virtual
	sl::StringRef
	getValueString(
		const void* p,
		const char* formatSpec
	);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

protected:
	virtual
	bool
	resolveImports() {
		return m_elementType->ensureNoImports();
	}

	virtual
	bool
	calcLayout();

	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_elementType, m_elementCount);
		m_flags |= m_elementType->getFlags() & TypeFlag_SignatureMask;
	}

	virtual
	sl::StringRef
	createItemString(size_t index);

	sl::String
	createDimensionString();

	void
	prepareRootType();

	virtual
	void
	prepareLlvmType() {
		ASSERT(m_elementCount != -1);
		m_llvmType = llvm::ArrayType::get(m_elementType->getLlvmType(), m_elementCount);
	}

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_ArrayType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ArrayType::ArrayType() {
	m_typeKind = TypeKind_Array;
	m_flags = TypeFlag_StructRet;
	m_elementType = NULL;
	m_rootType = NULL;
	m_elementCount = -1;
}

inline
Type*
ArrayType::getRootType() {
	if (!m_rootType)
		prepareRootType();

	return m_rootType;
}

//..............................................................................

class UserArrayType:
	public ArrayType,
	public ModuleItemContext,
	public ModuleItemInitializer {

	friend class DeclTypeCalc;
public:
	UserArrayType() {
		m_flags |= ModuleItemFlag_User;
	}

protected:
	virtual
	bool
	calcLayout();
};

//..............................................................................

} // namespace ct
} // namespace jnc
