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
#include "jnc_std_Array.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_CLASS_TYPE (
	Array,
	"std.Array",
	g_stdLibGuid,
	StdLibCacheSlot_Array
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Array)
	JNC_MAP_FUNCTION ("clear", &Array::clear)
	JNC_MAP_FUNCTION ("setCount", &Array::setCount)
	JNC_MAP_FUNCTION ("reserve", &Array::reserve)
	JNC_MAP_FUNCTION ("copy", &Array::copy)
	JNC_MAP_FUNCTION ("insert", &Array::insert)
	JNC_MAP_FUNCTION ("remove", &Array::remove)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
JNC_CDECL
Array::clear ()
{
	Variant* p = (Variant*) m_ptr.m_p;
	memset (p, 0, m_count * sizeof (Variant));
}

bool
JNC_CDECL
Array::setCount (size_t count)
{
	if (count == m_count)
		return true;

	if (count < m_count)
	{
		Variant* p = (Variant*) m_ptr.m_p;
		memset (p + count, 0, (m_count - count) * sizeof (Variant));
	}
	else if (count > m_maxCount)
	{
		bool result = reserve (count);
		if (!result)
			return false;
	}

	m_count = count;
	return true;
}

bool
JNC_CDECL
Array::reserve (size_t count)
{
	if (count <= m_maxCount)
		return true;

	Type* type = m_box->m_type->getModule ()->getPrimitiveType (TypeKind_Variant);
	size_t maxCount = sl::getMinPower2Ge (count);

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	DataPtr ptr = gcHeap->tryAllocateArray (type, maxCount);
	if (!ptr.m_p)
		return false;

	memcpy (ptr.m_p, m_ptr.m_p, m_count * sizeof (Variant));
	m_ptr = ptr;
	m_maxCount = maxCount;
	return true;
}

size_t
JNC_CDECL
Array::copy (
	DataPtr ptr,
	size_t count
	)
{
	bool result = reserve (count);
	if (!result)
		return -1;

	memcpy (m_ptr.m_p, ptr.m_p, count * sizeof (Variant));
	m_count = count;
	return count;
}

size_t
JNC_CDECL
Array::insert (
	size_t index,
	DataPtr ptr,
	size_t count
	)
{
	size_t newCount = m_count + count;
	bool result = reserve (newCount);
	if (!result)
		return -1;

	if (index > m_count)
		index = m_count;

	Variant* p = (Variant*) m_ptr.m_p;
	memmove (p + index + count, p + index, count * sizeof (Variant));
	memcpy (p, ptr.m_p, count * sizeof (Variant));
	m_count = newCount;
	return newCount;
}

size_t
JNC_CDECL
Array::remove (
	size_t index,
	size_t count
	)
{
	if (index > m_count)
		index = m_count;

	size_t maxRemoveCount = m_count - index;
	if (count > maxRemoveCount)
		count = maxRemoveCount;

	if (!count)
		return m_count;

	size_t newCount = m_count - count;
	size_t tailIdx = index + count;
	Variant* p = (Variant*) m_ptr.m_p;
	memmove (p + index, p + tailIdx, (m_count - tailIdx) * sizeof (Variant));
	memset (p + newCount, 0, count * sizeof (Variant));
	m_count = newCount;
	return newCount;
}

//..............................................................................

} // namespace std
} // namespace jnc
