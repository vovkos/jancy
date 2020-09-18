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

#include "jnc_ModuleItem.h"

//..............................................................................

enum jnc_CodeAssistKind
{
	jnc_CodeAssistKind_Undefined,
	jnc_CodeAssistKind_QuickInfoTip,
	jnc_CodeAssistKind_ArgumentTip,
	jnc_CodeAssistKind_AutoComplete,
	jnc_CodeAssistKind_ImportAutoComplete,
	jnc_CodeAssistKind_GotoDefinition,
};

typedef enum jnc_CodeAssistKind jnc_CodeAssistKind;

//..............................................................................

enum jnc_CodeAssistFlag
{
	jnc_CodeAssistFlag_IncludeParentNamespace = 0x01,
	jnc_CodeAssistFlag_AutoCompleteFallback   = 0x02,
	jnc_CodeAssistFlag_Declarator             = 0x04,
};

typedef enum jnc_CodeAssistFlag jnc_CodeAssistFlag;

//..............................................................................

JNC_EXTERN_C
jnc_CodeAssistKind
jnc_CodeAssist_getCodeAssistKind(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
uint_t
jnc_CodeAssist_getFlags(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
size_t
jnc_CodeAssist_getOffset(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
jnc_Module*
jnc_CodeAssist_getModule(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_CodeAssist_getModuleItem(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
jnc_FunctionType*
jnc_CodeAssist_getFunctionType(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
size_t
jnc_CodeAssist_getArgumentIdx(jnc_CodeAssist* codeAssist);

JNC_EXTERN_C
jnc_Namespace*
jnc_CodeAssist_getNamespace(jnc_CodeAssist* codeAssist);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_CodeAssist
{
	jnc_CodeAssistKind
	getCodeAssistKind()
	{
		return jnc_CodeAssist_getCodeAssistKind(this);
	}

	uint_t
	getOffset()
	{
		return jnc_CodeAssist_getOffset(this);
	}

	size_t
	getFlags()
	{
		return jnc_CodeAssist_getFlags(this);
	}

	jnc_Module*
	getModule()
	{
		return jnc_CodeAssist_getModule(this);
	}

	jnc_ModuleItem*
	getModuleItem()
	{
		return jnc_CodeAssist_getModuleItem(this);
	}

	jnc_FunctionType*
	getFunctionType()
	{
		return jnc_CodeAssist_getFunctionType(this);
	}

	size_t
	getArgumentIdx()
	{
		return jnc_CodeAssist_getArgumentIdx(this);
	}

	jnc_Namespace*
	getNamespace()
	{
		return jnc_CodeAssist_getNamespace(this);
	}
};
#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_CodeAssistKind CodeAssistKind;

const CodeAssistKind
	CodeAssistKind_Undefined          = jnc_CodeAssistKind_Undefined,
	CodeAssistKind_QuickInfoTip       = jnc_CodeAssistKind_QuickInfoTip,
	CodeAssistKind_ArgumentTip        = jnc_CodeAssistKind_ArgumentTip,
	CodeAssistKind_AutoComplete       = jnc_CodeAssistKind_AutoComplete,
	CodeAssistKind_ImportAutoComplete = jnc_CodeAssistKind_ImportAutoComplete,
	CodeAssistKind_GotoDefinition     = jnc_CodeAssistKind_GotoDefinition;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef jnc_CodeAssistFlag CodeAssistFlag;

const CodeAssistFlag
	CodeAssistFlag_IncludeParentNamespace = jnc_CodeAssistFlag_IncludeParentNamespace,
	CodeAssistFlag_AutoCompleteFallback   = jnc_CodeAssistFlag_AutoCompleteFallback,
	CodeAssistFlag_Declarator             = jnc_CodeAssistFlag_Declarator;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
