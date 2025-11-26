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
	if (index != ModuleItemStringKind_Synopsis)
		return createItemStringImpl(index, this);

	sl::String synopsis = "typedef ";
	synopsis += createSynopsisImpl(this, m_type, 0);
	return synopsis;
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

/*
void
TypedefShadowType::prepareDoxyLinkedText() {
	Unit* unit = m_typedef->getParentUnit();
	if (!unit || unit->getLib()) { // don't reference imported libraries
		Type::prepareDoxyLinkedText();
		return;
	}

	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(m_typedef);
	sl::String refId = doxyBlock->getRefId();
	getTypeStringTuple()->m_doxyLinkedTextPrefix.format(
		"<ref refid=\"%s\">%s</ref>",
		refId.sz(),
		getItemName().sz()
	);
}
*/

//..............................................................................

} // namespace ct
} // namespace jnc
