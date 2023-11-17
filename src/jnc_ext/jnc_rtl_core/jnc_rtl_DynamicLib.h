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
namespace rtl {

JNC_DECLARE_CLASS_TYPE(DynamicLib)

//..............................................................................

class DynamicLib: public IfaceHdr {
public:
	handle_t m_handle;

public:
	bool
	JNC_CDECL
	open(String fileName) {
		return openImpl(fileName >> toAxl);
	}

	bool
	openImpl(const sl::StringRef& fileName);

	void
	JNC_CDECL
	close() {
		getDynamicLib()->close();
	}

	void*
	JNC_CDECL
	getFunction(String name) {
		return getFunctionImpl(name >> toAxl);
	}

	void*
	getFunctionImpl(const sl::StringRef& name);

protected:
	sys::DynamicLib*
	getDynamicLib() {
		ASSERT(sizeof(sys::DynamicLib) == sizeof(m_handle));
		return (sys::DynamicLib*) &m_handle;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
