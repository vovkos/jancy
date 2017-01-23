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

/*!

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

/// Returns a pointer to the last error set in the context of the current thread.

JNC_EXTERN_C
const jnc_Error*
jnc_getLastError ();

/// Sets ``error`` as the last error of the current thread.

JNC_EXTERN_C
void
jnc_setError (const jnc_Error* error);

/// Sets POSIX ``errno`` error as the last error of the current thread.

JNC_EXTERN_C
void
jnc_setErrno (int code);

/// Sets string error described by a null-terminated string pointed to by ``string`` as the last error of the current thread.

JNC_EXTERN_C
void
jnc_setStringError (const char* string);

/*!
	\verbatim

	Creates and returns a human-readable description of the error.

	Suffix ``_v`` is used to denote the **volatile** nature of the returned pointer. The buffer will be overwritten by the very next call to any ``_v`` function. *Do NOT* save it to be re-used later; copy it to some buffer if it's necessary.

	.. rubric:: Sample:

	.. code-block:: cpp

		const jnc_Error* error = jnc_getLastError ();
		printf ("error: %s\n", jnc_getErrorDescription_v ());

	\endverbatim
*/

JNC_EXTERN_C
const char*
jnc_getErrorDescription_v (const jnc_Error* error);

/*!
	\verbatim

	Returns a human readable description of the the last error set in the context of the current thread.

	Suffix ``_v`` is used to denote the **volatile** nature of the returned pointer. The buffer will be overwritten by the very next call to any ``_v`` function. *Do NOT* save it to be re-used later; copy it to some buffer if it's necessary.

	.. rubric:: Equivalent to:

	.. code-block:: cpp

		jnc_getErrorDescription_v (jnc_getLastError ())

	\endverbatim
*/

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
	jnc_setError (axl::err::getLastError ());
}
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

inline
const jnc_Error*
getLastError ()
{
	return jnc_getLastError ();
}

inline
void
setError (const jnc_Error* error)
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
getErrorDescription_v (const jnc_Error* error)
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
