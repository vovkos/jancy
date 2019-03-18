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

#ifndef __STDC_LIMIT_MACROS
#	define __STDC_LIMIT_MACROS 1
#endif

#ifndef __STDC_CONSTANT_MACROS
#	define __STDC_CONSTANT_MACROS 1
#endif

#ifdef _WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS 1
#	endif

#	ifndef _SCL_SECURE_NO_WARNINGS
#		define _SCL_SECURE_NO_WARNINGS 1
#	endif

#	ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#		define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#	endif
#endif

//..............................................................................

// LLVM

#pragma warning(disable: 4141) // warning C4141: 'inline' : used more than once
#pragma warning(disable: 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable: 4291) // warning C4291: no matching operator delete found; memory will not be freed if initialization throws an exception
#pragma warning(disable: 4244) // warning C4244: 'return' : conversion from 'uint64_t' to 'unsigned int', possible loss of data
#pragma warning(disable: 4267) // warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4355) // warning C4355: 'this' : used in base member initializer list
#pragma warning(disable: 4624) // warning C4624: destructor could not be generated because a base class destructor is inaccessible
#pragma warning(disable: 4800) // warning C4800: 'unsigned int' : forcing value to bool 'true' or 'false' (performance warning)

#include <llvm/Config/llvm-config.h>

// make an easily comparable version like 0x0304

#define LLVM_VERSION ((LLVM_VERSION_MAJOR << 8) | LLVM_VERSION_MINOR)

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>

// they moved things around in LLVM 3.5

#if (LLVM_VERSION < 0x0305)
#	include <llvm/PassManager.h>
#	include <llvm/DIBuilder.h>
#	include <llvm/DebugInfo.h>
#	include <llvm/Analysis/Verifier.h>
#else
#	include <llvm/IR/LegacyPassManager.h>
#	include <llvm/IR/DIBuilder.h>
#	include <llvm/IR/DebugInfo.h>
#	include <llvm/IR/Verifier.h>
#endif

#include <llvm/Support/Debug.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

// LLVM JIT forces linkage to LLVM libraries if JIT is merely included;
// we want to be able to avoid that (i.e. if a libraries defines LLVM-dependent classes, but
// application does not use those classes -- then why link to LLVM?)

#ifndef _JNC_LLVM_NO_JIT
#	if (LLVM_VERSION < 0x0306) // legacy JIT is gone in LLVM 3.6
#		include <llvm/ExecutionEngine/JIT.h>
#		include <llvm/ExecutionEngine/JITMemoryManager.h>
#	endif

#	include <llvm/ExecutionEngine/JITEventListener.h>
#	include <llvm/ExecutionEngine/MCJIT.h>
#endif

#pragma warning(default: 4141)
#pragma warning(default: 4146)
#pragma warning(default: 4291)
#pragma warning(default: 4244)
#pragma warning(default: 4267)
#pragma warning(default: 4355)
#pragma warning(default: 4624)
#pragma warning(default: 4800)

//..............................................................................

// they changed the type model of llvm::DIBuilder in LLVM 3.7
// therefore, we define and use version-neutral typedefs

template <typename T>
class InitializedPtr
{
protected:
	T* m_p;

public:
	InitializedPtr()
	{
		m_p = NULL;
	}

	InitializedPtr(T* p)
	{
		m_p = p;
	}

	operator bool() const
	{
		return m_p != NULL;
	}

	template <typename T2>
	operator T2* () const
	{
		return (T2*)m_p;
	}

	T*
	operator -> () const
	{
		return m_p;
	}

	InitializedPtr&
	operator = (T* p)
	{
		m_p = p;
		return *this;
	}
};

namespace llvm {

#if (LLVM_VERSION < 0x0307)
#	if (LLVM_VERSION < 0x0306)

typedef DICompositeType DISubroutineType_vn;
typedef Value Metadata;

#	else

typedef DISubroutineType DISubroutineType_vn;

#	endif

typedef DIArray DINodeArray;

typedef DIType DIType_vn;
typedef DICompositeType DICompositeType_vn;
typedef DIGlobalVariable DIGlobalVariable_vn;
typedef DIVariable DIVariable_vn;
typedef DISubprogram DISubprogram_vn;
typedef DILexicalBlock DILexicalBlock_vn;
typedef DIScope DIScope_vn;
typedef DIFile DIFile_vn;

#else

typedef InitializedPtr<DIType> DIType_vn;
typedef InitializedPtr<DICompositeType> DICompositeType_vn;
typedef InitializedPtr<DISubroutineType> DISubroutineType_vn;
typedef InitializedPtr<DIGlobalVariable> DIGlobalVariable_vn;
typedef InitializedPtr<DILocalVariable> DILocalVariable_vn;
typedef InitializedPtr<DIVariable> DIVariable_vn;
typedef InitializedPtr<DISubprogram> DISubprogram_vn;
typedef InitializedPtr<DILexicalBlock> DILexicalBlock_vn;
typedef InitializedPtr<DIScope> DIScope_vn;
typedef InitializedPtr<DIFile> DIFile_vn;

#endif

#if (LLVM_VERSION < 0x0400)

typedef unsigned DIFlags;

#else

typedef DINode::DIFlags DIFlags;

#endif

#if (LLVM_VERSION < 0x0500)

typedef SynchronizationScope SynchronizationScope_vn;
const SynchronizationScope DefaultSynchronizationScope_vn = CrossThread;

#else

typedef SyncScope::ID SynchronizationScope_vn;
const SyncScope::ID DefaultSynchronizationScope_vn = llvm::SyncScope::System;

#endif

} // namespace llvm

//..............................................................................

// AXL

#include "axl_mem_Block.h"
#include "axl_err_Errno.h"
#include "axl_sl_List.h"
#include "axl_sl_ArrayList.h"
#include "axl_sl_AutoPtrArray.h"
#include "axl_sl_BitMap.h"
#include "axl_sl_StringHashTable.h"
#include "axl_sl_CmdLineParser.h"
#include "axl_sl_BoxList.h"
#include "axl_sl_ByteOrder.h"
#include "axl_sl_HandleTable.h"
#include "axl_sl_Singleton.h"
#include "axl_sl_Construct.h"
#include "axl_fsm_Regex.h"
#include "axl_enc_HexEncoding.h"
#include "axl_enc_EscapeEncoding.h"
#include "axl_io_FilePathUtils.h"
#include "axl_io_MappedFile.h"
#include "axl_io_FilePathUtils.h"
#include "axl_lex_RagelLexer.h"
#include "axl_sys_Time.h"
#include "axl_sys_Event.h"
#include "axl_sys_Thread.h"
#include "axl_sys_TlsMgr.h"
#include "axl_sys_TlsSlot.h"
#include "axl_sys_SjljTry.h"
#include "axl_sys_DynamicLibrary.h"
#include "axl_zip_ZipReader.h"

#if (_AXL_OS_WIN)
#	include "axl_sys_win_VirtualMemory.h"
#elif (_AXL_OS_POSIX)
#	include "axl_io_psx_Mapping.h"
#	if (_AXL_OS_DARWIN)
#		include "axl_sys_drw_Semaphore.h"
#	else
#		include "axl_sys_psx_Sem.h"
#	endif
#endif

using namespace axl;

//..............................................................................

#if (_AXL_OS_POSIX)
#	include <signal.h>
#endif

//..............................................................................

// converstions for axl::sl::StringRef

AXL_SELECT_ANY struct ToAxl* toAxl;
AXL_SELECT_ANY struct ToStl* toStl;
AXL_SELECT_ANY struct ToLlvm* toLlvm;

inline
std::string
operator >> (
	const sl::StringRef& string,
	const ToStl*
	)
{
	return std::string(string.cp(), string.getLength());
}

inline
llvm::StringRef
operator >> (
	const sl::StringRef& string,
	const ToLlvm*
	)
{
	return llvm::StringRef(string.cp(), string.getLength());
}

inline
sl::String
operator >> (
	llvm::StringRef& string,
	const ToAxl*
	)
{
	return sl::String(string.data(), string.size());
}

//..............................................................................
