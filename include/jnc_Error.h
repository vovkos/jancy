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

/**

\defgroup error-subsystem Error Subsystem
	\import{jnc_Error.h}

	This section describes types and facilities for setting and retrieving error information.

\addtogroup error-subsystem
@{

\struct jnc_Error
	\verbatim

	Opaque structure used as a handle to Jancy error-describing buffer.

	Use functions from the `Error Subsystem` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

#if (!defined _JNC_DYNAMIC_EXTENSION_LIB)

JNC_EXTERN_C
void
jnc_setErrorRouter(jnc_ErrorRouter* router);

#endif

/// Returns a pointer to the last error set in the context of the current thread.

JNC_EXTERN_C
const jnc_Error*
jnc_getLastError();

/**
	\subgroup
	\verbatim

	These functions set the last error to the TLS buffer of the current thread.

	* ``jnc_setError`` sets Jancy `jnc_Error` pointed to by ``error``;
	* ``jnc_setErrno`` sets POSIX *errno* identifed by ``code``;
	* ``jnc_setStringError`` sets string error described by a null-terminated string pointed to by ``string``.

	\endverbatim
*/

JNC_EXTERN_C
void
jnc_setError(const jnc_Error* error);

JNC_EXTERN_C
void
jnc_setErrno(int code);

JNC_EXTERN_C
void
jnc_setStringError(const char* string);

/**
	\subgroup
	\verbatim

	Creates and returns a human-readable description of the error.

	Suffix ``_v`` is used to denote the **volatile** nature of the returned pointer. The buffer will be overwritten by the very next call to any ``_v`` function invoked in the same thread. *Do NOT* save it to be re-used later; copy it to some buffer if it's necessary.

	``jnc_getLastErrorDescription_v`` is equivalent to:

	.. code-block:: c

		jnc_getErrorDescription_v(jnc_getLastError())

	.. rubric:: Sample:

	.. code-block:: c

		// try to compile some Jancy code, and it fails...

		printf("error: %s\n", jnc_getLastErrorDescription_v ());

	\endverbatim
*/

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v(const jnc_Error* error);

JNC_INLINE
const char*
jnc_getLastErrorDescription_v()
{
	return jnc_getErrorDescription_v(jnc_getLastError());
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

#if (!defined _JNC_DYNAMIC_EXTENSION_LIB)

inline
void
setErrorRouter(jnc_ErrorRouter* router)
{
	jnc_setErrorRouter(router);
}

#endif

inline
const jnc_Error*
getLastError()
{
	return jnc_getLastError();
}

inline
void
setError(const jnc_Error* error)
{
	jnc_setError(error);
}

inline
void
setErrno(int code)
{
	jnc_setErrno(code);
}

inline
void
setStringError(const char* string)
{
	jnc_setStringError(string);
}

inline
const char*
getErrorDescription_v(const jnc_Error* error)
{
	return jnc_getErrorDescription_v(error);
}

inline
const char*
getLastErrorDescription_v()
{
	return jnc_getLastErrorDescription_v();
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
