#include "pch.h"
#include "jnc_std_Buffer.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//.............................................................................

JNC_DEFINE_TYPE (
	ConstBufferRef, 
	"std.ConstBufferRef", 
	g_stdLibGuid, 
	StdLibCacheSlot_ConstBufferRef
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (ConstBufferRef)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	ConstBuffer, 
	"std.ConstBuffer", 
	g_stdLibGuid, 
	StdLibCacheSlot_ConstBuffer
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (ConstBuffer)
	JNC_MAP_FUNCTION ("copy", &ConstBuffer::copy_s1)
	JNC_MAP_OVERLOAD (&ConstBuffer::copy_s2)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	BufferRef, 
	"std.BufferRef", 
	g_stdLibGuid, 
	StdLibCacheSlot_BufferRef
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (BufferRef)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	Buffer, 
	"std.Buffer", 
	g_stdLibGuid, 
	StdLibCacheSlot_Buffer
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Buffer)
	JNC_MAP_FUNCTION ("copy", &Buffer::copy)
	JNC_MAP_FUNCTION ("append", &Buffer::append)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

bool
ConstBuffer::copy (ConstBufferRef ref)
{
	if (!ref.m_isFinal)
		return copy (ref.m_ptr, ref.m_size);

	m_ptr = ref.m_ptr;
	m_size = ref.m_size;
	return true;
}

bool
ConstBuffer::copy (
	DataPtr ptr,
	size_t size
	)
{
	if (!size)
	{
		m_ptr = g_nullPtr;
		m_size = 0;
		return true;
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	m_ptr = gcHeap->tryAllocateBuffer (size + 1);
	if (!m_ptr.m_p)
		return false;

	memcpy (m_ptr.m_p, ptr.m_p, size);
	return true;
}

//.............................................................................

bool
AXL_CDECL
Buffer::copy (
	DataPtr ptr,
	size_t size
	)
{
	bool result = setSize (size, false);
	if (!result)
		return false;

	memcpy (m_ptr.m_p, ptr.m_p, size);
	return true;
}

bool
AXL_CDECL
Buffer::append (
	DataPtr ptr,
	size_t size
	)
{
	size_t prevSize = m_size;

	bool result = setSize (prevSize + size, true);
	if (!result)
		return false;

	memcpy ((char*) m_ptr.m_p + prevSize, ptr.m_p, size);
	return true;
}

bool
Buffer::setSize (
	size_t size,
	bool saveContents
	)
{
	if (size <= m_maxSize)
	{
		((char*) m_ptr.m_p) [size] = 0;
		m_size = size;
		return true;
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	size_t maxSize = sl::getMinPower2Gt (size);
	DataPtr newPtr = gcHeap->tryAllocateBuffer (maxSize + 1);
	if (!newPtr.m_p)
		return false;

	if (saveContents)
		memcpy (newPtr.m_p, m_ptr.m_p, m_size);

	m_ptr = newPtr;
	m_size = size;
	m_maxSize = maxSize;
	return true;
}

//.............................................................................

} // namespace std
} // namespace jnc
