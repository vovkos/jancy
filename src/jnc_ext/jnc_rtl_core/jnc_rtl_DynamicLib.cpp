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

#include "pch.h"
#include "jnc_rtl_DynamicLib.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_CLASS_TYPE(
	DynamicLib,
	"jnc.DynamicLib",
	sl::g_nullGuid,
	-1
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLib)
	JNC_MAP_FUNCTION("open", &DynamicLib::open)
	JNC_MAP_FUNCTION("close", &DynamicLib::close)
	JNC_MAP_FUNCTION("getFunction", &DynamicLib::getFunction)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

bool
DynamicLib::openImpl(const sl::StringRef& fileName) {
	bool result = getDynamicLib()->open(fileName);
	if (!result) {
#if (_JNC_OS_WIN)
		err::pushFormatStringError("cannot open dynamiclib '%s'", fileName.sz());
#endif
		return false;
	}

	return true;
}

void*
DynamicLib::getFunctionImpl(const sl::StringRef& name) {
	ASSERT(sizeof(sys::DynamicLib) == sizeof(m_handle));

	if (!m_handle) {
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return NULL;
	}

	void* p = getDynamicLib()->getFunction(name);
	if (!p) {
#if (_JNC_OS_WIN)
		err::pushFormatStringError("cannot get dynamiclib function '%s'", name.sz());
#endif
		return NULL;
	}

	return p;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
