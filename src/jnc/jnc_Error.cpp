#include "pch.h"
#include "jnc_Error.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

DataPtr
Error::getDescription ()
{
	rtl::String string = err::ErrorData::getDescription ();
	size_t length = string.getLength ();
	return strDup (string, string.getLength ());
}

//.............................................................................

} // namespace jnc
