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
#include "jnc_BitFieldType.h"

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
jnc_BitFieldType_getBaseType (jnc_BitFieldType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_bitFieldTypeFuncTable->m_getBaseTypeFunc (type);
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitOffset (jnc_BitFieldType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_bitFieldTypeFuncTable->m_getBitOffsetFunc (type);
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitCount (jnc_BitFieldType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_bitFieldTypeFuncTable->m_getBitCountFunc (type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Type*
jnc_BitFieldType_getBaseType (jnc_BitFieldType* type)
{
	return type->getBaseType ();
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitOffset (jnc_BitFieldType* type)
{
	return type->getBitOffset ();
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitCount (jnc_BitFieldType* type)
{
	return type->getBitCount ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
