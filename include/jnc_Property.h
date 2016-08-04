#pragma once

#define _JNC_PROPERTY_H

#include "jnc_PropertyType.h"

//.............................................................................

inline
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//.............................................................................
