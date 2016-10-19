#include "pch.h"
#include "jnc_ArrayType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Type*
jnc_ArrayType_getElementType (jnc_ArrayType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_arrayTypeFuncTable->m_getElementTypeFunc (type);
}

JNC_EXTERN_C
size_t
jnc_ArrayType_getElementCount (jnc_ArrayType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_arrayTypeFuncTable->m_GetElementCountFunc (type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Type*
jnc_ArrayType_getElementType (jnc_ArrayType* type)
{
	return type->getElementType ();
}

JNC_EXTERN_C
size_t
jnc_ArrayType_getElementCount (jnc_ArrayType* type)
{
	return type->getElementCount ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
