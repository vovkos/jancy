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
#include "jnc_Error.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
const jnc_Error*
jnc_getLastError ()
{
	return jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_getLastErrorFunc ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setError (const jnc_Error* error)
{
	jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_setErrorFunc (error);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setErrno (int code)
{
	jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_setErrnoFunc (code);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setStringError (const char* string)
{
	jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_setStringErrorFunc (string);
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getErrorDescription_v (const jnc_Error* error)
{
	return jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_getErrorDescriptionFunc (error);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
const jnc_Error*
jnc_getLastError ()
{
	return err::getLastError ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setError (const jnc_Error* error)
{
	err::setError (error);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setErrno (int code)
{
	err::setErrno (code);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setStringError (const char* string)
{
	err::setError (string);
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getErrorDescription_v (const jnc_Error* error)
{
	return *jnc::getTlsStringBuffer () = error->getDescription ();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_setErrorRouter (jnc_ErrorRouter* router)
{
	err::ErrorMgr* errorMgr = err::getErrorMgr ();
	if (router != errorMgr)
		errorMgr->setForwardRouter (router);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
