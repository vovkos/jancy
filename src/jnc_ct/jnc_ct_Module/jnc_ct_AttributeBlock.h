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

#include "jnc_Variant.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

enum AttributeFlag {
	AttributeFlag_ValueReady   = 0x010000,
	AttributeFlag_VariantReady = 0x020000,
	AttributeFlag_Shared       = 0x040000,
	AttributeFlag_Dynamic      = 0x080000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AttributeBlockFlag {
	AttributeBlockFlag_ValuesReady = 0x010000,
	AttributeBlockFlag_Dynamic     = 0x080000, // same as AttributeFlag_Dynamic
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
	Variant m_variant;

public:
	Attribute() {
		m_itemKind = ModuleItemKind_Attribute;
		m_variant = g_nullVariant;
	}

	const Value&
	getValue() {
		ASSERT(m_flags & AttributeFlag_ValueReady);
		return m_value;
	}

	const Variant&
	getVariant() {
		ASSERT(m_flags & AttributeFlag_VariantReady);
		return m_variant;
	}

	bool
	ensureValueReady() {
		return (m_flags & AttributeFlag_ValueReady) || prepareValue();
	}

	void
	ensureVariantReady() {
		if (!(m_flags & AttributeFlag_VariantReady))
			prepareVariant();
	}

protected:
	bool
	prepareValue();

	void
	prepareVariant();
};

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

	~AttributeBlock() {
		if (m_flags & AttributeBlockFlag_Dynamic)
			deleteDynamicAttributes();
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

	void
	setDynamicAttributeValue(
		const sl::StringRef& name,
		const Variant& value
	);

protected:
	bool
	prepareAttributeValues();

	void
	deleteDynamicAttributes();
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
