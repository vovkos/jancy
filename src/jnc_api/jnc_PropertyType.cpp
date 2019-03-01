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
jnc_getPropertyTypeFlagString(jnc_PropertyTypeFlag flag)
{
	static const char* stringTable[] =
	{
		"const",     // PropertyTypeFlag_Const    = 0x010000,
		"bindable",  // PropertyTypeFlag_Bindable = 0x020000,
	};

	size_t i = sl::getLoBitIdx32(flag >> 16);
	return i < countof(stringTable) ?
		stringTable[i] :
		"undefined-property-type-flag";
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getPropertyPtrTypeKindString(jnc_PropertyPtrTypeKind ptrTypeKind)
{
	static const char* stringTable[jnc_PropertyPtrTypeKind__Count] =
	{
		"closure",  // PropertyPtrTypeKind_Normal = 0,
		"weak",     // PropertyPtrTypeKind_Weak,
		"thin",     // PropertyPtrTypeKind_Thin,
	};

	return (size_t)ptrTypeKind < countof(stringTable) ?
		stringTable[ptrTypeKind] :
		"undefined-property-ptr-kind";
}

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
