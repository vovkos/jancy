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
#include "jnc_Runtime.h"

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
	JNC_MAP_FUNCTION("parse",     &Guid::parse)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

String
JNC_CDECL
Guid::getString(
	DataPtr selfPtr,
	uint_t flags
) {
	Guid* self = (Guid*)selfPtr.m_p;
	sl::String string = self->sl::Guid::getString(flags);
	return allocateString(string);
}

bool
JNC_CDECL
Guid::parse(
	DataPtr selfPtr,
	String string
) {
	Guid* self = (Guid*)selfPtr.m_p;
	return self->sl::Guid::parse(string >> toAxl);
}

//..............................................................................

} // namespace std
} // namespace jnc
