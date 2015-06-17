#include "pch.h"
#include "jnc_Error.h"

namespace jnc {

//.............................................................................

DataPtr
Error::getDescription ()
{
	rtl::String string = err::ErrorData::getDescription ();
	size_t length = string.getLength ();

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	memcpy (p, string.cc (), length);
	p [length] = 0;

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + length + 1;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

//.............................................................................

} // namespace jnc
