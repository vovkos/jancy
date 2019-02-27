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
#include "jnc_ct_Alias.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

Alias::Alias()
{
	m_itemKind = ModuleItemKind_Alias;
	m_targetItem = NULL;
	m_type = NULL;
	m_ptrTypeFlags = 0;
}

bool
Alias::calcLayout()
{
	bool result;

	Parser parser(m_module);
	result = parser.parseTokenList(SymbolKind_qualified_name_save_name, m_initializer);
	if (!result)
		return false;

	m_targetItem = m_parentNamespace->findItemTraverse(parser.m_qualifiedName);
	if (!m_targetItem)
	{
		err::setFormatStringError("name '%s' is not found", parser.m_qualifiedName.getFullName ().sz ());
		return false;
	}

	if (m_targetItem->getItemKind() == ModuleItemKind_Alias)
	{
		result = m_targetItem->ensureLayout();
		if (!result)
			return false;

		m_targetItem = ((Alias*)m_targetItem)->getTargetItem();
		ASSERT(m_targetItem->getItemKind() != ModuleItemKind_Alias);
	}

	m_parentNamespace->replaceItem(m_name, m_targetItem);
	return true;
}

bool
Alias::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock();

	itemXml->format("<memberdef kind='alias' id='%s'", doxyBlock->getRefId ().sz ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString (m_accessKind));

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz ());

	itemXml->appendFormat(
		"<initializer>= %s</initializer>\n",
		getInitializerString().sz()
		);

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
