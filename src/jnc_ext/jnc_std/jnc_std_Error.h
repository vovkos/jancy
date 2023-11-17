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
#include "jnc_Runtime.h"

namespace jnc {
namespace std {

JNC_DECLARE_TYPE(Error)

//..............................................................................

struct Error: err::ErrorHdr {
	JNC_DECLARE_TYPE_STATIC_METHODS(Error)

	String
	getDescription() {
		return allocateString(err::ErrorHdr::getDescription());
	}

	static
	String
	getDescription_s(DataPtr selfPtr) {
		return ((Error*)selfPtr.m_p)->getDescription();
	}
};

//..............................................................................

} // namespace std
} // namespace jnc
