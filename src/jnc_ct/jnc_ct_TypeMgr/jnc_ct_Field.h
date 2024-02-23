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
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class TypeMgr;
	friend class MemberBlock;
	friend class DerivableType;
	friend class Property;
	friend class StructType;
	friend class UnionType;
	friend class ClassType;

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
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
