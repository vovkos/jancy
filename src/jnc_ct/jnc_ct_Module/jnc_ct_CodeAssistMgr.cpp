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

namespace jnc {
namespace ct {

//..............................................................................

CodeAssist*
CodeAssist::createModuleItemCodeAssist(
	CodeAssistKind kind,
	const lex::LineColOffset& pos,
	ModuleItem* item
	)
{
	CodeAssist* codeAssist = AXL_MEM_NEW(CodeAssist);
	codeAssist->m_codeAssistKind = kind;
	codeAssist->m_pos = pos;
	codeAssist->m_item = item;
	return codeAssist;
}

CodeAssist*
CodeAssist::createArgumentTip(
	const lex::LineColOffset& pos,
	FunctionType* functionType,
	size_t argumentIdx
	)
{
	CodeAssist* codeAssist = AXL_MEM_NEW(CodeAssist);
	codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	codeAssist->m_pos = pos;
	codeAssist->m_functionType = functionType;
	codeAssist->m_argumentIdx = argumentIdx;
	return codeAssist;
}

CodeAssist*
CodeAssist::createAutoCompleteList(
	const lex::LineColOffset& pos,
	Namespace* nspace,
	uint_t flags
	)
{
	CodeAssist* codeAssist = AXL_MEM_NEW(CodeAssist);
	codeAssist->m_codeAssistKind = CodeAssistKind_AutoCompleteList;
	codeAssist->m_pos = pos;
	codeAssist->m_namespace = nspace;
	codeAssist->m_namespaceFlags = flags;
	return codeAssist;
}

//..............................................................................

CodeAssistMgr::CodeAssistMgr()
{
	m_codeAssistKind = CodeAssistKind_Undefined;
	m_codeAssistCacheModule = NULL;
	m_codeAssistOffset = -1;
	m_codeAssistContainerItem = NULL;
	m_codeAssist = NULL;
}

void
CodeAssistMgr::clear()
{
	if (m_codeAssist)
		AXL_MEM_DELETE(m_codeAssist);

	m_codeAssistKind = CodeAssistKind_Undefined;
	m_codeAssistCacheModule = NULL;
	m_codeAssistOffset = -1;
	m_codeAssistContainerItem = NULL;
	m_codeAssist = NULL;
}

CodeAssist*
CodeAssistMgr::generateCodeAssist(
	jnc_CodeAssistKind kind,
	Module* cacheModule,
	size_t offset,
	const sl::StringRef& source
	)
{
	initialize("code-assist-module", ModuleCompileFlag_DisableCodeGen);

	m_codeAssistKind = kind;
	m_codeAssistOffset = offset;
	m_codeAssistCacheModule = cacheModule;

	parse("code-assist-source", source);

	if (m_codeAssist)
		return m_codeAssist;

	if (m_codeAssistContainerItem)
		generateCodeAssistImpl(m_codeAssistContainerItem);

	return m_codeAssist;
}

void
CodeAssistMgr::generateCodeAssistImpl(ModuleItem* item)
{
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		m_codeAssistContainerItem = NULL;
		((GlobalNamespace*)item)->ensureNamespaceReady();

		if (m_codeAssistContainerItem)
			generateCodeAssistImpl(m_codeAssistContainerItem);

		break;

	case ModuleItemKind_Function:
		((Function*)item)->compile();
		break;

	case ModuleItemKind_Type:
		break;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
