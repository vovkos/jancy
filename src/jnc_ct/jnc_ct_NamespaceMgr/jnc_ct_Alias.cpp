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
}

bool
Alias::resolveImpl()
{
	bool result;

	ASSERT(!m_targetItem);

	if (m_flags & AliasFlag_InResolve)
	{
		err::setFormatStringError("can't resolve '%s' due to recursion", getQualifiedName().sz());
		return false;
	}

	m_flags |= AliasFlag_InResolve;

	Parser parser(m_module);
	result = parser.parseTokenList(SymbolKind_qualified_name_save_name, m_initializer);
	if (!result)
		return false;

	FindModuleItemResult findResult = m_parentNamespace->findItemTraverse(parser.getLastQualifiedName());
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("name '%s' is not found", parser.getLastQualifiedName().getFullName().sz());
		return false;
	}

	m_targetItem = findResult.m_item;
	if (m_targetItem->getItemKind() == ModuleItemKind_Alias)
	{
		Alias* alias = (Alias*)m_targetItem;
		result = alias->ensureResolved();
		if (!result)
			return false;

		m_targetItem = alias->getTargetItem();
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
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format("<memberdef kind='alias' id='%s'", doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());

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
