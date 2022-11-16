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

#include "axl_io_UsbMonEnumerator.h"

#if (_AXL_OS_WIN)
#	include "axl_io_win_UsbPcap.h"
#	include "axl_io_win_UsbPcapTransferParser.h"
#elif (_AXL_OS_LINUX)
#	include "axl_io_UsbContext.h"
#	include "axl_io_lnx_UsbMon.h"
#	include "axl_io_lnx_UsbMonTransferParser.h"
#	include "axl_io_psx_Pipe.h"
#else
#	error This OS is not currently supported
#endif

using namespace axl;

// Jancy

#include "jnc_ExtensionLib.h"
#include "jnc_Capability.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"

//..............................................................................
