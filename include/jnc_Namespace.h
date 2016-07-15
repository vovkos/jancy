#pragma once

#include "jnc_Def.h"

//.............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_Namespace_findFunction (
	jnc_Namespace* nspace,
	const char* name
	);

JNC_EXTERN_C
jnc_Property*
jnc_Namespace_findProperty (
	jnc_Namespace* nspace,
	const char* name
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Namespace
{
	jnc_Function*
	findFunction (const char* name)
	{
		return jnc_Namespace_findFunction (this, name);
	}

	jnc_Property*
	findProperty (const char* name)
	{
		return jnc_Namespace_findProperty (this, name);
	}
};
#endif // _JNC_CORE

//.............................................................................
