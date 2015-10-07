#include "pch.h"
#include "jnc_std_String.h"

namespace jnc {
namespace std {

//.............................................................................

String
String::getZeroTerminatedString ()
{
	String resultString = *this;
	resultString.ensureZeroTerminated ();
	return resultString;
}

bool
String::copy (StringRef ref)
{
	if (!ref.m_isFinal)
		return copy (ref.m_ptr, ref.m_length);

	m_ptr = ref.m_ptr;
	m_length = ref.m_length;
	return true;
}

bool
String::copy (
	rt::DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	if (!length)
	{
		m_ptr.m_p = (void*) "";
		m_length = 0;
		return true;
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::DataPtr newPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!newPtr.m_p)
		return false;
	
	memcpy (newPtr.m_p, ptr.m_p, length);

	m_ptr = newPtr;
	m_length = length;
	return true;
}

//.............................................................................

bool
AXL_CDECL
StringBuilder::copy (
	rt::DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	bool result = setLength (length, false);
	if (!result)
		return false;

	memcpy (m_ptr.m_p, ptr.m_p, length);
	return true;
}

bool
AXL_CDECL
StringBuilder::append (
	rt::DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	size_t prevLength = m_length;

	bool result = setLength (prevLength + length, true);
	if (!result)
		return false;

	memcpy ((char*) m_ptr.m_p + prevLength, ptr.m_p, length);
	return true;
}

bool
StringBuilder::setLength (
	size_t length,
	bool saveContents
	)
{
	if (length <= m_maxLength)
	{
		((char*) m_ptr.m_p) [length] = 0;
		m_length = length;
		return true;
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t maxLength = sl::getMinPower2Gt (length);
	rt::DataPtr newPtr = runtime->m_gcHeap.tryAllocateBuffer (maxLength + 1);
	if (!newPtr.m_p)
		return false;

	if (saveContents)
		memcpy (newPtr.m_p, m_ptr.m_p, m_length);

	m_ptr = newPtr;
	m_length = length;
	m_maxLength = maxLength;
	return true;
}

//.............................................................................

} // namespace std
} // namespace jnc
