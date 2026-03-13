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

#define _JNC_PROPERTYTYPE_H

#include "jnc_FunctionType.h"

/**

\defgroup property-type Property Type
	\ingroup type-subsystem
	\import{jnc_PropertyType.h}

\brief Property type defines a signature of a property and is represented by a tuple of one or more function types.

Each property in Jancy has one getter and zero or more setters. Property type has information about function type for each and every accessor(getter or setter) a property of this type provides.

\addtogroup property-type
@{

\struct jnc_PropertyType
	\verbatim

	Opaque structure used as a handle to Jancy property type.

	Use functions from the `Property Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_PropertyTypeFlag {
	jnc_PropertyTypeFlag_Const    = 0x010000,
	jnc_PropertyTypeFlag_Bindable = 0x020000,
	jnc_PropertyTypeFlag_All      = 0x030000,
};

typedef enum jnc_PropertyTypeFlag jnc_PropertyTypeFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getPropertyTypeFlagString(jnc_PropertyTypeFlag flag);

//..............................................................................

enum jnc_PropertyPtrKind {
	jnc_PropertyPtrKind_Normal = 0x000000,
	jnc_PropertyPtrKind_Weak   = 0x100000,
	jnc_PropertyPtrKind_Thin   = 0x200000,
	jnc_PropertyPtrKind__Count = 3,
};

typedef enum jnc_PropertyPtrKind jnc_PropertyPtrKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getPropertyPtrKindString(jnc_PropertyPtrKind ptrKind);

JNC_INLINE
jnc_PropertyPtrKind
jnc_getPropertyPtrKindFromFlags(uint_t flags) {
	return (jnc_PropertyPtrKind)jnc_getPtrKindFromFlags(flags);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_PropertyTypeFlag PropertyTypeFlag;

const PropertyTypeFlag
	PropertyTypeFlag_Const    = jnc_PropertyTypeFlag_Const,
	PropertyTypeFlag_Bindable = jnc_PropertyTypeFlag_Bindable,
	PropertyTypeFlag_All      = jnc_PropertyTypeFlag_All;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getPropertyTypeFlagString(jnc_PropertyTypeFlag flag) {
	return jnc_getPropertyTypeFlagString(flag);
}

//..............................................................................

typedef jnc_PropertyPtrKind PropertyPtrKind;

const PropertyPtrKind
	PropertyPtrKind_Normal = jnc_PropertyPtrKind_Normal,
	PropertyPtrKind_Weak   = jnc_PropertyPtrKind_Weak,
	PropertyPtrKind_Thin   = jnc_PropertyPtrKind_Thin,
	PropertyPtrKind__Count = jnc_PropertyPtrKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getPropertyPtrKindString(jnc_PropertyPtrKind ptrKind) {
	return jnc_getPropertyPtrKindString(ptrKind);
}

inline
PropertyPtrKind
getPropertyPtrKindFromFlags(uint_t flags) {
	return jnc_getPropertyPtrKindFromFlags(flags);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
