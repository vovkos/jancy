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

JNC_EXTERN_C
void
jnc_setErrno (int code);

JNC_EXTERN_C
void
jnc_setStringError (const char* string);

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v (jnc_Error* error);

JNC_INLINE
const char*
jnc_getLastErrorDescription_v ()
{
	return jnc_getErrorDescription_v (jnc_getLastError ());
}

#if (defined _JNC_DYNAMIC_EXTENSION_LIB && defined _AXL_ERR_ERROR_H)
JNC_INLINE
void
jnc_propagateLastError ()
{
	jnc_setError ((jnc_Error*) axl::err::getLastError ().cp ());
}
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

inline
jnc_Error*
getLastError ()
{
	return jnc_getLastError ();
}

inline
void
setError (jnc_Error* error)
{
	jnc_setError (error);
}

inline
void
setErrno (int code)
{
	jnc_setErrno (code);
}

inline
void
setStringError (const char* string)
{
	jnc_setStringError (string);
}

inline
const char*
getErrorDescription_v (jnc_Error* error)
{
	return jnc_getErrorDescription_v (error);
}

inline
const char*
getLastErrorDescription_v ()
{
	return jnc_getLastErrorDescription_v ();
}

#if (defined _JNC_DYNAMIC_EXTENSION_LIB && defined _AXL_ERR_ERROR_H)
inline
void
propagateLastError ()
{
	jnc_propagateLastError ();
}
#endif

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
