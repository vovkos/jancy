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

class Field:
	public ModuleItemWithDecl<>,
	public ModuleItemInitializer {
	friend class Parser;
	friend class TypeMgr;
	friend class MemberBlock;
	friend class DerivableType;
	friend class Property;
	friend class StructType;
	friend class UnionType;
	friend class ClassType;
	friend class VariableMgr;

protected:
	Type* m_type;
	size_t m_offset;
	uint_t m_bitOffset;
	uint_t m_bitCount;
	uint_t m_ptrTypeFlags;
	uint_t m_llvmIndex;

	sl::List<Token> m_constructor;

public:
	Field();

	Type*
	getType() {
		return m_type;
	}

	DerivableType*
	getParentType();

	size_t
	getOffset() {
		return m_offset;
	}

	uint_t
	getBitOffset() {
		return m_bitOffset;
	}

	uint_t
	getBitCount() {
		return m_bitCount;
	}

	uint_t
	getPtrTypeFlags() {
		return m_ptrTypeFlags;
	}

	uint_t
	getLlvmIndex() {
		return m_llvmIndex;
	}

	sl::List<Token>*
	getConstructor() {
		return &m_constructor;
	}

	DataPtrType*
	getDataPtrType(
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
	);

	virtual
	Type*
	getItemType() {
		return m_type;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

protected:
	virtual
	sl::StringRef
	createItemString(size_t index) {
		return createItemStringImpl(index, this, m_type, m_ptrTypeFlags);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Field::Field() {
	m_itemKind = ModuleItemKind_Field;
	m_type = NULL;
	m_offset = 0;
	m_bitOffset = 0;
	m_bitCount = 0;
	m_ptrTypeFlags = 0;
	m_llvmIndex = -1;
}

//..............................................................................

} // namespace ct
} // namespace jnc
