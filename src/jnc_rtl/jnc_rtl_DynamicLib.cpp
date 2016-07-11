#include "pch.h"
#include "jnc_rtl_DynamicLib.h"

namespace jnc {
namespace rtl {

//.............................................................................

bool 
DynamicLib::openImpl (const char* fileName)
{
	bool result = getDynamicLibrary ()->open (fileName);
	if (!result)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		err::pushFormatStringError ("cannot open dynamiclib '%s'", fileName);
#endif
		return false;
	}

	return true;
}

void* 
DynamicLib::getFunctionImpl (const char* name)
{
	ASSERT (sizeof (sys::DynamicLibrary) == sizeof (m_handle));

	if (!m_handle)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return NULL;
	}

	void* p = getDynamicLibrary ()->getFunction (name);
	if (!p)
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		err::pushFormatStringError ("cannot get dynamiclib function '%s'", name);
#endif
		return NULL;
	}

	return p;
}

//.............................................................................

} // namespace rtl
} // namespace jnc
