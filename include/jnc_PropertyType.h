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

/*!

\defgroup property-type Property Type
	\ingroup type-subsystem
	\import{jnc_PropertyType.h}

\brief Property type defines a signature of a property and is represented by a tuple of one or more function types.

Each property in Jancy has one getter and zero or more setters. Property type has information about function type for each and every accessor (getter or setter) a property of this type provides.

\addtogroup property-type
@{

\struct jnc_PropertyType
	\verbatim

	Opaque structure used as a handle to Jancy property type.

	Use functions from the `Property Type` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
