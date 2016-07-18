#pragma once

#include "jnc_Def.h"

#define _JNC_ERROR_H

//.............................................................................

JNC_EXTERN_C
jnc_Error*
jnc_getLastError ();

JNC_EXTERN_C
void
jnc_setError (jnc_Error* error);

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
inline
void
jnc_propagateLastError ()
{
	jnc_setError (jnc_getLastError ());
}
#endif

JNC_EXTERN_C
size_t
jnc_getErrorDescription (
	jnc_Error* error,
	char* buffer,
	size_t bufferSize
	);

inline
size_t
jnc_getLastErrorDescription (
	char* buffer,
	size_t bufferSize
	)
{
	return jnc_getErrorDescription (jnc_getLastError (), buffer, bufferSize);
}

//.............................................................................
