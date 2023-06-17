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

#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ct {

class StructType;
class UnionType;
struct FmtLiteral;

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
	uint_t m_ptrTypeFlags;
	sl::List<Token> m_constructor;

	Type* m_bitFieldBaseType;
	size_t m_bitCount;
	size_t m_offset;

	union {
		uint_t m_llvmIndex;
		size_t m_prevDynamicFieldIndex;
	};

public:
	Field();

	Type*
	getType() {
		return m_type;
	}

	int
	getPtrTypeFlags() {
		return m_ptrTypeFlags;
	}

	sl::List<Token>*
	getConstructor() {
		return &m_constructor;
	}

	size_t
	getOffset() {
		return m_offset;
	}

	uint_t
	getLlvmIndex() {
		return m_llvmIndex;
	}

	size_t
	getPrevDynamicFieldIndex() {
		return m_prevDynamicFieldIndex;
	}

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
