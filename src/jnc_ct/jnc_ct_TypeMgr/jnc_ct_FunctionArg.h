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

class Type;

//..............................................................................

class FunctionArg:
	public ModuleItemWithDecl<>,
	public ModuleItemInitializer {
	friend class TypeMgr;
	friend class Function;
	friend class FunctionMgr;
	friend class ClassType;
	friend class Orphan;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;

public:
	FunctionArg();

	Type*
	getType() {
		return m_type;
	}

	virtual
	Type*
	getItemType() {
		return m_type;
	}

	uint_t
	getPtrTypeFlags() {
		return m_ptrTypeFlags;
	}

	template <bool IsDoxyLinkedText>
	void
	appendArgString(sl::String* string);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
FunctionArg::FunctionArg() {
	m_itemKind = ModuleItemKind_FunctionArg;
	m_type = NULL;
	m_ptrTypeFlags = 0;
}

template <bool IsDoxyLinkedText>
void
FunctionArg::appendArgString(sl::String* string) {
	*string += IsDoxyLinkedText ?
		m_type->getDoxyLinkedTextPrefix() :
		m_type->getTypeStringPrefix();

	if (m_storageKind == StorageKind_This)
		*string += " this";
	else if (!m_name.isEmpty()) {
		*string += ' ';
		*string += m_name;
	}

	sl::StringRef suffix = IsDoxyLinkedText ?
		m_type->getDoxyLinkedTextSuffix() :
		m_type->getTypeStringSuffix();

	if (!suffix.isEmpty())
		*string += suffix;

	if (!m_initializer.isEmpty()) {
		*string += " = ";
		*string += getInitializerString();
	}
}

//..............................................................................

struct FunctionArgTuple: sl::ListLink {
	FunctionArg* m_argArray[2][2][2]; // this x const x volatile
};

//..............................................................................

} // namespace ct
} // namespace jnc
