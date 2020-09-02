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
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

//..............................................................................

class CodeAssist
{
	friend class CodeAssistMgr;

protected:
	CodeAssistKind m_codeAssistKind;
	size_t m_offset; // not necessarily the same as request offset
	Module* m_module;

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

	size_t
	getOffset()
	{
		return m_offset;
	}

	Module*
	getModule()
	{
		return m_module;
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
	friend class Module;

protected:
	Module* m_module;
	Module* m_cacheModule;
	CodeAssistKind m_codeAssistKind;
	CodeAssist* m_codeAssist;
	size_t m_offset;
	ModuleItem* m_containerItem;

	size_t m_autoCompleteOffset;
	Namespace* m_autoCompleteNamespace;
	QualifiedName m_autoCompletePrefix;

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
	generateCodeAssist();

	void
	prepareAutoCompleteFallback(size_t offset);

	CodeAssist*
	createQuickInfoTip(
		size_t offset,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_QuickInfoTip, offset, item);
	}

	CodeAssist*
	createGotoDefinition(
		size_t offset,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_GotoDefinition, offset, item);
	}

	CodeAssist*
	createAutoComplete(
		size_t offset,
		ModuleItem* item
		)
	{
		return createModuleItemCodeAssist(CodeAssistKind_AutoComplete, offset, item);
	}

	CodeAssist*
	createEmptyCodeAssist(size_t offset)
	{
		return createModuleItemCodeAssist(CodeAssistKind_Undefined, offset, NULL);
	}

	CodeAssist*
	createArgumentTip(
		size_t offset,
		FunctionType* functionType,
		size_t arugmentIdx
		);

	CodeAssist*
	createAutoCompleteList(
		size_t offset,
		Namespace* nspace,
		uint_t flags = 0
		);

	CodeAssist*
	createImportAutoCompleteList(size_t offset);

protected:
	void
	freeCodeAssist();

	CodeAssist*
	generateCodeAssistImpl(ModuleItem* item);

	CodeAssist*
	createModuleItemCodeAssist(
		CodeAssistKind kind,
		size_t offset,
		ModuleItem* item
		);

	CodeAssist*
	createAutoCompleteListFromPrefix();
};

//..............................................................................

} // namespace ct
} // namespace jnc
