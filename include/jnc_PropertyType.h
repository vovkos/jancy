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

	Use functions from the `Property Type` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_PropertyTypeFlag
{
	jnc_PropertyTypeFlag_Const    = 0x010000,
	jnc_PropertyTypeFlag_Bindable = 0x020000,
};

typedef enum jnc_PropertyTypeFlag jnc_PropertyTypeFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getPropertyTypeFlagString(jnc_PropertyTypeFlag flag);

//..............................................................................

enum jnc_PropertyPtrTypeKind
{
	jnc_PropertyPtrTypeKind_Normal = 0,
	jnc_PropertyPtrTypeKind_Weak,
	jnc_PropertyPtrTypeKind_Thin,
	jnc_PropertyPtrTypeKind__Count,
};

typedef enum jnc_PropertyPtrTypeKind jnc_PropertyPtrTypeKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getPropertyPtrTypeKindString(jnc_PropertyPtrTypeKind ptrTypeKind);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_PropertyTypeFlag PropertyTypeFlag;

const PropertyTypeFlag
	PropertyTypeFlag_Const    = jnc_PropertyTypeFlag_Const,
	PropertyTypeFlag_Bindable = jnc_PropertyTypeFlag_Bindable;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getPropertyTypeFlagString(jnc_PropertyTypeFlag flag)
{
	return jnc_getPropertyTypeFlagString(flag);
}

//..............................................................................

typedef jnc_PropertyPtrTypeKind PropertyPtrTypeKind;

const PropertyPtrTypeKind
	PropertyPtrTypeKind_Normal = jnc_PropertyPtrTypeKind_Normal,
	PropertyPtrTypeKind_Weak   = jnc_PropertyPtrTypeKind_Weak,
	PropertyPtrTypeKind_Thin   = jnc_PropertyPtrTypeKind_Thin,
	PropertyPtrTypeKind__Count = jnc_PropertyPtrTypeKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getPropertyPtrTypeKindString(jnc_PropertyPtrTypeKind ptrTypeKind)
{
	return jnc_getPropertyPtrTypeKindString(ptrTypeKind);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
