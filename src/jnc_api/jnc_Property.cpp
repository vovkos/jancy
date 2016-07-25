#include "pch.h"
#include "jnc_Property.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_Property_getItemDecl (jnc_Property* prop)
{
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getItemDeclFunc (prop);
}

JNC_EXTERN_C
jnc_Namespace*
jnc_Property_getNamespace (jnc_Property* prop)
{
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getNamespaceFunc (prop);
}

JNC_EXTERN_C
jnc_PropertyType*
jnc_Property_getType (jnc_Property* prop)
{
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getTypeFunc (prop);
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter (jnc_Property* prop)
{
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getGetterFunc (prop);
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getSetter (jnc_Property* prop)
{
	return jnc_g_dynamicExtensionLibHost->m_propertyFuncTable->m_getSetterFunc (prop);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_Property_getItemDecl (jnc_Property* prop)
{
	return prop;
}

JNC_EXTERN_C
jnc_Namespace*
jnc_Property_getNamespace (jnc_Property* prop)
{
	return prop;
}

JNC_EXTERN_C
jnc_PropertyType*
jnc_Property_getType (jnc_Property* prop)
{
	return prop->getType ();
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter (jnc_Property* prop)
{
	return prop->getGetter ();
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getSetter (jnc_Property* prop)
{
	jnc_Function* function = prop->getSetter ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no setter", prop->m_tag.cc ());
		return NULL;
	}

	return function;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................

