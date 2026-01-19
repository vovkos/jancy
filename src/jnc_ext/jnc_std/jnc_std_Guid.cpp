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
#include "jnc_std_Guid.h"
#include "jnc_std_StdLib.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_TYPE(
	Guid,
	"std.Guid",
	g_stdLibGuid,
	StdLibCacheSlot_Guid
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Guid)
	JNC_MAP_FUNCTION("getString", &Guid::getString)
	JNC_MAP_FUNCTION("parse", &Guid::parse)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace std
} // namespace jnc
