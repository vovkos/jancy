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

#include "axl_sys_Thread.h"
#include "axl_sys_Event.h"
#include "axl_io_UsbContext.h"
#include "axl_io_UsbEventThread.h"
#include "axl_io_UsbDevice.h"
#include "axl_io_UsbTransfer.h"
#include "axl_sl_Construct.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_mem_Pool.h"
#include "axl_err_ErrorMgr.h"
#include "axl_g_Module.h"

#if (_AXL_OS_WIN)
#	include "axl_io_win_File.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

using namespace axl;

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"
#include "jnc_Promise.h"
