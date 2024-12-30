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
#	include "jnc_rt_Runtime.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

inline
bool
isValidString(const jnc_String* string) {
	return
		!string->m_ptr.m_validator ||
		(char*)string->m_ptr.m_p >= (char*)string->m_ptr.m_validator->m_rangeBegin &&
		(char*)string->m_ptr.m_p + string->m_length <= (char*)string->m_ptr.m_validator->m_rangeEnd;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_String_isEqual(
	const jnc_String* string,
	const jnc_String* string2
) {
	ASSERT(isValidString(string) && isValidString(string2));

	sl::StringRef stringRef1((char*)string->m_ptr.m_p, string->m_length);
	sl::StringRef stringRef2((char*)string2->m_ptr.m_p, string2->m_length);
	return stringRef1.isEqual(stringRef2);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_String_isEqualIgnoreCase(
	const jnc_String* string,
	const jnc_String* string2
) {
	ASSERT(isValidString(string) && isValidString(string2));

	sl::StringRef stringRef1((char*)string->m_ptr.m_p, string->m_length);
	sl::StringRef stringRef2((char*)string2->m_ptr.m_p, string2->m_length);
	return stringRef1.isEqualIgnoreCase(stringRef2);
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_String_cmp(
	const jnc_String* string,
	const jnc_String* string2
) {
	ASSERT(isValidString(string) && isValidString(string2));

	sl::StringRef stringRef1((char*)string->m_ptr.m_p, string->m_length);
	sl::StringRef stringRef2((char*)string2->m_ptr.m_p, string2->m_length);
	return stringRef1.cmp(stringRef2);
}

JNC_EXTERN_C
JNC_EXPORT_O
int
jnc_String_cmpIgnoreCase(
	const jnc_String* string,
	const jnc_String* string2
) {
	ASSERT(isValidString(string) && isValidString(string2));

	sl::StringRef stringRef1((char*)string->m_ptr.m_p, string->m_length);
	sl::StringRef stringRef2((char*)string2->m_ptr.m_p, string2->m_length);
	return stringRef1.cmpIgnoreCase(stringRef2);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_String_hash(const jnc_String* string) {
	ASSERT(isValidString(string));

	sl::StringRef stringRef((char*)string->m_ptr.m_p, string->m_length);
	return stringRef.hash();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_String_hashIgnoreCase(const jnc_String* string) {
	ASSERT(isValidString(string));

	sl::StringRef stringRef((char*)string->m_ptr.m_p, string->m_length);
	return stringRef.hashIgnoreCase();
}

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_String_sz(const jnc_String* string) {
	return jnc_g_dynamicExtensionLibHost->m_stringFuncTable->m_szFunc(string);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_String_setPtr(
	jnc_String* string,
	jnc_DataPtr ptr,
	size_t length
) {
	return jnc_g_dynamicExtensionLibHost->m_stringFuncTable->m_setPtrFunc(string, ptr, length);
}

#else

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DataPtr
jnc_String_sz(const jnc_String* string) {
	if (!string->m_ptr_sz.m_p)
		((jnc_String*)string)->m_ptr_sz = string->m_length ?
			jnc::strDup((char*)string->m_ptr.m_p, string->m_length) :
			jnc::getCurrentThreadRuntime()->getModule()->m_constMgr.getEmptyLiteralPtr();

	return string->m_ptr_sz;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_String_setPtr(
	jnc_String* string,
	jnc_DataPtr ptr,
	size_t length
) {
	string->m_ptr = ptr;

	if (!ptr.m_validator ||
		ptr.m_p < ptr.m_validator->m_rangeBegin ||
		(ptr.m_validator->m_targetBox->m_flags & jnc_BoxFlag_Invalid)
	) {
		string->m_ptr_sz = jnc_g_nullDataPtr;
		string->m_length = 0;
		return;
	}

	char* p0 = (char*)ptr.m_p;
	char* end = (char*)ptr.m_validator->m_rangeEnd;

	if (length == -1) { // calculate length
		char* p = (char*)memchr(p0, 0, end - p0);
		if (p) {
			string->m_ptr_sz = ptr;
			string->m_length = p - p0;
		} else {
			string->m_ptr_sz = jnc::g_nullDataPtr;
			string->m_length = end - p0;
		}
	} else if (p0 + length < end) { // length is in-range
		string->m_ptr_sz = !p0[length] ? ptr : jnc::g_nullDataPtr;
		string->m_length = length;
	} else { // length is out-of-range
		string->m_ptr_sz = jnc::g_nullDataPtr;
		string->m_length = end - p0;
	}
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
