#include "pch.h"
#include "jnc_sys_SysLib.h"

namespace jnc {
namespace ext {

//.............................................................................

void
initSysLib (ExtensionLibHost* host)
{
	sys::g_sysLibCacheSlot = host->getLibCacheSlot (sys::g_sysLibGuid);
}

ExtensionLib*
getSysLib (ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	sl::callOnce (initSysLib, host, &onceFlag);
	return sl::getSimpleSingleton <sys::SysLib> ();
}

//.............................................................................

} // namespace ext

namespace sys {

//.............................................................................

void
SysLib::sleep (uint32_t msCount)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.enterWaitRegion ();
	axl::sys::sleep (msCount);
	runtime->m_gcHeap.leaveWaitRegion ();
}

//.............................................................................

} // namespace sys
} // namespace jnc
