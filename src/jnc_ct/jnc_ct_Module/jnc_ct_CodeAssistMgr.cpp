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
#include "jnc_ct_CodeAssistMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CodeAssist::CodeAssist()
{
	m_codeAssistKind = CodeAssistKind_Undefined;
	m_offset = 0;
	m_module = NULL;
	m_item = NULL;
	m_itemParam = 0;
}

//..............................................................................

CodeAssistMgr::CodeAssistMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_codeAssistKind = CodeAssistKind_Undefined;
	m_cacheModule = NULL;
	m_offset = -1;
	m_containerItem = NULL;
	m_codeAssist = NULL;
	m_autoCompleteOffset = -1;
	m_autoCompleteNamespace = NULL;
}

void
CodeAssistMgr::clear()
{
	freeCodeAssist();
	m_codeAssistKind = CodeAssistKind_Undefined;
	m_cacheModule = NULL;
	m_offset = -1;
	m_containerItem = NULL;
	m_codeAssist = NULL;

	m_autoCompleteOffset = -1;
	m_autoCompleteNamespace = NULL;
	m_autoCompletePrefix.clear();
}

void
CodeAssistMgr::initialize(
	jnc_CodeAssistKind kind,
	Module* cacheModule,
	size_t offset
	)
{
	clear();

	m_codeAssistKind = kind;
	m_cacheModule = cacheModule;
	m_offset = offset;
}

CodeAssist*
CodeAssistMgr::generateCodeAssist()
{
	if (m_codeAssist)
		return m_codeAssist;

	if (m_containerItem)
	{
		ModuleItem* item = m_containerItem;
		m_containerItem = NULL;
		generateCodeAssistImpl(item);
	}

	if (!m_codeAssist && m_autoCompleteNamespace)
		if (!m_autoCompletePrefix.isEmpty())
			createAutoCompleteListFromPrefix();
		else
			createAutoCompleteList(
				m_autoCompleteOffset,
				m_autoCompleteNamespace,
				CodeAssistNamespaceFlag_IncludeParentNamespace
				);

	return m_codeAssist;
}

void
CodeAssistMgr::freeCodeAssist()
{
	if (m_codeAssist)
		AXL_MEM_DELETE(m_codeAssist);

	m_codeAssist = NULL;
}

CodeAssist*
CodeAssistMgr::generateCodeAssistImpl(ModuleItem* item)
{
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Orphan:
		item = ((Orphan*)item)->resolveForCodeAssist();
		if (item)
			generateCodeAssistImpl(item);
		break;

	case ModuleItemKind_Function:
		((Function*)item)->compile();
		break;

	case ModuleItemKind_Namespace:
		((GlobalNamespace*)item)->ensureNamespaceReady();
		generateCodeAssist();
		break;

	case ModuleItemKind_Type:
		ASSERT(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named);
		((NamedType*)item)->ensureNamespaceReady();
		generateCodeAssist();
		break;
	}

	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createAutoCompleteListFromPrefix()
{
	ASSERT(m_codeAssistKind == CodeAssistKind_AutoCompleteList);
	ASSERT(m_autoCompleteOffset != -1);
	ASSERT(!m_autoCompletePrefix.isEmpty());

	FindModuleItemResult findItemResult = m_autoCompleteNamespace->findItemTraverse(m_autoCompletePrefix, NULL);
	if (!findItemResult.m_item)
		return NULL;

	Namespace* nspace = findItemResult.m_item->getNamespace();
	return nspace ? createAutoCompleteList(m_autoCompleteOffset, nspace) : NULL;
}

CodeAssist*
CodeAssistMgr::createModuleItemCodeAssist(
	CodeAssistKind kind,
	size_t offset,
	ModuleItem* item
	)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = kind;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_item = item;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createArgumentTip(
	size_t offset,
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_functionType = functionType;
	m_codeAssist->m_argumentIdx = argumentIdx;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createAutoCompleteList(
	size_t offset,
	Namespace* nspace,
	uint_t flags
	)
{
	freeCodeAssist();

	NamespaceKind nspaceKind = nspace->getNamespaceKind();
	if (nspaceKind == NamespaceKind_Type)
	{
		((NamedType*)nspace)->ensureLayout();
	}
	else
	{
		if (nspace == m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc))
			nspace->parseLazyImports();

		nspace->ensureNamespaceReady();
	}

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_AutoCompleteList;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_namespace = nspace;
	m_codeAssist->m_namespaceFlags = flags;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createImportAutoCompleteList(size_t offset)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ImportAutoCompleteList;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	return m_codeAssist;
}

//..............................................................................

} // namespace ct
} // namespace jnc
