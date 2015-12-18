#pragma once

#define _JNC_SHARED_EXTENSION_LIB 1

#include "jnc_ext_Pch.h"

#ifndef ssize_t
#	define ssize_t intptr_t
#endif

#include <libssh2.h>

#include "axl_io_Socket.h"

#if (_AXL_ENV == AXL_ENV_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

#include "jnc_ext_ExtensionLib.h"
#include "jnc_rt_CallSite.h"

using namespace axl;
