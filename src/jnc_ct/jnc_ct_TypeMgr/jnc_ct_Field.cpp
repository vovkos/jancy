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
#include "jnc_ct_Field.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Field::Field() {
	m_itemKind = ModuleItemKind_Field;
	m_type = NULL;
	m_offset = 0;
	m_bitOffset = 0;
	m_bitCount = 0;
	m_ptrTypeFlags = 0;
	m_llvmIndex = -1;
}

DataPtrType*
Field::getDataPtrType(
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT(!m_bitCount == !(flags & PtrTypeFlag_BitField));

	return m_bitCount ?
		m_module->m_typeMgr.getDataPtrType(
			m_type,
			m_bitOffset,
			m_bitCount,
			typeKind,
			ptrTypeKind,
			flags
		) :
	 	m_module->m_typeMgr.getDataPtrType(
			m_type,
			typeKind,
			ptrTypeKind,
			flags
		);
}

bool
Field::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	bool result = m_type->ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	bool isMulticast = isClassType(m_type, ClassTypeKind_Multicast);
	const char* kind = isMulticast ? "event" : "variable";

	itemXml->format("<memberdef kind='%s' id='%s'", kind, doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append(" static='yes'");
	else if (m_storageKind == StorageKind_Tls)
		itemXml->append(" tls='yes'");

	if (m_ptrTypeFlags & PtrTypeFlag_Const)
		itemXml->append(" const='yes'");
	else if (m_ptrTypeFlags & PtrTypeFlag_ReadOnly)
		itemXml->append(" readonly='yes'");

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());
	itemXml->append(m_type->getDoxyTypeString());

	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_ptrTypeFlags & ~PtrTypeFlag_DualEvent);
	if (!ptrTypeFlagString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", ptrTypeFlagString.sz());

	if (!m_initializer.isEmpty())
		itemXml->appendFormat("<initializer>= %s</initializer>\n", getInitializerString_xml().sz());

	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
