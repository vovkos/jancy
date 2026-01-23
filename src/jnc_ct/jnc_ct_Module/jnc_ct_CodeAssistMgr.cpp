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
#include "jnc_ct_AsyncLauncherFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CodeAssistMgr::CodeAssistMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_codeAssistKind = CodeAssistKind_Undefined;
	m_codeAssist = NULL;
	m_offset = -1;
	m_containerItem = NULL;
	m_fallbackMode = FallbackMode_None;
}

void
CodeAssistMgr::clear() {
	freeCodeAssist();
	m_codeAssistKind = CodeAssistKind_Undefined;
	m_codeAssist = NULL;
	m_offset = -1;
	m_containerItem = NULL;
	m_argumentTipStack.clear();

	m_fallbackMode = FallbackMode_None;
	m_fallbackToken.m_token = 0;
	m_fallbackNamePrefix.clear();
	m_fallbackExpression.clear();
}

void
CodeAssistMgr::initialize(
	jnc_CodeAssistKind kind,
	size_t offset
) {
	clear();

	m_codeAssistKind = kind;
	m_offset = offset;
}

CodeAssist*
CodeAssistMgr::generateCodeAssist() {
	if (m_codeAssist)
		return m_codeAssist;

	if (m_containerItem) {
		ModuleItem* item = m_containerItem;
		m_containerItem = NULL;
		generateCodeAssistImpl(item);
	}

	if (!m_codeAssist && m_fallbackMode)
		createFallbackCodeAssist();

	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::generateCodeAssistImpl(ModuleItem* item) {
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Orphan:
		item = ((Orphan*)item)->resolveForCodeAssist();
		if (item)
			generateCodeAssistImpl(item);
		break;

	case ModuleItemKind_Function:
		if (((Function*)item)->getType()->getFlags() & FunctionTypeFlag_Async)
			((AsyncLauncherFunction*)item)->generateCodeAssist();
		else
			((Function*)item)->compile();

		generateCodeAssist(); // process nested containers
		break;

	case ModuleItemKind_Namespace:
		((GlobalNamespace*)item)->ensureNamespaceReady();
		generateCodeAssist(); // process nested containers
		break;

	case ModuleItemKind_Type:
		ASSERT(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named);
		if (isClassType((Type*)item, ClassTypeKind_Reactor)) {
			generateCodeAssistImpl(((ReactorClassType*)item)->getReactor());
			break;
		}

		((NamedType*)item)->ensureNamespaceReady();
		generateCodeAssist(); // process nested containers
		break;
	}

	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createModuleItemCodeAssist(
	CodeAssistKind kind,
	size_t offset,
	ModuleItem* item
) {
	freeCodeAssist();

	m_codeAssist = new CodeAssist;
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
) {
	freeCodeAssist();

	m_codeAssist = new CodeAssist;
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_functionTypeOverload = functionType;
	m_codeAssist->m_argumentIdx = argumentIdx;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createArgumentTip(
	size_t offset,
	const FunctionTypeOverload& typeOverload,
	size_t argumentIdx
) {
	freeCodeAssist();

	size_t overloadCount = typeOverload.getOverloadCount();
	for (size_t i = 0; i < overloadCount; i++)
		typeOverload.getOverload(i)->ensureNoImports();

	m_codeAssist = new CodeAssist;
	m_codeAssist->m_codeAssistKind = CodeAssistKind_ArgumentTip;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_functionTypeOverload = typeOverload;
	m_codeAssist->m_argumentIdx = argumentIdx;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createArgumentTipFromStack() {
	if (m_argumentTipStack.isEmpty())
		return NULL;

	ArgumentTip* argumentTip = *m_argumentTipStack.getTail();

	size_t baseArgumentIdx;
	FunctionTypeOverload typeOverload = m_module->m_operatorMgr.getValueFunctionTypeOverload(
		argumentTip->m_value,
		&baseArgumentIdx
	);

	if (!typeOverload.getOverloadCount())
		return NULL;

	return createArgumentTip(
		argumentTip->m_pos.m_offset,
		typeOverload,
		baseArgumentIdx + argumentTip->m_argumentIdx
	);
}

CodeAssist*
CodeAssistMgr::createAutoComplete(
	size_t offset,
	Namespace* nspace,
	uint_t flags
) {
	freeCodeAssist();

	NamespaceKind nspaceKind = nspace->getNamespaceKind();
	if (nspaceKind == NamespaceKind_Type) {
		((NamedType*)nspace)->ensureLayout();
	} else {
		if (nspace == m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc))
			nspace->parseLazyImports();

		nspace->ensureNamespaceReady();
	}

	m_codeAssist = new CodeAssist;
	m_codeAssist->m_codeAssistKind = CodeAssistKind_AutoComplete;
	m_codeAssist->m_flags = flags;
	m_codeAssist->m_offset = offset;
	m_codeAssist->m_module = m_module;
	m_codeAssist->m_namespace = nspace;
	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createImportCodeAssist(size_t offset) {
	freeCodeAssist();

	switch (m_codeAssistKind) {
	case CodeAssistKind_AutoComplete:
		m_codeAssist = new CodeAssist;
		m_codeAssist->m_codeAssistKind = CodeAssistKind_ImportAutoComplete;
		m_codeAssist->m_offset = offset;
		m_codeAssist->m_module = m_module;
		break;

	case CodeAssistKind_QuickInfoTip:
		// full path?
		break;
	}

	return m_codeAssist;
}

CodeAssist*
CodeAssistMgr::createFallbackCodeAssist() {
	switch (m_fallbackMode) {
	case FallbackMode_QualifiedName:
		if (!m_fallbackNamePrefix.isEmpty()) {
			FindModuleItemResult findItemResult = m_fallbackContext.getParentNamespace()->findItemTraverse(
				m_fallbackContext,
				m_fallbackNamePrefix
			);

			Namespace* nspace = findItemResult.m_item ? findItemResult.m_item->getNamespace() : NULL;
			if (!nspace)
				return NULL;

			nspace->ensureNamespaceReady();

			switch (m_codeAssistKind) {
			case CodeAssistKind_AutoComplete:
				return createAutoComplete(
					m_fallbackToken.m_tokenKind == TokenKind_Identifier ?
						m_fallbackToken.m_pos.m_offset :
						m_fallbackToken.m_pos.m_offset + m_fallbackToken.m_pos.m_length,
					nspace,
					CodeAssistFlag_AutoCompleteFallback |
					CodeAssistFlag_QualifiedName
				);

			case CodeAssistKind_QuickInfoTip:
				if (m_fallbackToken.m_tokenKind != TokenKind_Identifier)
					return NULL;

				findItemResult = nspace->findItem(m_fallbackToken.m_data.m_string);
				return findItemResult.m_item ?
					createQuickInfoTip(m_fallbackToken.m_pos.m_offset, findItemResult.m_item) :
					NULL;

			case CodeAssistKind_GotoDefinition:
				// not yet

			default:
				return NULL;
			}
		} // else fall through (it's the first identifier of a qualified name)

	case FallbackMode_Identifier:
		ASSERT(m_fallbackToken.m_tokenKind == TokenKind_Identifier);
		switch (m_codeAssistKind) {
		case CodeAssistKind_AutoComplete:
			return createAutoComplete(
				m_fallbackToken.m_pos.m_offset,
				m_fallbackContext.getParentNamespace(),
				CodeAssistFlag_AutoCompleteFallback |
				CodeAssistFlag_IncludeParentNamespace
			);

		case CodeAssistKind_QuickInfoTip: {
			FindModuleItemResult findItemResult = m_fallbackContext.getParentNamespace()->findItem(m_fallbackToken.m_data.m_string);
			return findItemResult.m_item ?
				createQuickInfoTip(m_fallbackToken.m_pos.m_offset, findItemResult.m_item) :
				NULL;
			}

		case CodeAssistKind_GotoDefinition:
			// not yet

		default:
			return NULL;
		}

	case FallbackMode_Namespace:
		return m_codeAssistKind == CodeAssistKind_AutoComplete ?
			createAutoComplete(
				m_offset,
				m_fallbackContext.getParentNamespace(),
				CodeAssistFlag_AutoCompleteFallback |
				CodeAssistFlag_IncludeParentNamespace
			) :
			NULL;

	case FallbackMode_Expression:
		break;

	default:
		return NULL;
	}

	m_module->m_unitMgr.setCurrentUnit(m_module->m_unitMgr.getRootUnit());
	m_fallbackMode = FallbackMode_None;

	Value value;
	FunctionType* functionType = (FunctionType*)m_module->m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_module->m_functionMgr.createInternalFunction("jnci.expressionFallbackContainter", functionType);
	function->m_parentNamespace = m_fallbackContext.getParentNamespace();
	m_module->m_functionMgr.prologue(function, m_fallbackExpression.getHead()->m_pos);
	m_module->m_operatorMgr.parseExpression(&m_fallbackExpression, &value);
	m_module->m_functionMgr.epilogue();

	return
		m_codeAssist ? m_codeAssist :
		m_fallbackMode && m_fallbackMode != FallbackMode_Expression ? createFallbackCodeAssist() :
		NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
