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

JNC_DEFINE_CLASS_TYPE (
	StringBuilder,
	"std.StringBuilder",
	g_stdLibGuid,
	StdLibCacheSlot_StringBuilder
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (StringBuilder)
	JNC_MAP_FUNCTION ("clear", &StringBuilder::clear)
	JNC_MAP_FUNCTION ("reserve", &StringBuilder::reserve)
	JNC_MAP_FUNCTION ("copy", &StringBuilder::copy_utf8)
	JNC_MAP_OVERLOAD (&StringBuilder::copy_utf16)
	JNC_MAP_OVERLOAD (&StringBuilder::copy_utf32)
	JNC_MAP_FUNCTION ("insert", &StringBuilder::insert_utf8)
	JNC_MAP_OVERLOAD (&StringBuilder::insert_utf16)
	JNC_MAP_OVERLOAD (&StringBuilder::insert_utf32)
	JNC_MAP_FUNCTION ("remove", &StringBuilder::remove)
	JNC_MAP_FUNCTION ("detachString", &StringBuilder::detachString)
	JNC_MAP_FUNCTION ("cloneString", &StringBuilder::cloneString)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

bool
JNC_CDECL
StringBuilder::reserve (size_t length)
{
	if (length <= m_maxLength)
		return true;

	size_t size = sl::getHiBit (length + 1); // reserve space for null
	ASSERT (size > length);

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	DataPtr ptr = gcHeap->tryAllocateBuffer (size);
	if (!ptr.m_p)
		return false;

	memcpy (ptr.m_p, m_ptr.m_p, m_length);
	m_ptr = ptr;
	m_maxLength = size - 1;
	return true;
}

size_t
JNC_CDECL
StringBuilder::copy_utf8 (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return copyImpl ((char*) ptr.m_p, length);
}

size_t
JNC_CDECL
StringBuilder::copy_utf16 (
	DataPtr ptr,
	size_t length
	)
{
	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.copy ((utf16_t*) ptr.m_p, length);
	return copyImpl (string, string.getLength ());
}

size_t
JNC_CDECL
StringBuilder::copy_utf32 (
	DataPtr ptr,
	size_t length
	)
{
	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.copy ((utf32_t*) ptr.m_p, length);
	return copyImpl (string, string.getLength ());
}

size_t
JNC_CDECL
StringBuilder::insert_utf8 (
	size_t offset,
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return insertImpl (offset, (char*) ptr.m_p, length);
}

size_t
JNC_CDECL
StringBuilder::insert_utf16 (
	size_t offset,
	DataPtr ptr,
	size_t length
	)
{
	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.copy ((utf16_t*) ptr.m_p, length);
	return insertImpl (offset, string, string.getLength ());
}

size_t
JNC_CDECL
StringBuilder::insert_utf32 (
	size_t offset,
	DataPtr ptr,
	size_t length
	)
{
	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.copy ((utf32_t*) ptr.m_p, length);
	return insertImpl (offset, string, string.getLength ());
}

size_t
JNC_CDECL
StringBuilder::remove (
	size_t offset,
	size_t length
	)
{
	if (offset > m_length)
		offset = m_length;

	size_t maxRemoveSize = m_length - offset;
	if (length > maxRemoveSize)
		length = maxRemoveSize;

	if (!length)
		return m_length;

	size_t newLength = m_length - length;
	size_t tailIdx = offset + length;
	char* p = (char*) m_ptr.m_p;
	memmove (p + offset, p + tailIdx, m_length - tailIdx);
	m_length = newLength;
	return newLength;
}

DataPtr
StringBuilder::detachString (StringBuilder* self)
{
	if (!self->m_maxLength)
	{
		GcHeap* gcHeap = getCurrentThreadGcHeap ();
		return gcHeap->tryAllocateBuffer (1); // empty string + null
	}

	((char*) self->m_ptr.m_p) [self->m_length] = 0;

	DataPtr ptr = self->m_ptr;
	self->m_ptr = g_nullPtr;
	self->m_length = 0;
	self->m_maxLength = 0;

	return ptr;
}

DataPtr
StringBuilder::cloneString (StringBuilder* self)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	if (!self->m_maxLength)
		return gcHeap->tryAllocateBuffer (1); // empty string + null

	DataPtr ptr = gcHeap->tryAllocateBuffer (self->m_length + 1);
	if (!ptr.m_p)
		return g_nullPtr;

	memcpy (ptr.m_p, self->m_ptr.m_p, self->m_length);
	return ptr;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
StringBuilder::copyImpl (
	const char* p,
	size_t length
	)
{
	ASSERT (length != -1);

	bool result = reserve (length);
	if (!result)
		return -1;

	memcpy (m_ptr.m_p, p, length);
	m_length = length;
	return length;
}

size_t
StringBuilder::insertImpl (
	size_t offset,
	const char* src,
	size_t length
	)
{
	ASSERT (length != -1);

	size_t newLength = m_length + length;
	bool result = reserve (newLength);
	if (!result)
		return -1;

	if (offset > m_length)
		offset = m_length;

	char* dst = (char*) m_ptr.m_p;

	if (offset < m_length)
		memmove (dst + offset + length, dst + offset, m_length - offset);

	memcpy (dst + offset, src, length);
	m_length = newLength;
	return newLength;
}

//..............................................................................

} // namespace std
} // namespace jnc
