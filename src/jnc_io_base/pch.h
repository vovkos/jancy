#pragma once

#define _JNC_SHARED_EXTENSION_LIB 1

#include "jnc_ext_Pch.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	pragma comment (lib, "setupapi.lib")
#	pragma comment (lib, "iphlpapi.lib")
#	include <winioctl.h>
#	include <setupapi.h>
#endif

#include "axl_io_Serial.h"
#include "axl_io_Socket.h"
#include "axl_io_NetworkAdapter.h"
#include "axl_io_MappedFile.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	include "axl_io_win_NamedPipe.h"
#endif

#include "jnc_ext_ExtensionLib.h"
#include "jnc_rt_CallSite.h"

using namespace axl;
