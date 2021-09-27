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

enum AttributeBlockFlag {
	AttributeBlockFlag_ValuesReady = 0x010000,
};

//..............................................................................

class Attribute:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class AttributeBlock;

protected:
	Value m_value;

public:
	Attribute() {
		m_itemKind = ModuleItemKind_Attribute;
	}

	const Value&
	getValue() {
		return m_value;
	}

	bool
	parseInitializer();
};

//..............................................................................

class AttributeBlock:
	public ModuleItem,
	public ModuleItemDecl {
	friend class AttributeMgr;

protected:
	sl::List<Attribute> m_attributeList;
	sl::Array<Attribute*> m_attributeArray;
	sl::StringHashTable<Attribute*> m_attributeMap;

public:
	AttributeBlock() {
		m_itemKind = ModuleItemKind_AttributeBlock;
	}

	const sl::Array<Attribute*>&
	getAttributeArray() {
		return m_attributeArray;
	}

	Attribute*
	findAttribute(const sl::StringRef& name) {
		sl::StringHashTableIterator<Attribute*> it = m_attributeMap.find(name);
		return it ? it->m_value : NULL;
	}

	Attribute*
	createAttribute(
		const sl::StringRef& name,
		sl::BoxList<Token>* initializer = NULL
	);

	bool
	ensureAttributeValuesReady() {
		return (m_flags & AttributeBlockFlag_ValuesReady) || prepareAttributeValues();
	}

protected:
	bool
	prepareAttributeValues();
};

//..............................................................................

inline
bool
ModuleItemDecl::ensureAttributeValuesReady() {
	return m_attributeBlock ? m_attributeBlock->ensureAttributeValuesReady() : true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
