#pragma once

#include "jnc_Def.h"

#define _JNC_ERROR_H

/// \addtogroup error-subsystem
/// @{

//..............................................................................

JNC_EXTERN_C
jnc_Error*
jnc_getLastError ();

JNC_EXTERN_C
void
jnc_setError (jnc_Error* error);

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
JNC_INLINE
void
jnc_propagateLastError ()
{
	jnc_setError (jnc_getLastError ());
}
#endif

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v (jnc_Error* error);

JNC_INLINE
const char*
jnc_getLastErrorDescription_v ()
{
	return jnc_getErrorDescription_v (jnc_getLastError ());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

JNC_INLINE
jnc_Error*
getLastError ()
{
	return jnc_getLastError ();
}

JNC_INLINE
void
setError (jnc_Error* error)
{
	jnc_setError (error);
}

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
JNC_INLINE
void
propagateLastError ()
{
	jnc_propagateLastError ();
}
#endif

JNC_INLINE
const char*
getErrorDescription_v (jnc_Error* error)
{
	return jnc_getErrorDescription_v (error);
}

JNC_INLINE
const char*
getLastErrorDescription_v ()
{
	return jnc_getLastErrorDescription_v ();
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
