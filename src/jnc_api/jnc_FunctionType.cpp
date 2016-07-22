#include "pch.h"
#include "jnc_FunctionType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

JNC_EXTERN_C
const char*
jnc_getFunctionTypeFlagString (jnc_FunctionTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"vararg",     // jnc_FunctionTypeFlag_VarArg      = 0x010000,
		"errorcode",  // jnc_FunctionTypeFlag_ErrorCode   = 0x020000,
		"coerced",    // jnc_FunctionTypeFlag_CoercedArgs = 0x040000,
		"automaton",  // jnc_FunctionTypeFlag_Automaton   = 0x080000,
		"unsafe",     // jnc_FunctionTypeFlag_Unsafe      = 0x100000,
	};

	size_t i = sl::getLoBitIdx32 (flag >> 16);
	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-function-flag";
}

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB


JNC_EXTERN_C
jnc_Type*
jnc_FunctionType_getReturnType (jnc_FunctionType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getReturnTypeFunc (type);
}

JNC_EXTERN_C
size_t
jnc_FunctionType_getArgCount (jnc_FunctionType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionTypeFuncTable->m_getArgCountFunc (type);
}

JNC_EXTERN_C
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_getPtrTypeKind (jnc_FunctionPtrType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionPtrTypeFuncTable->m_getPtrTypeKindFunc (type);
}

JNC_EXTERN_C
jnc_FunctionType*
jnc_FunctionPtrType_getTargetType (jnc_FunctionPtrType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_functionPtrTypeFuncTable->m_getTargetTypeFunc (type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_FunctionArg_getItemDecl (jnc_FunctionArg* arg)
{
	return arg;
}

JNC_EXTERN_C
jnc_Type*
jnc_FunctionArg_getType (jnc_FunctionArg* arg)
{
	return arg->getType ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Type*
jnc_FunctionType_getReturnType (jnc_FunctionType* type)
{
	return type->getReturnType ();
}

JNC_EXTERN_C
size_t
jnc_FunctionType_getArgCount (jnc_FunctionType* type)
{
	return type->getArgArray ().getCount ();
}

JNC_EXTERN_C
jnc_FunctionArg*
jnc_FunctionType_getArg (
	jnc_FunctionType* type,
	size_t index	
	)
{
	return type->getArgArray () [index];
}

JNC_EXTERN_C
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_getPtrTypeKind (jnc_FunctionPtrType* type)
{
	return type->getPtrTypeKind ();
}

JNC_EXTERN_C
jnc_FunctionType*
jnc_FunctionPtrType_getTargetType (jnc_FunctionPtrType* type)
{
	return type->getTargetType ();
}


JNC_EXTERN_C
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
jnc_FunctionType*
jnc_FunctionType_getShortType (jnc_FunctionType* type)
{
	return type->getShortType ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
