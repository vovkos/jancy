#include "pch.h"
#include "jnc_std_Error.h"

namespace jnc {
namespace std {

//.............................................................................

rt::DataPtr
Error::getDescription ()
{
	sl::String string = err::ErrorHdr::getDescription ();
	size_t length = string.getLength ();
	return rt::strDup (string, string.getLength ());
}

//.............................................................................

} // namespace std
} // namespace jnc
