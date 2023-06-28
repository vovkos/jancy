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
#include "jnc_sys_SysLib.h"
#include "jnc_sys_Lock.h"
#include "jnc_sys_Event.h"
#include "jnc_sys_Thread.h"
#include "jnc_sys_Timer.h"

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// jancy sources

;static char g_sys_globalsSrc[] =
#include "sys_globals.jnc.cpp"
;static char g_sys_LockSrc[] =
#include "sys_Lock.jnc.cpp"
;static char g_sys_EventSrc[] =
#include "sys_Event.jnc.cpp"
;static char g_sys_NotificationEventSrc[] =
#include "sys_NotificationEvent.jnc.cpp"
;static char g_sys_ThreadSrc[] =
#include "sys_Thread.jnc.cpp"
;static char g_sys_TimerSrc[] =
#include "sys_Timer.jnc.cpp"
;

#if (_JNC_OS_WIN)
#	include "jnc_sys_Registry.h"
;static char g_sys_RegistrySrc[] =
#	include "sys_Registry.jnc.cpp"
;
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

namespace jnc {
namespace sys {

//..............................................................................

uint_t
getCurrentProcessId() {
	return axl::sys::getCurrentProcessId();
}

uintptr_t
getCurrentThreadId() {
	return (uintptr_t)axl::sys::getCurrentThreadId();
}

DataPtr
getProcessImageName(uint_t pid) {
	return strDup(axl::sys::getProcessImageName(pid));
}

DataPtr
getEnv(DataPtr namePtr) {
	if (!namePtr.m_p)
		return g_nullDataPtr;

	const char* value = getenv((const char*) namePtr.m_p);
	return strDup(value);
}

void
setEnv(
	DataPtr namePtr,
	DataPtr valuePtr
) {
#if (_AXL_OS_WIN)
	char buffer[256];
	sl::String envString(rc::BufKind_Stack, buffer, sizeof(buffer));
	envString.format("%s=%s", namePtr.m_p, valuePtr.m_p ? valuePtr.m_p : "");
	_putenv(envString);
#else
	setenv(
		(const char*) namePtr.m_p,
		(const char*) valuePtr.m_p,
		true
	);
#endif
}

void
sleep(uint32_t msCount) {
	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->enterWaitRegion();

#if (_AXL_OS_WIN)
	::SleepEx(msCount, TRUE); // alertable wait (so we can abort with an APC)
#else
	axl::sys::sleep(msCount);
#endif

	gcHeap->leaveWaitRegion();
}

void
initSystemInfo(SystemInfo* systemInfo) {
#if (_JNC_CPU_X86)
	systemInfo->m_cpuKind = CpuKind_Ia32;
#elif (_JNC_CPU_AMD64)
	systemInfo->m_cpuKind = CpuKind_Amd64;
#elif (_JNC_CPU_ARM32)
	systemInfo->m_cpuKind = CpuKind_Arm32;
#elif (_JNC_CPU_ARM64)
	systemInfo->m_cpuKind = CpuKind_Arm64;
#else
#	error unsupported CPU architecture
#endif

#if (_JNC_OS_WIN)
	systemInfo->m_osKind = OsKind_Windows;
	systemInfo->m_osFlags = 0;
#elif (_JNC_OS_LINUX)
	systemInfo->m_osKind = OsKind_Linux;
	systemInfo->m_osFlags = OsFlag_Posix;
#elif (_JNC_OS_SOLARIS)
	systemInfo->m_osKind = OsKind_Solaris;
	systemInfo->m_osFlags = OsFlag_Posix;
#elif (_JNC_OS_DARWIN)
	systemInfo->m_osKind = OsKind_Mac;
	systemInfo->m_osFlags = OsFlag_Posix | OsFlag_Bsd | OsFlag_Darwin;
#elif (_JNC_OS_BSD)
	systemInfo->m_osKind = OsKind_Bsd;
	systemInfo->m_osFlags = OsFlag_Posix | OsFlag_Bsd;
#else
#	error unsupported OS
#endif

#if (_JNC_CPP_MSC)
	systemInfo->m_cppKind = CppKind_Msc;
	systemInfo->m_cppFlags = 0;
#elif (_JNC_CPP_GCC)
#	if (_JNC_CPP_CLANG)
	systemInfo->m_cppKind = CppKind_Clang;
#	elif (_JNC_CPP_ICC)
	systemInfo->m_cppKind = CppKind_Icc;
#	else
	systemInfo->m_cppKind = CppKind_Gcc;
#	endif
	systemInfo->m_cppFlags = CppFlag_Gcc;
#else
#	error unsupported C++ compiler
#endif

	const g::SystemInfo* axlSystemInfo = g::getModule()->getSystemInfo();
	systemInfo->m_processorCount = axlSystemInfo->m_processorCount;
	systemInfo->m_pageSize = axlSystemInfo->m_pageSize;
	systemInfo->m_mappingAlignFactor = axlSystemInfo->m_mappingAlignFactor;
}

inline
SystemInfo*
getSystemInfo() {
	static SystemInfo systemInfo;
	axl::sl::callOnce(initSystemInfo, &systemInfo);
	return &systemInfo;
}

DataPtr
formatTimestamp_0(
	uint64_t timestamp,
	DataPtr format
) {
	axl::sys::Time time(timestamp);
	sl::String string = time.format((const char*) format.m_p);
	return strDup(string);
}

DataPtr
formatTimestamp_1(
	uint64_t timestamp,
	int timeZone,
	DataPtr format
) {
	axl::sys::Time time(timestamp, timeZone);
	sl::String string = time.format((const char*) format.m_p);
	return strDup(string);
}

//..............................................................................

} // namespace sys
} // namespace jnc

using namespace jnc::sys;

JNC_DEFINE_LIB(
	jnc_SysLib,
	g_sysLibGuid,
	"SysLib",
	"Jancy standard extension library"
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(jnc_SysLib)
	JNC_LIB_SOURCE_FILE("sys_globals.jnc",  g_sys_globalsSrc)
	JNC_LIB_SOURCE_FILE("sys_Lock.jnc",     g_sys_LockSrc)
	JNC_LIB_SOURCE_FILE("sys_Event.jnc",    g_sys_EventSrc)
	JNC_LIB_SOURCE_FILE("sys_NotificationEvent.jnc", g_sys_NotificationEventSrc)
	JNC_LIB_SOURCE_FILE("sys_Thread.jnc",   g_sys_ThreadSrc)
	JNC_LIB_SOURCE_FILE("sys_Timer.jnc",    g_sys_TimerSrc)
#if (_JNC_OS_WIN)
	JNC_LIB_SOURCE_FILE("sys_Registry.jnc", g_sys_RegistrySrc)
#endif
	JNC_LIB_IMPORT("sys_globals.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(jnc_SysLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Lock)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Event)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(NotificationEvent)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Thread)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Timer)
#if (_JNC_OS_WIN)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(RegKey)
#endif
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(jnc_SysLib)
	JNC_MAP_FUNCTION_Q("sys.getCurrentProcessId", getCurrentProcessId)
	JNC_MAP_FUNCTION_Q("sys.getCurrentThreadId",  getCurrentThreadId)
	JNC_MAP_FUNCTION_Q("sys.getProcessImageName", getProcessImageName)
	JNC_MAP_FUNCTION_Q("sys.getTimestamp",        axl::sys::getTimestamp)
	JNC_MAP_FUNCTION_Q("sys.getPreciseTimestamp", axl::sys::getPreciseTimestamp)
	JNC_MAP_FUNCTION_Q("sys.formatTimestamp",     formatTimestamp_0)
	JNC_MAP_OVERLOAD(formatTimestamp_1)
	JNC_MAP_FUNCTION_Q("sys.sleep",               jnc::sys::sleep)
	JNC_MAP_PROPERTY_Q("sys.g_env",               getEnv, setEnv)
	JNC_MAP_VARIABLE_Q("sys.g_systemInfo",        getSystemInfo ())

	JNC_MAP_TYPE(Lock)
	JNC_MAP_TYPE(Event)
	JNC_MAP_TYPE(NotificationEvent)
	JNC_MAP_TYPE(Thread)
	JNC_MAP_TYPE(Timer)
#if (_JNC_OS_WIN)
	JNC_MAP_TYPE(RegKey)
#endif
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................
