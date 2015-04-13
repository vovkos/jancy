#include "pch.h"
#include "jnc_Library.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

bool 
Library::loadImpl (const char* fileName)
{
	m_handle = ::LoadLibraryA (fileName);
	if (!m_handle)
	{
		err::setLastSystemError ();
		err::pushFormatStringError ("cannot load library '%s'", fileName);
		return false;
	}

	return true;
}

void
AXL_CDECL
Library::release ()
{
	if (!m_handle)
		return;

	::FreeLibrary ((HMODULE) m_handle);
}

void* 
Library::getFunctionImpl (const char* name)
{
	if (!m_handle)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	void* p = ::GetProcAddress ((HMODULE) m_handle, name);
	if (!p)
	{
		err::setLastSystemError ();
		err::pushFormatStringError ("cannot get library function '%s'", name);
		return NULL;
	}

	return p;
}

//.............................................................................

} // namespace jnc
