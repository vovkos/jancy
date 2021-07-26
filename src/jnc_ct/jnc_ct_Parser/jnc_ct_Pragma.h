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

namespace jnc {
namespace ct {

//..............................................................................

enum Pragma
{
	Pragma_Undefined = 0,
	Pragma_Alignment,
	Pragma_ThinPointers,
	Pragma_ExposedEnums,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(PragmaMap, Pragma)
	AXL_SL_HASH_TABLE_ENTRY("Alignment",    Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("ThinPointers", Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("ExposedEnums", Pragma_ExposedEnums)
AXL_SL_END_STRING_HASH_TABLE()

//..............................................................................

enum PragmaDefault
{
	PragmaDefault_Alignment        = 8,
	PragmaDefault_PointerModifiers = 0,
	PragmaDefault_EnumFlags        = 0,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PragmaSettings
{
	size_t m_fieldAlignment;
	uint_t m_pointerModifiers;
	uint_t m_enumFlags;

	PragmaSettings()
	{
		m_fieldAlignment = PragmaDefault_Alignment;
		m_pointerModifiers = PragmaDefault_PointerModifiers;
		m_enumFlags = PragmaDefault_EnumFlags;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
