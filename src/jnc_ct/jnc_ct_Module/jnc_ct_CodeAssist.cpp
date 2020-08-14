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
#include "jnc_ct_CodeAssist.h"

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
	Function* function,
	size_t argumentIdx
	)
{
	CodeAssist* codeAssist = AXL_MEM_NEW(CodeAssist);
	codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	codeAssist->m_pos = pos;
	codeAssist->m_function = function;
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

} // namespace ct
} // namespace jnc
