#include "pch.h"
#include "jnc_Function.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_ct_Function.h"
#	include "jnc_ct_MulticastClassType.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Function_getOverload (
	jnc_Function* function,
	size_t overloadIdx
	)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getOverloadFunc (function, overloadIdx);
}

JNC_EXTERN_C
void*
jnc_Function_getMachineCode (jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getMachineCodeFunc (function);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Function_getOverload (
	jnc_Function* function,
	size_t overloadIdx
	)
{
	jnc_Function* overload = function->getOverload (overloadIdx);
	if (!overload)
	{
		err::setFormatStringError ("'%s' has no overload #%d", function->getQualifiedName ().cc (), overloadIdx);
		return NULL;
	}

	return overload;
}

JNC_EXTERN_C
void*
jnc_Function_getMachineCode (jnc_Function* function)
{
	return function->getMachineCode ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
