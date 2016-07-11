#include "pch.h"
#include "jnc_Error.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_ct_DerivableType.h"
#	include "jnc_ct_ClassType.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Error*
jnc_getLastError ()
{
	return (jnc_Error*) (const err::ErrorHdr*) err::getLastError ();
}

JNC_EXTERN_C
void
jnc_setError (jnc_Error* error)
{
	jnc_g_dynamicExtensionLibHost->m_errorFuncTable->m_setErrorFunc (error);
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
	err::setError ((err::ErrorHdr*) error);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
size_t
jnc_getErrorDescription (
	jnc_Error* error,
	char* buffer,
	size_t bufferSize
	)
{
	sl::String description = ((err::ErrorHdr*) error)->getDescription ();
	size_t length = description.getLength ();

	if (bufferSize == 0)
		return length;

	size_t copySize = AXL_MIN (bufferSize, length);
	memcpy (buffer, description, copySize);
	if (copySize < bufferSize)
		buffer [copySize] = 0;

	return copySize;
}

//.............................................................................
