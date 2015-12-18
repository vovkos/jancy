// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "axl_g_Pch.h"

#if (_AXL_ENV == AXL_ENV_POSIX)
#	include <signal.h>
#endif

//.............................................................................

// LLVM

// warning C4800: 'unsigned int' : forcing value to bool 'true' or 'false' (performance warning)
// warning C4244: 'return' : conversion from 'uint64_t' to 'unsigned int', possible loss of data
// warning C4624: destructor could not be generated because a base class destructor is inaccessible

#pragma warning (disable: 4800)
#pragma warning (disable: 4244)
#pragma warning (disable: 4624)

#undef min
#undef max

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Support/Dwarf.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/PassManager.h"
#include "llvm/DIBuilder.h"
#include "llvm/DebugInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ObjectImage.h"

// LLVM JIT forces linkage to LLVM libraries if JIT is merely included;
// we want to be able avoid that (i.e. if a libraries defines LLVM-dependent classes, but
// application does not use those classes -- then why link to LLVM?)

#ifndef _JNC_LLVM_NO_JIT
#	include "llvm/ExecutionEngine/JIT.h"
#	include "llvm/ExecutionEngine/JITEventListener.h"
#	include "llvm/ExecutionEngine/JITMemoryManager.h"
#	include "llvm/ExecutionEngine/JITEventListener.h"
#	include "llvm/ExecutionEngine/MCJIT.h"
#endif

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "llvm/CodeGen/MachineCodeInfo.h"
#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/CodeGen/GCs.h"

// make an easily comparable version like 0x0304

#define LLVM_VERSION ((LLVM_VERSION_MAJOR << 8) | LLVM_VERSION_MINOR)

#pragma warning (default: 4800)
#pragma warning (default: 4244)
#pragma warning (default: 4624)

//.............................................................................

// AXL

#include "axl_g_Time.h"
#include "axl_mem_Block.h"
#include "axl_err_ParseError.h"
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
#include "axl_mt_Event.h"
#include "axl_mt_Thread.h"
#include "axl_mt_TlsMgr.h"
#include "axl_mt_TlsSlot.h"
#include "axl_mt_LongJmpTry.h"
#include "axl_sys_DynamicLibrary.h"
#include "axl_mt_Singleton.h"
#include "axl_zip_ZipReader.h"

#if (_AXL_ENV == AXL_ENV_WIN)
#	include "axl_mem_win_VirtualMemory.h"
#elif (_AXL_ENV == AXL_ENV_POSIX)
#	include "axl_io_psx_Mapping.h"
#	if (_AXL_POSIX == AXL_POSIX_DARWIN)
#		include "axl_mt_drw_Semaphore.h"
#	else
#		include "axl_mt_psx_Sem.h"
#	endif
#endif

using namespace axl;
