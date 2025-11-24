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

#include "jnc_AttributeBlock.h"
#include "jnc_ct_AttributeBlock.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

enum AttributeFlag {
	AttributeFlag_ValueReady   = 0x010000,
	AttributeFlag_VariantReady = 0x020000,
	AttributeFlag_Shared       = 0x040000,
	AttributeFlag_Dynamic      = 0x080000,
	AttributeFlag_DynamicValue = 0x100000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AttributeBlockFlag {
	AttributeBlockFlag_ValuesReady   = 0x010000,
	AttributeBlockFlag_Dynamic       = 0x080000, // same as AttributeFlag_Dynamic
	AttributeBlockFlag_DynamicValues = 0x100000, // same as AttributeFlag_DynamicValue
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
	getValue();

	const Variant&
	getVariant() {
		ensureVariantReady();
		return m_variant;
	}

	bool
	ensureValueReady() {
		return (m_flags & AttributeFlag_ValueReady) || prepareValue(false);
	}

	void
	ensureVariantReady() {
		if (!(m_flags & AttributeFlag_VariantReady))
			prepareVariant();
	}

protected:
	bool
	prepareValue(bool isDynamic);

	void
	prepareVariant();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const Value&
Attribute::getValue() {
	if (!(m_flags & AttributeFlag_ValueReady))
		TRACE("-- WARNING: accessing an unready attribute value\n");

	return m_value;
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

	~AttributeBlock() {
		if (m_flags & AttributeBlockFlag_Dynamic)
			deleteDynamicAttributes();
	}

	const sl::Array<Attribute*>&
	getAttributeArray() {
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
		return (m_flags & AttributeBlockFlag_ValuesReady) || prepareAttributeValues(false);
	}

	void
	setDynamicAttributeValue(
		size_t i,
		const Variant& value
	);

protected:
	bool
	prepareAttributeValues(bool isDynamic);

	void
	deleteDynamicAttributes();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Attribute*
AttributeBlock::findAttribute(const sl::StringRef& name) {
	return m_attributeMap.findValue(name, NULL);
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
