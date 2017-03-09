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
#include "axl_sl_Construct.h"
#include "axl_g_Module.h"

#if (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

using namespace axl;

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

#ifndef ssize_t
#	define ssize_t intptr_t
#endif

#include <libssh2.h>

#if (_AXL_OS_WIN)
#	pragma comment (lib, "ws2_32.lib")
#	pragma comment (lib, "crypt32.lib")
#endif
