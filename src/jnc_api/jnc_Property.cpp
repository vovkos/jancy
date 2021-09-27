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
#include "jnc_Property.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Property.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter(jnc_Property* prop) {
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getGetterFunc(prop);
}

JNC_EXTERN_C
jnc_OverloadableFunction
jnc_Property_getSetter(jnc_Property* prop) {
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getSetterFunc(prop);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_Property_getGetter(jnc_Property* prop) {
	return prop->getGetter();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_Property_getSetter(jnc_Property* prop) {
	return prop->getSetter();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
