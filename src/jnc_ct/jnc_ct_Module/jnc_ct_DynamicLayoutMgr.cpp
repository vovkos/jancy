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

#include "pch.h"
#include "jnc_ct_DynamicLayoutMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
DynamicLayoutDataSection::createItemString(size_t index) {
	Type* type;
	switch (m_sectionKind) {
	case DynamicLayoutSectionKind_Array:
		type = m_type->getArrayType(-1);
		break;

	case DynamicLayoutSectionKind_Field:
		type = m_type;
		break;

	default:
		return createItemStringImpl(index, this);
	}

	sl::StringRef string = createItemStringImpl(index, this, type, m_ptrTypeFlags);
	return index != ModuleItemStringKind_Synopsis ?
		string :
		sl::StringRef("dyfield " + string);
}

//..............................................................................

DynamicLayoutMgr::DynamicLayoutMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

void
DynamicLayoutMgr::clear() {
	m_sectionList.clear();
}

DynamicLayoutDataSection*
DynamicLayoutMgr::createDataSection(
	DynamicLayoutSectionKind sectionKind,
	const sl::StringRef& name,
	Type* type,
	uint_t ptrTypeFlags
) {
	DynamicLayoutDataSection* section = new DynamicLayoutDataSection;
	section->m_sectionKind = sectionKind;
	section->m_name = name;
	section->m_storageKind = StorageKind_DynamicField;
	section->m_type = type;
	section->m_ptrTypeFlags = ptrTypeFlags;
	m_sectionList.insertTail(section);
	return section;
}

DynamicLayoutSection*
DynamicLayoutMgr::createGroup(const sl::StringRef& name) {
	DynamicLayoutSection* section = new DynamicLayoutSection;
	section->m_sectionKind = DynamicLayoutSectionKind_Group;
	section->m_name = name;
	section->m_storageKind = StorageKind_DynamicField;
	m_sectionList.insertTail(section);
	return section;
}

//..............................................................................

} // namespace ct
} // namespace jnc
