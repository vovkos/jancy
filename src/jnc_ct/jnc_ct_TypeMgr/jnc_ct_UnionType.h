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

#include "jnc_ct_StructType.h"

namespace jnc {
namespace ct {

class UnionType;

//..............................................................................

// union cannot be a child, but it can be a parent

class UnionType: public DerivableType {
	friend class TypeMgr;
	friend class Parser;

protected:
	StructType* m_structType;

public:
	UnionType();

	StructType*
	getStructType() {
		ASSERT(m_structType);
		return m_structType;
	}

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	) {
		ASSERT(false); // unions are POD and hence are never GC roots
	}

protected:
	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount,
		uint_t ptrTypeFlags,
		sl::List<Token>* constructor,
		sl::List<Token>* initializer
	);

	virtual
	void
	prepareSignature() {
		m_signature = 'U' + m_qualifiedName;
		m_flags |= TypeFlag_SignatureFinal;
	}

	virtual
	void
	prepareLlvmType() {
		m_llvmType = getStructType()->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_UnionType);
	}

	virtual
	bool
	calcLayout();
};

//..............................................................................

} // namespace ct
} // namespace jnc
