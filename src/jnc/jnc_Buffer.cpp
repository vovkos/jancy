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

	char* p = (char*) AXL_MEM_ALLOC (size + 1);
	if (!p)
		return false;

	memcpy (p, ptr.m_p, size);
	p [size] = 0;

	m_ptr.m_p = p;
	m_ptr.m_rangeBegin = p;
	m_ptr.m_rangeEnd = p + size + 1;
	m_ptr.m_object = jnc::getStaticObjHdr ();
	m_size = size;
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

	size_t maxSize = rtl::getMinPower2Gt (size);

	char* p = (char*) AXL_MEM_ALLOC (maxSize + 1);
	if (!p)
		return false;

	p [size] = 0;

	if (saveContents)
		memcpy (p, m_ptr.m_p, m_size);

	m_ptr.m_p = p;
	m_ptr.m_rangeBegin = p;
	m_ptr.m_rangeEnd = p + maxSize + 1;
	m_ptr.m_object = jnc::getStaticObjHdr ();

	m_size = size;
	m_maxSize = maxSize;
	return true;
}

//.............................................................................

} // namespace jnc
