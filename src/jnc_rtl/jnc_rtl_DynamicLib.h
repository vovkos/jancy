#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_CLASS_TYPE (DynamicLib)

//.............................................................................

class DynamicLib: public IfaceHdr
{
public:
	handle_t m_handle;

public:
	bool 
	AXL_CDECL
	open (DataPtr fileNamePtr)
	{
		return openImpl ((const char*) fileNamePtr.m_p);
	}

	bool
	openImpl (const char* fileName);

	void
	AXL_CDECL
	close ()
	{
		getDynamicLibrary ()->close ();
	}

	void* 
	AXL_CDECL
	getFunction (DataPtr namePtr)
	{
		return getFunctionImpl ((const char*) namePtr.m_p);
	}

	void* 
	getFunctionImpl (const char* name);

	sys::DynamicLibrary*
	getDynamicLibrary ()
	{
		ASSERT (sizeof (sys::DynamicLibrary) == sizeof (m_handle));
		return (sys::DynamicLibrary*) &m_handle;
	}
};

//.............................................................................

} // namespace rtl
} // namespace jnc
