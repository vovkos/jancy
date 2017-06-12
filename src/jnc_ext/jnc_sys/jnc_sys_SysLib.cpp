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
#include "jnc_sys_NotificationEvent.h"
#include "jnc_sys_Thread.h"
#include "jnc_sys_Timer.h"

#include "sys_globals.jnc.cpp"
#include "sys_Lock.jnc.cpp"
#include "sys_Event.jnc.cpp"
#include "sys_NotificationEvent.jnc.cpp"
#include "sys_Thread.jnc.cpp"
#include "sys_Timer.jnc.cpp"

namespace jnc {
namespace sys {

//..............................................................................

intptr_t
getCurrentThreadId ()
{
	return (intptr_t) axl::sys::getCurrentThreadId ();
}

DataPtr
getEnv (DataPtr namePtr)
{
	if (!namePtr.m_p)
		return g_nullPtr;

	const char* value = getenv ((const char*) namePtr.m_p);
	return strDup (value);
}

void
setEnv (
	DataPtr namePtr,
	DataPtr valuePtr
	)
{
#if (_AXL_OS_WIN)
	char buffer [256];
	sl::String envString (ref::BufKind_Stack, buffer, sizeof (buffer));
	envString.format ("%s=%s", namePtr.m_p, valuePtr.m_p ? valuePtr.m_p : "");
	_putenv (envString);
#else
	setenv (
		(const char*) namePtr.m_p,
		(const char*) valuePtr.m_p,
		true
		);
#endif
}

uint64_t
getTimestamp ()
{
	return axl::sys::getTimestamp ();
}

void
sleep (uint32_t msCount)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->enterWaitRegion ();
	axl::sys::sleep (msCount);
	gcHeap->leaveWaitRegion ();
}

void
collectGarbage ()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->collect ();
}

GcStats
getGcStats ()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	GcStats stats;
	gcHeap->getStats (&stats);
	return stats;
}

GcSizeTriggers
getGcTriggers ()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	GcSizeTriggers triggers;
	gcHeap->getSizeTriggers (&triggers);
	return triggers;
}

void
setGcTriggers (GcSizeTriggers triggers)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->setSizeTriggers (&triggers);
}

//..............................................................................

} // namespace sys
} // namespace jnc

using namespace jnc::sys;

JNC_DEFINE_LIB (
	jnc_SysLib,
	g_sysLibGuid,
	"SysLib",
	"Jancy standard extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (jnc_SysLib)
	JNC_LIB_SOURCE_FILE ("sys_globals.jnc", g_sys_globalsSrc)
	JNC_LIB_SOURCE_FILE ("sys_Lock.jnc",    g_sys_LockSrc)
	JNC_LIB_SOURCE_FILE ("sys_Event.jnc",   g_sys_EventSrc)
	JNC_LIB_SOURCE_FILE ("sys_NotificationEvent.jnc", g_sys_NotificationEventSrc)
	JNC_LIB_SOURCE_FILE ("sys_Thread.jnc",  g_sys_ThreadSrc)
	JNC_LIB_SOURCE_FILE ("sys_Timer.jnc",   g_sys_TimerSrc)

	JNC_LIB_IMPORT ("sys_globals.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (jnc_SysLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Lock)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Event)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (NotificationEvent)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Thread)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Timer)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (jnc_SysLib)
	JNC_MAP_FUNCTION ("sys.getCurrentThreadId", getCurrentThreadId)
	JNC_MAP_FUNCTION ("sys.getTimestamp",       getTimestamp)
	JNC_MAP_FUNCTION ("sys.sleep",              jnc::sys::sleep)
	JNC_MAP_FUNCTION ("sys.collectGarbage",     collectGarbage)
	JNC_MAP_FUNCTION ("sys.getGcStats",         getGcStats)
	JNC_MAP_PROPERTY ("sys.g_gcTriggers",       getGcTriggers, setGcTriggers)
	JNC_MAP_PROPERTY ("sys.g_env",              getEnv, setEnv)

	JNC_MAP_TYPE (Lock)
	JNC_MAP_TYPE (Event)
	JNC_MAP_TYPE (Thread)
	JNC_MAP_TYPE (Timer)
JNC_END_LIB_FUNCTION_MAP ()

//..............................................................................
