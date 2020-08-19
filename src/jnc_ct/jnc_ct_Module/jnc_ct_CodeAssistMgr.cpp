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

CodeAssistMgr::CodeAssistMgr()
{
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
CodeAssistMgr::generateCodeAssist()
{
	if (!m_codeAssist && m_containerItem)
		generateCodeAssistImpl(m_containerItem);

	return m_codeAssist;
}

void
CodeAssistMgr::generateCodeAssistImpl(ModuleItem* item)
{
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		m_containerItem = NULL;
		((GlobalNamespace*)item)->ensureNamespaceReady();

		if (m_containerItem)
			generateCodeAssistImpl(m_containerItem);

		break;

	case ModuleItemKind_Function:
		((Function*)item)->compile();
		break;

	case ModuleItemKind_Type:
		break;
	}
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

	m_codeAssist = AXL_MEM_NEW(CodeAssist);
	m_codeAssist->m_codeAssistKind = CodeAssistKind_AutoCompleteList;
	m_codeAssist->m_pos = pos;
	m_codeAssist->m_namespace = nspace;
	m_codeAssist->m_namespaceFlags = flags;
	return m_codeAssist;
}

//..............................................................................

} // namespace ct
} // namespace jnc
