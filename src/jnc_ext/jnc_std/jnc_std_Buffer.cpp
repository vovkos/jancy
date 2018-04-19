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
#include "jnc_std_Buffer.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_CLASS_TYPE (
	Buffer,
	"std.Buffer",
	g_stdLibGuid,
	StdLibCacheSlot_Buffer
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Buffer)
	JNC_MAP_FUNCTION ("clear", &Buffer::clear)
	JNC_MAP_FUNCTION ("setSize", &Buffer::setSize)
	JNC_MAP_FUNCTION ("reserve", &Buffer::reserve)
	JNC_MAP_FUNCTION ("copy", &Buffer::copy)
	JNC_MAP_FUNCTION ("insert", &Buffer::insert)
	JNC_MAP_FUNCTION ("remove", &Buffer::remove)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

bool
JNC_CDECL
Buffer::setSize (size_t size)
{
	if (size == m_size)
		return true;

	if (size > m_maxSize)
	{
		bool result = reserve (size);
		if (!result)
			return false;
	}
	else if (size > m_size)
	{
		memset ((char*) m_ptr.m_p + m_size, 0, size - m_size);
	}

	m_size = size;
	return true;
}

bool
JNC_CDECL
Buffer::reserve (size_t size)
{
	if (size <= m_maxSize)
		return true;

	size_t maxSize = sl::getHiBit (size);

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	DataPtr ptr = gcHeap->tryAllocateBuffer (maxSize);
	if (!ptr.m_p)
		return false;

	memcpy (ptr.m_p, m_ptr.m_p, m_size);
	m_ptr = ptr;
	m_maxSize = maxSize;
	return true;
}

size_t
JNC_CDECL
Buffer::copy (
	DataPtr ptr,
	size_t size
	)
{
	bool result = reserve (size);
	if (!result)
		return -1;

	memcpy (m_ptr.m_p, ptr.m_p, size);
	m_size = size;
	return size;
}

size_t
JNC_CDECL
Buffer::insert (
	size_t offset,
	DataPtr ptr,
	size_t size
	)
{
	size_t newSize = m_size + size;
	bool result = reserve (newSize);
	if (!result)
		return -1;

	if (offset > m_size)
		offset = m_size;

	char* p = (char*) m_ptr.m_p;

	if (offset < m_size)
		memmove (p + offset + size, p + offset, m_size - offset);

	memcpy (p + offset, ptr.m_p, size);
	m_size = newSize;
	return newSize;
}

size_t
JNC_CDECL
Buffer::remove (
	size_t offset,
	size_t size
	)
{
	if (offset > m_size)
		offset = m_size;

	size_t maxRemoveSize = m_size - offset;
	if (size > maxRemoveSize)
		size = maxRemoveSize;

	if (!size)
		return m_size;

	size_t newSize = m_size - size;
	size_t tailIdx = offset + size;
	char* p = (char*) m_ptr.m_p;
	memmove (p + offset, p + tailIdx, m_size - tailIdx);
	m_size = newSize;
	return newSize;
}

//..............................................................................

} // namespace std
} // namespace jnc
