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
#include "jnc_ct_Typedef.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
Typedef::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		return createItemStringImpl(index, this);

	case ModuleItemStringKind_Synopsis:
		break;
	}

	m_type->ensureNoImports();
	return "typedef " + createSynopsisImpl(this, m_type, 0);
}

bool
Typedef::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
) {
	bool result = m_type->ensureNoImports();
	if (!result)
		return false;

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format(
		"<memberdef kind='typedef' id='%s'>\n"
		"<name>%s</name>\n",
		doxyBlock->getRefId().sz(),
		m_name.sz()
	);

	itemXml->append(m_type->getDoxyTypeString());
	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");
	return true;
}

//..............................................................................

sl::StringRef
TypedefShadowType::createItemString(size_t index) {
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
	case ModuleItemStringKind_Synopsis:
		return m_typedef->getItemString(index);

	case TypeStringKind_Prefix:
		return m_typedef->getItemName();

	case TypeStringKind_DoxyLinkedTextPrefix: {
		Unit* unit = m_typedef->getParentUnit();
		if (!unit || unit->getLib()) // don't reference imported libraries
			return m_typedef->getItemName();

		dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(m_typedef);
		sl::String refId = doxyBlock->getRefId();
		return sl::formatString(
			"<ref refid=\"%s\">%s</ref>",
			refId.sz(),
			getItemName().sz()
		);
		}

	case TypeStringKind_DoxyTypeString:
		return sl::formatString(
			"<type>%s</type>",
			m_typedef->getItemString(TypeStringKind_DoxyLinkedTextPrefix).sz()
		);

	default:
		return sl::StringRef();
	}
}

bool
TypedefShadowType::calcLayout() {
	bool result = m_typedef->getType()->ensureLayout();
	if (!result)
		return false;

	Type* type = m_typedef->getType(); // fetch type *after* layout (due to potential named types fixups)
	m_flags |= (type->getFlags() & TypeFlag_Pod);
	m_size = type->getSize();
	m_alignment = type->getAlignment();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
