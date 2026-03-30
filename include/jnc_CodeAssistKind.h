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

//..............................................................................

enum jnc_CodeAssistKind {
	jnc_CodeAssistKind_Undefined,
	jnc_CodeAssistKind_QuickInfoTip,
	jnc_CodeAssistKind_ArgumentTip,
	jnc_CodeAssistKind_AutoComplete,
	jnc_CodeAssistKind_ImportAutoComplete,
	jnc_CodeAssistKind_GotoDefinition,
	jnc_CodeAssistKind__Count,
};

typedef enum jnc_CodeAssistKind jnc_CodeAssistKind;

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
	CodeAssistKind_GotoDefinition     = jnc_CodeAssistKind_GotoDefinition,
	CodeAssistKind__Count             = jnc_CodeAssistKind__Count;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
