// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "axl_g_Pch.h"

//.............................................................................

// AXL

#include "axl_sys_Time.h"
#include "axl_mem_Block.h"
#include "axl_err_Errno.h"
#include "axl_sl_List.h"
#include "axl_sl_ArraySlice.h"
#include "axl_sl_ArrayList.h"
#include "axl_sl_AutoPtrArray.h"
#include "axl_sl_BitMap.h"
#include "axl_sl_StringSlice.h"
#include "axl_sl_StringCache.h"
#include "axl_sl_StringHashTable.h"
#include "axl_sl_CmdLineParser.h"
#include "axl_sl_BoxList.h"
#include "axl_sl_ByteOrder.h"
#include "axl_sl_HandleTable.h"
#include "axl_fsm_RegExp.h"
#include "axl_fsm_StdRegExpNameMgr.h"
#include "axl_enc_HexEncoding.h"
#include "axl_enc_EscapeEncoding.h"
#include "axl_io_FilePathUtils.h"
#include "axl_io_MappedFile.h"
#include "axl_io_FilePathUtils.h"
#include "axl_lex_RagelLexer.h"
#include "axl_sys_Event.h"
#include "axl_sys_Thread.h"
#include "axl_sys_TlsMgr.h"
#include "axl_sys_TlsSlot.h"
#include "axl_sys_LongJmpTry.h"
#include "axl_sys_DynamicLibrary.h"
#include "axl_sl_Singleton.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	include "axl_sys_win_VirtualMemory.h"
#elif (_AXL_ENV == AXL_ENV_POSIX)
#	include "axl_io_psx_Mapping.h"
#	include "axl_sys_psx_Sem.h"
#endif

using namespace axl;
