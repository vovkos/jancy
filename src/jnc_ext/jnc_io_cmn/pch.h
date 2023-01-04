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

#include "axl_io_Socket.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_sys_Event.h"
#include "axl_err_Errno.h"
#include "axl_mem_Pool.h"

#if (_AXL_OS_WIN)
#	include "axl_io_win_File.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

#if (_JNC_IO_SSL)
#	include "axl_cry_Bio.h"
#	include "axl_cry_Dh.h"
#	include "axl_cry_EcKey.h"
#	include "axl_io_Ssl.h"
#	include "axl_io_MappedFile.h"
#endif

#if (_JNC_IO_USB)
#	include "axl_io_UsbDeviceStrings.h"
#endif

#include "jnc_ExtensionLib.h"
#include "jnc_Capability.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"
#include "jnc_Promise.h"

#if (_AXL_OS_POSIX)
#	include <netinet/tcp.h>
#endif

using namespace axl;
