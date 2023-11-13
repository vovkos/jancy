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

#define _JNC_STRING_H

#include "jnc_RuntimeStructs.h"

/**

\defgroup string String
	\ingroup runtime-structs
	\import{jnc_String.h}

\addtogroup string
@{
*/

typedef struct jnc_String jnc_String;

//..............................................................................

JNC_INLINE
bool_t
jnc_String_isEmpty(const jnc_String* string);

JNC_EXTERN_C
bool_t
jnc_String_isEqual(
	const jnc_String* string,
	const jnc_String* string2
);

JNC_EXTERN_C
bool_t
jnc_String_isEqualIgnoreCase(
	const jnc_String* string,
	const jnc_String* string2
);

JNC_EXTERN_C
size_t
jnc_String_hash(const jnc_String* string);

JNC_EXTERN_C
size_t
jnc_String_hashIgnoreCase(const jnc_String* string);

JNC_EXTERN_C
jnc_DataPtr
jnc_String_sz(const jnc_String* string);

JNC_INLINE
jnc_DataPtr
jnc_String_szn(const jnc_String* string);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_String {
	jnc_DataPtr m_ptr;
	jnc_DataPtr m_ptr_sz;
	size_t m_length;

#ifdef __cplusplus
	bool
	isEmpty() const {
		return jnc_String_isEmpty(this) != 0;
	}

	bool
	isEqual(const jnc_String* string2) const {
		return jnc_String_isEqual(this, string2) != 0;
	}

	bool
	isEqualIgnoreCase(const jnc_String* string2) const {
		return jnc_String_isEqualIgnoreCase(this, string2) != 0;
	}

	size_t
	hash() const {
		return jnc_String_hash(this);
	}

	size_t
	hashIgnoreCase() const {
		return jnc_String_hashIgnoreCase(this);
	}

	jnc_DataPtr
	sz() const {
		return jnc_String_sz(this);
	}

	jnc_DataPtr
	szn() const {
		return jnc_String_szn(this);
	}
#endif // __cplusplus
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
bool_t
jnc_String_isEmpty(const jnc_String* string) {
	return !string->m_length;
}

JNC_INLINE
jnc_DataPtr
jnc_String_szn(const jnc_String* string) {
	return string->m_length ? jnc_String_sz(string) : jnc_g_nullDataPtr;
}

JNC_SELECT_ANY jnc_String jnc_g_nullString = { 0 };

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_String String;

const String g_nullString = jnc_g_nullString;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
