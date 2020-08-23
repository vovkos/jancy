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
}

void
CodeAssistMgr::freeCodeAssist()
{
	if (m_codeAssist)
		AXL_MEM_DELETE(m_codeAssist);

	m_codeAssist = NULL;
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
		m_containerItem = NULL;
		((GlobalNamespace*)item)->ensureNamespaceReady();
		generateCodeAssist();
		break;

	case ModuleItemKind_Type:
		ASSERT(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named);
		m_containerItem = NULL;
		((NamedType*)item)->ensureNamespaceReady();
		generateCodeAssist();
		break;
	}

	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createModuleItemCodeAssist(
	CodeAssistKind kind,
	const lex::LineColOffset& pos,
	ModuleItem* item
	)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = kind;
	m_codeAssist->m_pos = pos;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_item = item;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createArgumentTip(
	const lex::LineColOffset& pos,
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	m_codeAssist->m_pos = pos;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_functionType = functionType;
	m_codeAssist->m_argumentIdx = argumentIdx;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createAutoCompleteList(
	const lex::LineColOffset& pos,
	Namespace* nspace,
	uint_t flags
	)
{
	freeCodeAssist();

	nspace->ensureNamespaceReady();

	if (nspace == m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc))
		nspace->parseLazyImports();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_AutoCompleteList;
	m_codeAssist->m_pos = pos;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_namespace = nspace;
	m_codeAssist->m_namespaceFlags = flags;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createImportAutoCompleteList(const lex::LineColOffset& pos)
{
	freeCodeAssist();

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ImportAutoCompleteList;
	m_codeAssist->m_pos = pos;
	m_codeAssist->m_module = m_module;
	return m_codeAssist;
}

//..............................................................................

} // namespace ct
} // namespace jnc
