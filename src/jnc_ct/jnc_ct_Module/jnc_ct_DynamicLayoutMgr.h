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
#include "jnc_DynamicLayout.h"

namespace jnc {
namespace ct {

//..............................................................................

class DynamicSection:
	public ModuleItem,
	public ModuleItemDecl {
	friend class DynamicLayoutMgr;

protected:
	DynamicSectionKind m_sectionKind;

public:
	DynamicSection() {
		m_itemKind = ModuleItemKind_DynamicSection;
		m_sectionKind = DynamicSectionKind_Undefined;
	}

	DynamicSectionKind
	getSectionKind() const {
		return m_sectionKind;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicDataSection: public DynamicSection {
	friend class DynamicLayoutMgr;
	friend class Parser;

protected:
	Value m_offsetValue;
	Type* m_type;
	uint_t m_ptrTypeFlags;

public:
	DynamicDataSection() {
		m_type = NULL;
	}

	const Value&
	getOffsetValue() const {
		return m_offsetValue;
	}

	Type*
	getType() const {
		return m_type;
	}

	uint_t
	getPtrTypeFlags() const {
		return m_ptrTypeFlags;
	}
};

//..............................................................................

class DynamicLayoutMgr {
	friend class Module;

protected:
	Module* m_module;
	sl::List<DynamicSection> m_sectionList;

public:
	DynamicLayoutMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	DynamicDataSection*
	createDataSection(
		DynamicSectionKind sectionKind,
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0
	);

	DynamicSection*
	createGroup(const sl::StringRef& name);
};

//..............................................................................

} // namespace ct
} // namespace jnc
