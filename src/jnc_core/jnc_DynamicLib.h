#pragma once

#include "jnc_Module.h"

namespace jnc {

//.............................................................................

class DynamicLib: public IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("jnc.DynamicLib", -1, -1)
		JNC_MAP_FUNCTION ("release", &DynamicLib::release)
		JNC_MAP_FUNCTION ("load", &DynamicLib::load)
		JNC_MAP_FUNCTION ("getFunction", &DynamicLib::getFunction)
	JNC_END_CLASS_TYPE_MAP ()

public:
	handle_t m_handle;

public:
	bool 
	AXL_CDECL
	load (DataPtr fileNamePtr)
	{
		return loadImpl ((const char*) fileNamePtr.m_p);
	}

	bool
	loadImpl (const char* fileName);

	void
	AXL_CDECL
	release ()
	{
		getDynamicLibrary ()->release ();
	}

	void* 
	AXL_CDECL
	getFunction (DataPtr namePtr)
	{
		return getFunctionImpl ((const char*) namePtr.m_p);
	}

	void* 
	getFunctionImpl (const char* name);

	mt::DynamicLibrary*
	getDynamicLibrary ()
	{
		ASSERT (sizeof (mt::DynamicLibrary) == sizeof (m_handle));
		return (mt::DynamicLibrary*) &m_handle;
	}
};

//.............................................................................

} // namespace jnc
