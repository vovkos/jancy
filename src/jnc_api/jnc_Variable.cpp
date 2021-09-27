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
#include "jnc_Variable.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Variable.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
uint_t
jnc_Variable_getPtrTypeFlags(jnc_Variable* variable) {
	return jnc_g_dynamicExtensionLibHost->m_variableFuncTable->m_getPtrTypeFlagsFunc(variable);
}

JNC_EXTERN_C
bool_t
jnc_Variable_hasInitializer(jnc_Variable* variable) {
	return jnc_g_dynamicExtensionLibHost->m_variableFuncTable->m_hasInitializerFunc(variable);
}

JNC_EXTERN_C
const char*
jnc_Variable_getInitializerString_v(jnc_Variable* variable) {
	return jnc_g_dynamicExtensionLibHost->m_variableFuncTable->m_getInitializerStringFunc(variable);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_Variable_getPtrTypeFlags(jnc_Variable* variable) {
	return variable->getPtrTypeFlags();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variable_hasInitializer(jnc_Variable* variable) {
	return !variable->getInitializer().isEmpty();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Variable_getInitializerString_v(jnc_Variable* variable) {
	return *jnc::getTlsStringBuffer() = variable->getInitializerString();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
