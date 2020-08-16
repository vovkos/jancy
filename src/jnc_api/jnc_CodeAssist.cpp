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
#include "jnc_CodeAssist.h"

#if (_JNC_CORE)
#	include "jnc_ct_CodeAssist.h"
#endif

//..............................................................................

const char*
jnc_getCodeAssistKindString(jnc_CodeAssistKind kind)
{
	static const char* const stringTable[] =
	{
		"QuickInfoTip",
		"ArgumentTip",
		"AutoComplete",
		"AutoCompleteList",
		"GotoDefinition",
	};

	return (size_t)kind < countof(stringTable) ? stringTable[kind] : "?";
}

//..............................................................................

#if (_JNC_CORE)

JNC_EXTERN_C
JNC_EXPORT_O
jnc_CodeAssistKind
jnc_CodeAssist_getCodeAssistKind(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getCodeAssistKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_CodeAssist_getLine(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getLine();
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_CodeAssist_getCol(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getCol();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_CodeAssist_getModuleItem(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getModuleItem();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_CodeAssist_getFunctionType(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getFunctionType();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_CodeAssist_getArgumentIdx(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getArgumentIdx();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Namespace*
jnc_CodeAssist_getNamespace(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getNamespace();
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_CodeAssist_getNamespaceFlags(jnc_CodeAssist* codeAssistResult)
{
	return codeAssistResult->getNamespaceFlags();
}

#endif