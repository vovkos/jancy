#pragma once

#include "jnc_ClassType.h"
#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {

//.............................................................................

class Library: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE ("jnc.Library", StdApiSlot_Library)
		JNC_FUNCTION ("release", &Library::release)
		JNC_FUNCTION ("load", &Library::load)
		JNC_FUNCTION ("getFunction", &Library::getFunction)
	JNC_END_TYPE ()

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
