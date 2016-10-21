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
#include "jnc_std_Error.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_TYPE (
	Error,
	"std.Error",
	g_stdLibGuid,
	StdLibCacheSlot_Error
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Error)
	JNC_MAP_CONST_PROPERTY ("m_description", Error::getDescription_s)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

DataPtr
Error::getDescription ()
{
	sl::String string = err::ErrorHdr::getDescription ();
	return strDup (string, string.getLength ());
}

//..............................................................................

} // namespace std
} // namespace jnc
