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

#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

class BitFieldType: public Type {
	friend class TypeMgr;

protected:
	Type* m_baseType;
	size_t m_bitOffset;
	size_t m_bitCount;

public:
	BitFieldType();

	Type*
	getBaseType() {
		return m_baseType;
	}

	size_t
	getBitOffset() {
		return m_bitOffset;
	}

	size_t
	getBitCount() {
		return m_bitCount;
	}

	static
	sl::String
	createSignature(
		Type* baseType,
		size_t bitOffset,
		size_t bitCount
	) {
		return sl::formatString(
			"B%s:%d:%d",
			baseType->getSignature().sz(),
			bitOffset,
			bitOffset + bitCount
		);
	}

	virtual
	sl::StringRef
	getValueString(
		const void* p,
		const char* formatSpec
	);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_baseType, m_bitOffset, m_bitCount);
	}

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType() {
		m_llvmType = m_baseType->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType() {
		m_llvmDiType = m_baseType->getLlvmDiType();
	}

	virtual
	bool
	calcLayout();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_BitFieldType);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
