#pragma once

#define _JNC_PROPERTY_H

#include "jnc_PropertyType.h"

//.............................................................................

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_Property_getItemDecl (jnc_Property* prop);

JNC_EXTERN_C
jnc_Namespace*
jnc_Property_getNamespace (jnc_Property* prop);

JNC_EXTERN_C
jnc_PropertyType*
jnc_Property_getType (jnc_Property* prop);

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
	jnc_ModuleItemDecl*
	getItemDecl ()
	{
		return jnc_Property_getItemDecl (this);
	}

	jnc_Namespace*
	getNamespace ()
	{
		return jnc_Property_getNamespace (this);
	}

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
