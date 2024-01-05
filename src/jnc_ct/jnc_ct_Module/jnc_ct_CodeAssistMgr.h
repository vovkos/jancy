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
#include "jnc_ct_FunctionTypeOverload.h"

namespace jnc {
namespace ct {

//..............................................................................

class CodeAssist {
	friend class CodeAssistMgr;

protected:
	CodeAssistKind m_codeAssistKind;
	uint_t m_flags;
	size_t m_offset; // not necessarily the same as request offset
	Module* m_module;
	FunctionTypeOverload m_functionTypeOverload;

	union {
		ModuleItem* m_item;
		Namespace* m_namespace;
	};

	union {
		size_t m_itemParam;
		size_t m_argumentIdx;
	};

public:
	CodeAssist();

	CodeAssistKind
	getCodeAssistKind() {
		return m_codeAssistKind;
	}

	uint_t
	getFlags() {
		return m_flags;
	}

	size_t
	getOffset() {
		return m_offset;
	}

	Module*
	getModule() {
		return m_module;
	}

	ModuleItem*
	getModuleItem() {
		ASSERT(m_codeAssistKind == CodeAssistKind_QuickInfoTip || m_codeAssistKind == CodeAssistKind_GotoDefinition);
		return m_item;
	}

	Namespace*
	getNamespace() {
		ASSERT(m_codeAssistKind == CodeAssistKind_AutoComplete);
		return m_namespace;
	}

	FunctionTypeOverload*
	getFunctionTypeOverload() {
		ASSERT(m_codeAssistKind == CodeAssistKind_ArgumentTip);
		return &m_functionTypeOverload;
	}

	size_t
	getArgumentIdx() {
		ASSERT(m_codeAssistKind == CodeAssistKind_ArgumentTip);
		return m_argumentIdx;
	}
};

//..............................................................................

class CodeAssistMgr {
	friend class Parser;
	friend class Module;

protected:
	struct AutoCompleteFallback {
		size_t m_offset;
		Namespace* m_namespace;
		QualifiedName m_prefix;
		uint_t m_flags;

		AutoCompleteFallback() {
			clear();
		}

		void
		clear();
	};

	struct ArgumentTip: sl::ListLink {
		lex::LineColOffset m_pos;
		Value m_value;
		size_t m_argumentIdx;

		ArgumentTip(
			const lex::LineColOffset& pos,
			const Value& value
		) {
			m_pos = pos;
			m_value = value;
			m_argumentIdx = 0;
		}
	};

protected:
	Module* m_module;
	Module* m_cacheModule;
	CodeAssistKind m_codeAssistKind;
	CodeAssist* m_codeAssist;
	size_t m_offset;
	ModuleItem* m_containerItem;
	AutoCompleteFallback m_autoCompleteFallback;
	sl::List<ArgumentTip> m_argumentTipStack;

public:
	CodeAssistMgr();

	~CodeAssistMgr() {
		freeCodeAssist();
	}

	CodeAssistKind
	getCodeAssistKind() {
		return m_codeAssistKind;
	}

	CodeAssist*
	getCodeAssist() {
		return m_codeAssist;
	}

	size_t
	getOffset() {
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
	) {
		return createModuleItemCodeAssist(CodeAssistKind_QuickInfoTip, offset, item);
	}

	CodeAssist*
	createGotoDefinition(
		size_t offset,
		ModuleItem* item
	) {
		return createModuleItemCodeAssist(CodeAssistKind_GotoDefinition, offset, item);
	}

	CodeAssist*
	createAutoComplete(
		size_t offset,
		ModuleItem* item
	) {
		return createModuleItemCodeAssist(CodeAssistKind_AutoComplete, offset, item);
	}

	CodeAssist*
	createEmptyCodeAssist(size_t offset) {
		return createModuleItemCodeAssist(CodeAssistKind_Undefined, offset, NULL);
	}

	CodeAssist*
	createArgumentTip(
		size_t offset,
		FunctionType* functionType,
		size_t arugmentIdx
	);

	CodeAssist*
	createArgumentTip(
		size_t offset,
		const FunctionTypeOverload& typeOverload,
		size_t argumentIdx
	);

	CodeAssist*
	createArgumentTipFromStack();

	CodeAssist*
	createAutoComplete(
		size_t offset,
		Namespace* nspace,
		uint_t flags = 0
	);

	CodeAssist*
	createImportAutoComplete(size_t offset);

	bool
	hasArgumentTipStack() {
		return !m_argumentTipStack.isEmpty();
	}

	void
	resetArgumentTipStack() {
		m_argumentTipStack.clear();
	}

	void
	resetArgumentTipStackIf(const lex::LineColOffset& pos) {
		if (!m_argumentTipStack.isEmpty() && pos.m_offset < m_offset)
			m_argumentTipStack.clear();
	}

	void
	pushArgumentTipIf(
		const lex::LineColOffset& pos,
		const Value& value
	) {
		if (m_codeAssistKind == CodeAssistKind_ArgumentTip && pos.m_offset < m_offset)
			m_argumentTipStack.insertTail(new ArgumentTip(pos, value));
	}

	void
	popArgumentTipIf(const lex::LineColOffset& pos) {
		if (!m_argumentTipStack.isEmpty() && pos.m_offset < m_offset)
			m_argumentTipStack.eraseTail();
	}

	void
	bumpArgumentTipIdxIf(const lex::LineColOffset& pos) {
		if (!m_argumentTipStack.isEmpty() && pos.m_offset < m_offset)
			m_argumentTipStack.getTail()->m_argumentIdx++;
	}

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
	createAutoCompleteFallback();
};

//..............................................................................

} // namespace ct
} // namespace jnc
