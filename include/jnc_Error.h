#pragma once

typedef struct jnc_Error jnc_Error;

//.............................................................................

jnc_Error*
jnc_getLastError ();

void
jnc_setError (jnc_Error* error);

void
jnc_propagateLastError ();

size_t
jnc_getErrorDescription (
	jnc_Error* error,
	char* buffer,
	size_t bufferSize
	);

size_t
jnc_getLastErrorDescription (
	char* buffer,
	size_t bufferSize
	);

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

//.............................................................................

} // namespace jnc

#endif // __cplusplus
