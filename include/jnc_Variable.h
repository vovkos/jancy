#pragma once

#define _JNC_VARIABLE_H

#include "jnc_Type.h"

//.............................................................................

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_Variable_getItemDecl (jnc_Variable* variable);

JNC_EXTERN_C
jnc_Type*
jnc_Variable_getType (jnc_Variable* variable);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Variable: jnc_ModuleItem
{
	jnc_ModuleItemDecl*
	getItemDecl ()
	{
		return jnc_Variable_getItemDecl (this);
	}

	jnc_Type*
	getType ()
	{
		return jnc_Variable_getType (this);
	}
};

#endif // _JNC_CORE

//.............................................................................
