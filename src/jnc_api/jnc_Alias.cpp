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
#include "jnc_Alias.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Alias.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Alias_getInitializerString_v(jnc_Alias* alias) {
	return *jnc::getTlsStringBuffer() = alias->getInitializerString();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Alias_isResolved(jnc_Alias* alias) {
	return alias->isResolved();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Alias_getTargetItem(jnc_Alias* alias) {
	return alias->getTargetItem();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
