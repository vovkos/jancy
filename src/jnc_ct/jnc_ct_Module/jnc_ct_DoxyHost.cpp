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
#include "jnc_ct_DoxyHost.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

DoxyHost::DoxyHost()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

dox::Block*
DoxyHost::findItemBlock(handle_t item0)
{
	ModuleItem* item = (ModuleItem*) item0;
	return item->getDecl()->m_doxyBlock;
}

dox::Block*
DoxyHost::getItemBlock(handle_t item0)
{
	ModuleItem* item = (ModuleItem*) item0;
	return getItemBlock(item, item->getDecl());
}

void
DoxyHost::setItemBlock(
	handle_t item0,
	dox::Block* block
	)
{
	ModuleItem* item = (ModuleItem*) item0;
	setItemBlock(item, item->getDecl(), block);
}

dox::Block*
DoxyHost::getItemBlock(
	ModuleItem* item,
	ModuleItemDecl* itemDecl
	)
{
	if (!itemDecl->m_doxyBlock)
		itemDecl->m_doxyBlock = m_module->m_doxyModule.createBlock(item);

	return itemDecl->m_doxyBlock;
}

void
DoxyHost::setItemBlock(
	ModuleItem* item,
	ModuleItemDecl* itemDecl,
	dox::Block* block
	)
{
	itemDecl->m_doxyBlock = block;
	if (block)
		block->m_item = item;
}

sl::String
DoxyHost::createItemRefId(handle_t item)
{
	return ((ModuleItem*)item)->createDoxyRefId();
}

sl::StringRef
DoxyHost::getItemCompoundElementName(handle_t item)
{
	ModuleItemKind itemKind = ((ModuleItem*) item)->getItemKind();

	bool isCompoundFile =
		itemKind == ModuleItemKind_Namespace ||
		itemKind == ModuleItemKind_Type && ((Type*)item)->getTypeKind() != TypeKind_Enum;

	return isCompoundFile ? itemKind == ModuleItemKind_Namespace ? "innernamespace" : "innerclass" : NULL;
}

handle_t
DoxyHost::findItem(
	const sl::StringRef& name,
	size_t overloadIdx
	)
{
	ModuleItem* item = m_module->m_namespaceMgr.getGlobalNamespace()->findItemByName(name);
	if (!item)
		return NULL;

	if (overloadIdx && item->getItemKind() == ModuleItemKind_Function)
	{
		Function* overload = ((Function*)item)->getOverload(overloadIdx);
		if (overload)
			item = overload;
	}

	return item;
}

handle_t
DoxyHost::getCurrentNamespace()
{
	return m_module->m_namespaceMgr.getCurrentNamespace();
}

bool
DoxyHost::generateGlobalNamespaceDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	return m_module->m_namespaceMgr.getGlobalNamespace()->generateDocumentation(outputDir, itemXml, indexXml);
}

//..............................................................................

} // namespace jnc
} // namespace ct
