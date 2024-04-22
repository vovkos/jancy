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

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

enum AttributeFlag {
	AttributeFlag_ValueReady = 0x010000,
	AttributeFlag_Shared     = 0x020000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AttributeBlockFlag {
	AttributeBlockFlag_ValuesReady = 0x010000,
};

//..............................................................................

class Attribute:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class AttributeBlock;
	friend class AttributeMgr;

protected:
	Value m_value;

public:
	Attribute() {
		m_itemKind = ModuleItemKind_Attribute;
	}

	Attribute(
		Module* module,
		const sl::StringRef& name,
		const Value& value
	);

	const Value&
	getValue() {
		return m_value;
	}

	bool
	ensureValueReady() {
		return (m_flags & AttributeFlag_ValueReady) || prepareValue();
	}

protected:
	bool
	prepareValue();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Attribute::Attribute(
	Module* module,
	const sl::StringRef& name,
	const Value& value
) {
	m_itemKind = ModuleItemKind_Attribute;
	m_module = module;
	m_name = name;
	m_value = value;
}

//..............................................................................

class AttributeBlock:
	public ModuleItem,
	public ModuleItemDecl {
	friend class Parser;
	friend class AttributeMgr;

protected:
	sl::Array<Attribute*> m_attributeArray;
	sl::StringHashTable<Attribute*> m_attributeMap;

public:
	AttributeBlock() {
		m_itemKind = ModuleItemKind_AttributeBlock;
	}

	const sl::Array<Attribute*>&
	getAttributeArray() {
		ensureAttributeValuesReady();
		return m_attributeArray;
	}

	Attribute*
	findAttribute(const sl::StringRef& name);

	bool
	addAttribute(Attribute* attribute);

	void
	addAttributeBlock(AttributeBlock* attributeBlock);

	bool
	ensureAttributeValuesReady() {
		return (m_flags & AttributeBlockFlag_ValuesReady) || prepareAttributeValues();
	}

protected:
	bool
	prepareAttributeValues();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Attribute*
AttributeBlock::findAttribute(const sl::StringRef& name) {
	sl::StringHashTableIterator<Attribute*> it = m_attributeMap.find(name);
	if (!it)
		return NULL;

	ensureAttributeValuesReady();
	return it->m_value;
}

//..............................................................................

inline
Attribute*
ModuleItemDecl::findAttribute(const sl::StringRef& name) {
	return m_attributeBlock ? m_attributeBlock->findAttribute(name) : NULL;
}

inline
bool
ModuleItemDecl::ensureAttributeValuesReady() {
	return m_attributeBlock ? m_attributeBlock->ensureAttributeValuesReady() : true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
