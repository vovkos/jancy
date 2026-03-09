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

#include "pch.h"
#include "jnc_PropertyType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_PropertyType.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getPropertyTypeFlagString(jnc_PropertyTypeFlag flag) {
	static const char* stringTable[] = {
		"const",     // PropertyTypeFlag_Const    = 0x010000,
		"bindable",  // PropertyTypeFlag_Bindable = 0x020000,
	};

	size_t i = sl::getLoBitIdx8((uint8_t)(flag >> 16));
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-property-type-flag";
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getPropertyPtrKindString(jnc_PropertyPtrKind ptrKind) {
	static const char* stringTable[jnc_PropertyPtrKind__Count] = {
		"normal", // PropertyPtrKind_Normal = 0,
		"weak",   // PropertyPtrKind_Weak,
		"thin",   // PropertyPtrKind_Thin,
	};

	size_t i = ptrKind >> jnc_PtrTypeFlag__PtrKindBit;
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-property-ptr-kind";
}

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
