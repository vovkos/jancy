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

#if (_AXL_OS_WIN)
#	include "axl_io_win_FileHandle.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

#include "jnc_ExtensionLib.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"

#if (_AXL_OS_POSIX)
#	include <netinet/tcp.h>
#endif

using namespace axl;
