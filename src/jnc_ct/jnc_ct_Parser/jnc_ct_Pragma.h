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
	AXL_SL_HASH_TABLE_ENTRY("alignment",    Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("thinPointers", Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("exposedEnums", Pragma_ExposedEnums)
AXL_SL_END_STRING_HASH_TABLE()

//..............................................................................

} // namespace ct
} // namespace jnc
