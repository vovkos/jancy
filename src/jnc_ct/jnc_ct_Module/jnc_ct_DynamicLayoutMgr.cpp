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

DynamicLayoutMgr::DynamicLayoutMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

void
DynamicLayoutMgr::clear() {
	m_sectionList.clear();
}

DynamicDataSection*
DynamicLayoutMgr::createDataSection(
	DynamicSectionKind sectionKind,
	const sl::StringRef& name,
	Type* type,
	uint_t ptrTypeFlags
) {
	DynamicDataSection* section = new DynamicDataSection;
	section->m_sectionKind = sectionKind;
	section->m_name = name;
	section->m_qualifiedName = name;
	section->m_storageKind = StorageKind_DynamicField;
	section->m_type = type;
	section->m_ptrTypeFlags = ptrTypeFlags;
	m_sectionList.insertTail(section);
	return section;
}

DynamicSection*
DynamicLayoutMgr::createGroup(const sl::StringRef& name) {
	DynamicSection* section = new DynamicSection;
	section->m_sectionKind = DynamicSectionKind_Group;
	section->m_name = name;
	section->m_qualifiedName = name;
	section->m_storageKind = StorageKind_DynamicField;
	m_sectionList.insertTail(section);
	return section;
}

//..............................................................................

} // namespace ct
} // namespace jnc
