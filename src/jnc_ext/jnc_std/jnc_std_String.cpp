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
#include "jnc_std_String.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_CLASS_TYPE(
	StringBuilder,
	"std.StringBuilder",
	g_stdLibGuid,
	StdLibCacheSlot_StringBuilder
)

JNC_BEGIN_TYPE_FUNCTION_MAP(StringBuilder)
	JNC_MAP_FUNCTION("clear", &StringBuilder::clear)
	JNC_MAP_FUNCTION("reserve", &StringBuilder::reserve)
	JNC_MAP_FUNCTION("copy", &StringBuilder::copy_string)
	JNC_MAP_OVERLOAD(&StringBuilder::copy_char)
	JNC_MAP_FUNCTION("insert", &StringBuilder::insert_string)
	JNC_MAP_OVERLOAD(&StringBuilder::insert_char)
	JNC_MAP_FUNCTION("remove", &StringBuilder::remove)
	JNC_MAP_FUNCTION("chop", &StringBuilder::chop)
	JNC_MAP_FUNCTION("trimLeft", &StringBuilder::trimLeft)
	JNC_MAP_FUNCTION("trimRight", &StringBuilder::trimRight)
	JNC_MAP_FUNCTION("detachString", &StringBuilder::detachString)
	JNC_MAP_FUNCTION("cloneString", &StringBuilder::cloneString)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
StringBuilder::clear() {
	if (m_length)
		*(char*)m_ptr.m_p = 0;

	m_length = 0;
}

bool
JNC_CDECL
StringBuilder::reserve(size_t length) {
	if (length < m_bufferSize)
		return true;

	size_t bufferSize = sl::getAllocSize(length + 1); // reserve space for null
	ASSERT(bufferSize > length);

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	DataPtr ptr = gcHeap->tryAllocateBuffer(bufferSize);
	if (!ptr.m_p)
		return false;

	memcpy(ptr.m_p, m_ptr.m_p, m_length);
	m_ptr = ptr;
	m_bufferSize = bufferSize;
	return true;
}

size_t
JNC_CDECL
StringBuilder::copy_char(
	utf32_t c,
	size_t count
) {
	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));
	string.copy(c, count);
	return copyImpl(string.cp(), string.getLength());
}

size_t
JNC_CDECL
StringBuilder::copy_string(String string) {
	return copyImpl((char*)string.m_ptr.m_p, string.m_length);
}

size_t
JNC_CDECL
StringBuilder::insert_char(
	size_t offset,
	utf32_t c,
	size_t count
) {
	char buffer[256];
	sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));
	string.copy(c, count);
	return insertImpl(offset, string.cp(), string.getLength());
}

size_t
JNC_CDECL
StringBuilder::insert_string(
	size_t offset,
	String string
) {
	return insertImpl(offset, (char*)string.m_ptr.m_p, string.m_length);
}

size_t
JNC_CDECL
StringBuilder::remove(
	size_t offset,
	size_t length
) {
	if (offset >= m_length)
		return m_length;

	size_t maxRemoveSize = m_length - offset;
	if (length > maxRemoveSize)
		length = maxRemoveSize;

	if (!length)
		return m_length;

	size_t newLength = m_length - length;
	size_t tailIdx = offset + length;
	char* p = (char*)m_ptr.m_p;
	memmove(p + offset, p + tailIdx, m_length - tailIdx);
	p[newLength] = 0;
	m_length = newLength;
	return newLength;
}

size_t
JNC_CDECL
StringBuilder::chop(size_t length) {
	if (length >= m_length) {
		clear();
		return 0;
	}

	m_length -= length;
	((char*)m_ptr.m_p)[m_length] = 0;
	return m_length;
}

size_t
JNC_CDECL
StringBuilder::trimLeft() {
	sl::StringRef string((char*)m_ptr.m_p, m_length);
	size_t i = string.findNotOneOf(sl::StringRef::Details::getWhitespace());
	if (i == -1) {
		clear();
		return 0;
	}

	return remove(0, i);
}

size_t
JNC_CDECL
StringBuilder::trimRight() {
	sl::StringRef string((char*)m_ptr.m_p, m_length);
	size_t i = string.reverseFindNotOneOf(sl::StringRef::Details::getWhitespace());

	if (i == -1) {
		clear();
		return 0;
	}

	m_length = i + 1;
	((char*)m_ptr.m_p)[m_length] = 0;
	return m_length;
}

String
StringBuilder::detachString(StringBuilder* self) {
	if (!self->m_bufferSize)
		return g_nullString;

	String string;
	string.m_ptr = self->m_ptr;
	string.m_ptr_sz = self->m_ptr;
	string.m_length = self->m_length;
	self->m_ptr = g_nullDataPtr;
	self->m_length = 0;
	self->m_bufferSize = 0;
	return string;
}

String
StringBuilder::cloneString(StringBuilder* self) {
	if (!self->m_bufferSize)
		return g_nullString;

	return allocateString((char*)self->m_ptr.m_p, self->m_length);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
StringBuilder::copyImpl(
	const char* p,
	size_t length
) {
	ASSERT(length != -1);

	bool result = reserve(length);
	if (!result)
		return -1;

	memcpy(m_ptr.m_p, p, length);
	((char*)m_ptr.m_p)[length] = 0;
	m_length = length;
	return length;
}

size_t
StringBuilder::insertImpl(
	size_t offset,
	const char* src,
	size_t length
) {
	ASSERT(length != -1);

	size_t newLength = m_length + length;
	bool result = reserve(newLength);
	if (!result)
		return -1;

	if (offset > m_length)
		offset = m_length;

	char* dst = (char*)m_ptr.m_p + offset;

	if (offset < m_length)
		memmove(dst + length, dst, m_length - offset);

	memcpy(dst, src, length);
	((char*)m_ptr.m_p)[newLength] = 0;
	m_length = newLength;
	return newLength;
}

//..............................................................................

} // namespace std
} // namespace jnc
