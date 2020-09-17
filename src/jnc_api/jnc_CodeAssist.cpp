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
#	include "jnc_ct_CodeAssistMgr.h"
#endif

//..............................................................................

#if (_JNC_CORE)

JNC_EXTERN_C
JNC_EXPORT_O
jnc_CodeAssistKind
jnc_CodeAssist_getCodeAssistKind(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getCodeAssistKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_CodeAssist_getFlags(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getFlags();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_CodeAssist_getOffset(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getOffset();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Module*
jnc_CodeAssist_getModule(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getModule();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_CodeAssist_getModuleItem(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getModuleItem();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_CodeAssist_getFunctionType(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getFunctionType();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_CodeAssist_getArgumentIdx(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getArgumentIdx();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Namespace*
jnc_CodeAssist_getNamespace(jnc_CodeAssist* codeAssist)
{
	return codeAssist->getNamespace();
}

#endif