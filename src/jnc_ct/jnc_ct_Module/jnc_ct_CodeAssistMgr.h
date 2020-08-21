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
	friend class CodeAssistMgr;

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
		size_t m_itemParam;
		size_t m_argumentIdx;
		uint_t m_namespaceFlags;
	};

public:
	CodeAssist();

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

class CodeAssistMgr
{
	friend class Parser;

protected:
	Module* m_module;
	Module* m_cacheModule;

	CodeAssistKind m_codeAssistKind;
	CodeAssist* m_codeAssist;
	size_t m_offset;
	ModuleItem* m_containerItem;

public:
	CodeAssistMgr();

	~CodeAssistMgr()
	{
		freeCodeAssist();
	}

	CodeAssistKind
	getCodeAssistKind()
	{
		return m_codeAssistKind;
	}

	CodeAssist*
	getCodeAssist()
	{
		return m_codeAssist;
	}

	size_t
	getOffset()
	{
		return m_offset;
	}

	void
	clear();

	void
	initialize(
		CodeAssistKind kind,
		Module* cacheModule,
		size_t offset
		);

	CodeAssist*
	generateCodeAssist()
	{
		return
			m_codeAssist ? m_codeAssist :
			m_containerItem ? generateCodeAssistImpl(m_containerItem) :
			NULL;
	}

	CodeAssist*
	createQuickInfoTip(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_QuickInfoTip, pos, item);
	}

	CodeAssist*
	createGotoDefinition(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_GotoDefinition, pos, item);
	}

	CodeAssist*
	createAutoComplete(
		const lex::LineColOffset& pos,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_AutoComplete, pos, item);
	}

	CodeAssist*
	createArgumentTip(
		const lex::LineColOffset& pos,
		FunctionType* functionType,
		size_t arugmentIdx
		);

	CodeAssist*
	createAutoCompleteList(
		const lex::LineColOffset& pos,
		Namespace* nspace,
		uint_t flags = 0
		);

protected:
	CodeAssist*
	createModuleItemCodeAssist(
		CodeAssistKind kind,
		const lex::LineColOffset& pos,
		ModuleItem* item
		);

	void
	freeCodeAssist();

	CodeAssist*
	generateCodeAssistImpl(ModuleItem* item);
};

} // namespace ct
} // namespace jnc
