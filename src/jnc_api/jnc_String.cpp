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
#include "jnc_String.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_ct_ArrayType.h"
#	include "jnc_rt_Runtime.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_String_isEqual(
	const jnc_String* string1,
	const jnc_String* string2
) {
	ASSERT(
		(!string1->m_ptr.m_validator ||
		(char*)string1->m_ptr.m_p + string1->m_length <= (char*)string1->m_ptr.m_validator->m_rangeEnd) &&
		(!string2->m_ptr.m_validator ||
		(char*)string2->m_ptr.m_p + string2->m_length <= (char*)string2->m_ptr.m_validator->m_rangeEnd)
	);

	return
		string1->m_length == string2->m_length &&
		memcmp(string1->m_ptr.m_p, string2->m_ptr.m_p, string1->m_length) == 0;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_String_hash(const jnc_String* string) {
	ASSERT(
		!string->m_ptr.m_validator ||
		(char*)string->m_ptr.m_p + string->m_length <= (char*)string->m_ptr.m_validator->m_rangeEnd
	);

	return sl::djb2(string->m_ptr.m_p, string->m_length);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_String_sz(const jnc_String* string) {
	if (!string->m_ptr_sz.m_p)
		((jnc_String*)string)->m_ptr_sz = string->m_length ?
			jnc_strDup((char*)string->m_ptr.m_p, string->m_length) :
			jnc_createForeignBufferPtr("", 0, false);

	return string->m_ptr_sz;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
