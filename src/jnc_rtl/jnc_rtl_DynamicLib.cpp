#include "pch.h"
#include "jnc_rtl_DynamicLib.h"

namespace jnc {
namespace rtl {

//.............................................................................

JNC_DEFINE_CLASS_TYPE (
	DynamicLib, 
	"jnc.DynamicLib", 
	sl::g_nullGuid, 
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (DynamicLib)
	JNC_MAP_FUNCTION ("open", &DynamicLib::open)
	JNC_MAP_FUNCTION ("close", &DynamicLib::close)
	JNC_MAP_FUNCTION ("getFunction", &DynamicLib::getFunction)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

bool 
DynamicLib::openImpl (const char* fileName)
{
	bool result = getDynamicLibrary ()->open (fileName);
	if (!result)
	{
#if (_JNC_OS_WIN)
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
#if (_JNC_OS_WIN)
		err::pushFormatStringError ("cannot get dynamiclib function '%s'", name);
#endif
		return NULL;
	}

	return p;
}

//.............................................................................

} // namespace rtl
} // namespace jnc
