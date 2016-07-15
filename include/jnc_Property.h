#pragma once

#include "jnc_Def.h"

//.............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_Property_getGetter (jnc_Property* prop);

JNC_EXTERN_C
jnc_Function*
jnc_Property_getSetter (jnc_Property* prop);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Property
{
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
