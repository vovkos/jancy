// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_sys_Lock.h"
#include "jnc_sys_Event.h"
#include "jnc_sys_Thread.h"
#include "jnc_sys_Timer.h"

#include "sys_globals.jnc.cpp"
#include "sys_Lock.jnc.cpp"
#include "sys_Event.jnc.cpp"
#include "sys_Thread.jnc.cpp"
#include "sys_Timer.jnc.cpp"

namespace jnc {
namespace sys {

//.............................................................................

class SysLib: public ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("sys.getCurrentThreadId", getCurrentThreadId)
		JNC_MAP_FUNCTION ("sys.getTimestamp",       getTimestamp)
		JNC_MAP_FUNCTION ("sys.sleep",              sleep)

		JNC_MAP_TYPE (Lock)
		JNC_MAP_TYPE (Event)
		JNC_MAP_TYPE (Thread)
		JNC_MAP_TYPE (Timer)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Lock)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Event)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Thread)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Timer)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE ("sys_Lock.jnc",    g_sys_LockSrc)
		JNC_LIB_SOURCE_FILE ("sys_Event.jnc",   g_sys_EventSrc)
		JNC_LIB_SOURCE_FILE ("sys_Thread.jnc",  g_sys_ThreadSrc)
		JNC_LIB_SOURCE_FILE ("sys_Timer.jnc",   g_sys_TimerSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()

	JNC_BEGIN_LIB_FORCED_EXPORT ()
		JNC_LIB_FORCED_SOURCE_FILE ("sys_globals.jnc", g_sys_globalsSrc)
	JNC_END_LIB_FORCED_EXPORT ()

public:
	static
	intptr_t
	getCurrentThreadId ()
	{
		return (intptr_t) axl::sys::getCurrentThreadId ();
	}

	static
	uint64_t
	getTimestamp ()
	{
		return axl::sys::getTimestamp ();
	}

	static
	void
	sleep (uint32_t msCount);
};

//.............................................................................

} // namespace sys
} // namespace jnc
