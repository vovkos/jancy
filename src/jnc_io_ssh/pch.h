#pragma once

#include "axl_g_Pch.h"

#ifndef ssize_t
#	define ssize_t intptr_t
#endif

#include <libssh2.h>

#include "axl_sys_Thread.h"
#include "axl_sys_Event.h"
#include "axl_sys_Time.h"
#include "axl_io_Socket.h"
#include "axl_sl_Construct.h"

#if (_AXL_ENV == AXL_ENV_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

using namespace axl;
