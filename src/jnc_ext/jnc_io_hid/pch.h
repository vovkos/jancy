#pragma once

//..............................................................................

// AXL

#include "axl_io_HidPch.h"
#include "axl_sl_Construct.h"
#include "axl_sl_CircularBuffer.h"
#include "axl_sl_HandleTable.h"
#include "axl_sl_StringHashTable.h"
#include "axl_io_File.h"
#include "axl_io_MappedFile.h"
#include "axl_io_Serial.h"
#include "axl_sys_Thread.h"
#include "axl_sys_Time.h"
#include "axl_sys_win_NtStatus.h"
#include "axl_mem_Pool.h"
#include "axl_err_Errno.h"
#include "axl_err_ErrorMgr.h"
#include "axl_g_Module.h"
#include "axl_zip_ZipReader.h"
#include "axl_io_HidDevice.h"
#include "axl_io_HidEnumerator.h"
#if (_JNC_IO_USBMON)
#	include "axl_io_HidMonEnumerator.h"
#endif
#include "axl_io_HidRd.h"
#include "axl_io_HidDb.h"

#if (_AXL_OS_POSIX)
#	include "axl_io_psx_Pipe.h"
#endif

using namespace axl;

// Jancy

#include "jnc_ExtensionLib.h"
#include "jnc_Capability.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"
#include "jnc_std_Buffer.h"

//..............................................................................
