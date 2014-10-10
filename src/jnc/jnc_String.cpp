#include "pch.h"
#include "jnc_String.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

String 
String::getZeroTerminatedString ()
{
	String resultString = *this;
	resultString.ensureZeroTerminated ();
	return resultString;
}

bool 
String::copy (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = StdLib::strLen (ptr);

	if (!length)
	{
		m_ptr.m_p = "";
		m_length = 0;
		return true;
	}

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	if (!p)
		return false;

	memcpy (p, ptr.m_p, length);
	p [length] = 0;

	m_ptr.m_p = p;
	m_ptr.m_rangeBegin = p;
	m_ptr.m_rangeEnd = p + length + 1;
	m_ptr.m_object = jnc::getStaticObjHdr ();
	m_length = length;
	return true;
}

//.............................................................................

} // namespace jnc
