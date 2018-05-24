#pragma once

//..............................................................................

// AXL

#include "axl_sl_Construct.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_io_File.h"
#include "axl_io_Serial.h"
#include "axl_sys_Thread.h"
#include "axl_sys_Time.h"
#include "axl_sys_win_NtStatus.h"
#include "axl_mem_Pool.h"
#include "axl_err_Errno.h"
#include "axl_err_ErrorMgr.h"
#include "axl_g_Module.h"

#if (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

using namespace axl;

// devmon

#if (_AXL_OS_WIN)
#	include "dm_win_Pch.h"
#	include "dm_win_Monitor.h"
#elif (_AXL_OS_LINUX)
#	include "dm_lnx_Pch.h"
#	include "dm_lnx_Monitor.h"
#else
#	error Your OS is not currently supported
#endif

// Jancy

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"

//..............................................................................
