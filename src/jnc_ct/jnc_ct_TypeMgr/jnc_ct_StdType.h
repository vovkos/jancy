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

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_StdNamespace.h"
#include "jnc_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getStdTypeName(StdType stdType);

const StdItemSource*
getStdTypeSource(StdType stdType);

//..............................................................................

enum StdTypedef {
	StdTypedef_variant_t,
	StdTypedef_string_t,
	StdTypedef_uint_t,
	StdTypedef_intptr_t,
	StdTypedef_uintptr_t,
	StdTypedef_size_t,
	StdTypedef_int8_t,
	StdTypedef_utf8_t,
	StdTypedef_uint8_t,
	StdTypedef_uchar_t,
	StdTypedef_byte_t,
	StdTypedef_int16_t,
	StdTypedef_utf16_t,
	StdTypedef_uint16_t,
	StdTypedef_ushort_t,
	StdTypedef_word_t,
	StdTypedef_int32_t,
	StdTypedef_utf32_t,
	StdTypedef_uint32_t,
	StdTypedef_dword_t,
	StdTypedef_int64_t,
	StdTypedef_uint64_t,
	StdTypedef_ulong_t,
	StdTypedef_qword_t,
	StdTypedef__Count,
};

//..............................................................................

#define JNC_MAP_STD_TYPE(stdType, Type) \
	if (module->m_typeMgr.isStdTypeUsed(stdType)) { \
		JNC_MAP_TYPE(Type); \
	}

//..............................................................................

} // namespace ct
} // namespace jnc
