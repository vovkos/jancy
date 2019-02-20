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
#include "jnc_FunctionType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getFunctionTypeFlagString (jnc_FunctionTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"vararg",     // jnc_FunctionTypeFlag_VarArg      = 0x010000,
		"errorcode",  // jnc_FunctionTypeFlag_ErrorCode   = 0x020000,
		"byval",      // jnc_FunctionTypeFlag_ByValArgs   = 0x040000,
		"coerced",    // jnc_FunctionTypeFlag_CoercedArgs = 0x040000,
		"unsafe",     // jnc_FunctionTypeFlag_Unsafe      = 0x100000,
		"async",      // jnc_FunctionTypeFlag_Async       = 0x200000,
	};

	size_t i = axl::sl::getLoBitIdx32 (flag >> 16);
	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-function-flag";
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_FunctionType_getReturnType (jnc_FunctionType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getReturnTypeFunc (type);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_FunctionType_getArgCount (jnc_FunctionType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getArgCountFunc (type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionArg*
jnc_FunctionType_getArg (
	jnc_FunctionType* type,
	size_t index
	)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getArgFunc (type, index);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrType*
jnc_FunctionType_getFunctionPtrType (
	jnc_FunctionType* type,
	jnc_FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getFunctionPtrTypeFunc (type, ptrTypeKind, flags);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_FunctionType_getShortType (jnc_FunctionType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getShortTypeFunc (type);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_getPtrTypeKind (jnc_FunctionPtrType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionPtrTypeFuncTable->m_getPtrTypeKindFunc (type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_FunctionPtrType_getTargetType (jnc_FunctionPtrType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionPtrTypeFuncTable->m_getTargetTypeFunc (type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_FunctionArg_hasDefaultValue (jnc_FunctionArg* arg)
{
	return !arg->getInitializer ().isEmpty ();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_FunctionArg_getDefaultValueString_v (jnc_FunctionArg* arg)
{
	return *jnc::getTlsStringBuffer () = arg->getInitializerString ();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_FunctionType_getReturnType (jnc_FunctionType* type)
{
	return type->getReturnType ();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_FunctionType_getArgCount (jnc_FunctionType* type)
{
	return type->getArgArray ().getCount ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionArg*
jnc_FunctionType_getArg (
	jnc_FunctionType* type,
	size_t index
	)
{
	return type->getArgArray () [index];
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_getPtrTypeKind (jnc_FunctionPtrType* type)
{
	return type->getPtrTypeKind ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_FunctionPtrType_getTargetType (jnc_FunctionPtrType* type)
{
	return type->getTargetType ();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionPtrType*
jnc_FunctionType_getFunctionPtrType (
	jnc_FunctionType* type,
	jnc_FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return type->getFunctionPtrType (ptrTypeKind, flags);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionType*
jnc_FunctionType_getShortType (jnc_FunctionType* type)
{
	return type->getShortType ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
