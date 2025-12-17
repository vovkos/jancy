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

enum PragmaState {
	PragmaState_Default = 0,
	PragmaState_NoValue,
	PragmaState_CustomValue,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum Pragma {
	Pragma_Undefined = 0,
	Pragma_Alignment,
	Pragma_FirstBoolDefaultFalse,
	Pragma_FirstBool              = Pragma_FirstBoolDefaultFalse,
	Pragma_ThinPointers           = Pragma_FirstBoolDefaultFalse,
	Pragma_ExposedEnums,
	Pragma_RegexAnchored,
	Pragma_RegexFullMatch,
	Pragma_RegexCaseInsensitive,
	Pragma_RegexLatin1,
	Pragma_RegexOneLine,
	Pragma_FirstBoolDefaultTrue,
	Pragma_RegexUnanchored        = Pragma_FirstBoolDefaultTrue,
	Pragma_RegexCaseSensitive,
	Pragma_RegexUtf8,
	Pragma_RegexMultiLine,
	Pragma__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum PragmaDefault {
	PragmaDefault_Alignment = 8,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getPragmaName(Pragma pragmaKind);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(PragmaMap, Pragma)
	AXL_SL_HASH_TABLE_ENTRY("Alignment",            Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("ThinPointers",         Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("ExposedEnums",         Pragma_ExposedEnums)
	AXL_SL_HASH_TABLE_ENTRY("RegexUnanchored",      Pragma_RegexUnanchored)
	AXL_SL_HASH_TABLE_ENTRY("RegexAnchored",        Pragma_RegexAnchored)
	AXL_SL_HASH_TABLE_ENTRY("RegexFullMatch",       Pragma_RegexFullMatch)
	AXL_SL_HASH_TABLE_ENTRY("RegexCaseSensitive",   Pragma_RegexCaseSensitive)
	AXL_SL_HASH_TABLE_ENTRY("RegexCaseInsensitive", Pragma_RegexCaseInsensitive)
	AXL_SL_HASH_TABLE_ENTRY("RegexUtf8",            Pragma_RegexUtf8)
	AXL_SL_HASH_TABLE_ENTRY("RegexLatin1",          Pragma_RegexLatin1)
	AXL_SL_HASH_TABLE_ENTRY("RegexMultiLine",       Pragma_RegexMultiLine)
	AXL_SL_HASH_TABLE_ENTRY("RegexOneLine",         Pragma_RegexOneLine)
AXL_SL_END_STRING_HASH_TABLE()

//..............................................................................

} // namespace ct
} // namespace jnc
