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

#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace std {

JNC_DECLARE_TYPE (Guid)

//..............................................................................

struct Guid: sl::Guid
{
	JNC_DECLARE_TYPE_STATIC_METHODS (Guid)
};

//..............................................................................

} // namespace std
} // namespace jnc
