#include "pch.h"
#include "jnc_Library.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

bool 
Library::loadImpl (const char* fileName)
{
	bool result = getDynamicLibrary ()->load (fileName);
	if (!result)
	{
		err::pushFormatStringError ("cannot load library '%s'", fileName);
		return false;
	}

	return true;
}

void* 
Library::getFunctionImpl (const char* name)
{
	ASSERT (sizeof (mt::DynamicLibrary) == sizeof (m_handle));

	if (!m_handle)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return NULL;
	}

	void* p = getDynamicLibrary ()->getFunction (name);
	if (!p)
	{
		err::pushFormatStringError ("cannot get library function '%s'", name);
		return NULL;
	}

	return p;
}

//.............................................................................

} // namespace jnc
