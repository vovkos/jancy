#include "pch.h"
#include "jnc_Buffer.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

bool 
ConstBuffer::copy (
	DataPtr ptr,
	size_t size
	)
{
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
	return true;
}

bool 
AXL_CDECL
Buffer::append (
	DataPtr ptr,
	size_t size
	)
{
	return true;
}

//.............................................................................

} // namespace jnc
