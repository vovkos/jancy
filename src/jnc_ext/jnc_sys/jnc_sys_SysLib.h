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

#include "jnc_Def.h"

namespace jnc {
namespace sys {

//..............................................................................

// {D8C0847C-93D5-4146-B795-5DB1A111855A}
JNC_DEFINE_GUID (
	g_sysLibGuid,
	0xd8c0847c, 0x93d5, 0x4146, 0xb7, 0x95, 0x5d, 0xb1, 0xa1, 0x11, 0x85, 0x5a
	);

enum SysLibCacheSlot
{
	SysLibCacheSlot_Lock,
	SysLibCacheSlot_Event,
	SysLibCacheSlot_NotificationEvent,
	SysLibCacheSlot_Thread,
	SysLibCacheSlot_Timer,
};

//..............................................................................

enum CpuKind
{
	CpuKind_Ia32,
	CpuKind_Amd64,
	CpuKind_Arm32,
	CpuKind_Arm64,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum OsKind
{
	OsKind_Windows,
	OsKind_Linux,
	OsKind_Solaris,
	OsKind_Bsd,
	OsKind_Mac,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum OsFlag
{
	OsFlag_Posix,
	OsFlag_Bsd,
	OsFlag_Darwin,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum CppKind
{
	CppKind_Msc,
	CppKind_Gcc,
	CppKind_Icc,
	CppKind_Clang,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SystemInfo
{
	uint_t m_cpuKind;
	uint_t m_osKind;
	uint_t m_osFlags;
	uint_t m_cppKind;
	uint_t m_cppFlags;
	uint_t m_processorCount;
	size_t m_pageSize;
	size_t m_mappingAlignFactor;
};

//..............................................................................

} // namespace sys
} // namespace jnc
