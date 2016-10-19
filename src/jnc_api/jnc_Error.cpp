#include "pch.h"
#include "jnc_Error.h"

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
jnc_Error*
jnc_getLastError ()
{
	return jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_getLastErrorFunc ();
}

JNC_EXTERN_C
void
jnc_setError (jnc_Error* error)
{
	jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_setErrorFunc (error);
}

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v (jnc_Error* error)
{
	return jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_getErrorDescriptionFunc (error);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Error*
jnc_getLastError ()
{
	return err::getLastError ();
}

JNC_EXTERN_C
void
jnc_setError (jnc_Error* error)
{
	err::setError (error);
}

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v (jnc_Error* error)
{
	return *jnc::getTlsStringBuffer () = error->getDescription ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
