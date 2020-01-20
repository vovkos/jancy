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
#include "axl_sys_Time.h"
#include "axl_io_Socket.h"
#include "axl_io_Ssl.h"
#include "axl_cry_Bio.h"
#include "axl_cry_BigNum.h"
#include "axl_cry_Asn1.h"
#include "axl_cry_X509.h"
#include "axl_sl_Construct.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_err_Errno.h"
#include "axl_err_ErrorMgr.h"
#include "axl_mem_Pool.h"
#include "axl_g_Module.h"

#if (_AXL_OS_WIN)
#	include "axl_io_win_File.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

using namespace axl;

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"
#include "jnc_std_Buffer.h"

#ifndef ssize_t
#	define ssize_t intptr_t
#endif

#if (_AXL_OS_WIN)
#	pragma comment(lib, "ws2_32.lib")
#	pragma comment(lib, "crypt32.lib")
#elif (_AXL_OS_POSIX)
#	include <netinet/tcp.h>
#endif
