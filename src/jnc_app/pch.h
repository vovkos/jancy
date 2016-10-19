#pragma once

//..............................................................................

// Jancy

#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"
#include "jnc_ExtensionLib.h"

// AXL

#include "axl_sl_CmdLineParser.h"
#include "axl_sl_Singleton.h"
#include "axl_sl_BoxList.h"
#include "axl_sys_Time.h"
#include "axl_io_FilePathUtils.h"
#include "axl_io_FileEnumerator.h"

using namespace axl;

#if (_JNC_OS_WIN)
#	include <io.h>
#endif

//..............................................................................
