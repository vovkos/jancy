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

#include "jnc_DynamicLayoutSection.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

class DynamicLayoutSection: public ModuleItemWithDecl<> {
	friend class DynamicLayoutMgr;

protected:
	DynamicLayoutSectionKind m_sectionKind;

public:
	DynamicLayoutSection() {
		m_itemKind = ModuleItemKind_DynamicLayoutSection;
		m_sectionKind = DynamicLayoutSectionKind_Undefined;
	}

	DynamicLayoutSectionKind
	getSectionKind() const {
		return m_sectionKind;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicLayoutDataSection: public DynamicLayoutSection {
	friend class DynamicLayoutMgr;
	friend class Parser;

protected:
	Type* m_type;
	Value m_offsetValue;
	Value m_bitOffsetValue;
	uint_t m_bitCount;
	uint_t m_ptrTypeFlags;

public:
	DynamicLayoutDataSection();

	Type*
	getType() const {
		return m_type;
	}

	virtual
	Type*
	getItemType() {
		return m_type;
	}

	const Value&
	getOffsetValue() const {
		return m_offsetValue;
	}

	const Value&
	getBitOffsetValue() const {
		return m_bitOffsetValue;
	}

	uint_t
	getBitCount() const {
		return m_bitCount;
	}

	uint_t
	getPtrTypeFlags() const {
		return m_ptrTypeFlags;
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DynamicLayoutDataSection::DynamicLayoutDataSection() {
	m_type = NULL;
	m_bitCount = 0;
	m_ptrTypeFlags = 0;
}

//..............................................................................

class DynamicLayoutMgr {
	friend class Module;

protected:
	Module* m_module;
	sl::List<DynamicLayoutSection> m_sectionList;

public:
	DynamicLayoutMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	DynamicLayoutDataSection*
	createDataSection(
		DynamicLayoutSectionKind sectionKind,
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0
	);

	DynamicLayoutSection*
	createGroup(const sl::StringRef& name);
};

//..............................................................................

} // namespace ct
} // namespace jnc
