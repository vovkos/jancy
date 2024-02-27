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

#pragma once

#include "axl_io_Serial.h"
#include "axl_io_SerialPortEnumerator.h"
#include "axl_io_Socket.h"
#include "axl_io_NetworkAdapter.h"
#include "axl_io_MappedFile.h"
#include "axl_io_FilePathUtils.h"
#include "axl_sys_Process.h"
#include "axl_sys_Thread.h"
#include "axl_sys_Event.h"
#include "axl_sys_Time.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_mem_Pool.h"
#include "axl_err_Errno.h"
#include "axl_err_ErrorMgr.h"
#include "axl_g_Module.h"

#if (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

#if (_AXL_OS_WIN)
#	include "axl_io_win_NamedPipe.h"
#	include "axl_io_win_File.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#	include "axl_io_psx_Pty.h"
#endif

using namespace axl;

#include "jnc_ExtensionLib.h"
#include "jnc_Capability.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"
#include "jnc_Promise.h"

#if (_AXL_OS_WIN)
#	pragma comment(lib, "setupapi.lib")
#	pragma comment(lib, "iphlpapi.lib")
#	include <winioctl.h>
#elif (_AXL_OS_POSIX)
#	include <ifaddrs.h>
#	include <netdb.h>
#	include <net/if.h>
#	include <netinet/tcp.h>
#	include <sys/wait.h>
#	include <signal.h>
#endif
