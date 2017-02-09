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

#define _JNC_PROPERTY_H

#include "jnc_PropertyType.h"

/**

\defgroup property Property
	\ingroup module-subsystem
	\import{jnc_Property.h}

\addtogroup property
@{

\struct jnc_Property
	\verbatim

	Opaque structure used as a handle to Jancy property.

	Use functions from the :ref:`Property <cid-property>` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

JNC_INLINE
jnc_PropertyType*
jnc_Property_getType (jnc_Property* prop)
{
	return (jnc_PropertyType*) jnc_ModuleItem_getType ((jnc_ModuleItem*) prop);
}

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter (jnc_Property* prop);

JNC_EXTERN_C
jnc_Function*
jnc_Property_getSetter (jnc_Property* prop);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Property: jnc_ModuleItem
{
	jnc_PropertyType*
	getType ()
	{
		return jnc_Property_getType (this);
	}

	jnc_Function*
	getGetter ()
	{
		return jnc_Property_getGetter (this);
	}

	jnc_Function*
	getSetter ()
	{
		return jnc_Property_getSetter (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
