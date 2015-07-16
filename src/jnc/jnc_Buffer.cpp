#include "pch.h"
#include "jnc_Buffer.h"

namespace jnc {

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
		m_ptr.m_p = (void*) "";
		m_size = 0;
		return true;
	}

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DynamicArrayBox* box = runtime->m_gcHeap.tryAllocateBuffer (size + 1);
	if (!box)
		return false;

	char* p = (char*) (box + 1);
	memcpy (p, ptr.m_p, size);

	m_ptr.m_p = p;
	m_ptr.m_validator = &box->m_validator;
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

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t maxSize = rtl::getMinPower2Gt (size);
	DynamicArrayBox* box = runtime->m_gcHeap.tryAllocateBuffer (maxSize + 1);
	if (!box)
		return false;

	char* p = (char*) (box + 1);

	if (saveContents)
		memcpy (p, m_ptr.m_p, m_size);

	m_ptr.m_p = p;
	m_ptr.m_validator = &box->m_validator;
	m_size = size;
	m_maxSize = maxSize;
	return true;
}

//.............................................................................

} // namespace jnc
