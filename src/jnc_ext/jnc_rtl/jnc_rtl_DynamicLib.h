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

class DynamicLib: public IfaceHdr
{
public:
	handle_t m_handle;

public:
	bool
	JNC_CDECL
	open(DataPtr fileNamePtr)
	{
		return openImpl((const char*) fileNamePtr.m_p);
	}

	bool
	openImpl(const sl::StringRef& fileName);

	void
	JNC_CDECL
	close()
	{
		getDynamicLibrary()->close();
	}

	void*
	JNC_CDECL
	getFunction(DataPtr namePtr)
	{
		return getFunctionImpl((const char*) namePtr.m_p);
	}

	void*
	getFunctionImpl(const sl::StringRef& name);

	sys::DynamicLibrary*
	getDynamicLibrary()
	{
		ASSERT(sizeof(sys::DynamicLibrary) == sizeof(m_handle));
		return (sys::DynamicLibrary*) &m_handle;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
