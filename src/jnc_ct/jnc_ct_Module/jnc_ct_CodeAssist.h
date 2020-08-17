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

#pragma once

#include "jnc_CodeAssist.h"

namespace jnc {
namespace ct {

//..............................................................................

class CodeAssist
{
protected:
	CodeAssistKind m_codeAssistKind;
	lex::LineColOffset m_pos; // not necessarily the same as request position

	union
	{
		ModuleItem* m_item;
		FunctionType* m_functionType;
		Namespace* m_namespace;
	};

	union
	{
		size_t m_argumentIdx;
		uint_t m_namespaceFlags;
	};

protected:
	static
	CodeAssist*
	createModuleItemCodeAssist(
		CodeAssistKind kind,
		const lex::LineColOffset& pos,
		ModuleItem* item
		);

public:
	static
	CodeAssist*
	createQuickInfoTip(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_QuickInfoTip, pos, item);
	}

	static
	CodeAssist*
	createGotoDefinition(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_GotoDefinition, pos, item);
	}

	static
	CodeAssist*
	createAutoComplete(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_AutoComplete, pos, item);
	}

	static
	CodeAssist*
	createArgumentTip(
		const lex::LineColOffset& pos,
		FunctionType* functionType,
		size_t arugmentIdx
		);

	static
	CodeAssist*
	createAutoCompleteList(
		const lex::LineColOffset& pos,
		Namespace* nspace,
		uint_t flags = 0
		);

	CodeAssistKind
	getCodeAssistKind()
	{
		return m_codeAssistKind;
	}

	int
	getLine()
	{
		return m_pos.m_line;
	}

	int
	getCol()
	{
		return m_pos.m_col;
	}

	size_t
	getOffset()
	{
		return m_pos.m_offset;
	}

	ModuleItem*
	getModuleItem()
	{
		ASSERT(m_codeAssistKind == CodeAssistKind_QuickInfoTip || m_codeAssistKind == CodeAssistKind_GotoDefinition || m_codeAssistKind == CodeAssistKind_AutoComplete);
		return m_item;
	}

	FunctionType*
	getFunctionType()
	{
		ASSERT(m_codeAssistKind == CodeAssistKind_ArgumentTip);
		return m_functionType;
	}

	Namespace*
	getNamespace()
	{
		ASSERT(m_codeAssistKind == CodeAssistKind_AutoCompleteList);
		return m_namespace;
	}

	size_t
	getArgumentIdx()
	{
		ASSERT(m_codeAssistKind == CodeAssistKind_ArgumentTip);
		return m_argumentIdx;
	}

	uint_t
	getNamespaceFlags()
	{
		ASSERT(m_codeAssistKind == CodeAssistKind_AutoCompleteList);
		return m_namespaceFlags;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
