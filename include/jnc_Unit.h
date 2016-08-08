#pragma once

#define _JNC_UNIT_H

#include "jnc_Def.h"

//.............................................................................

JNC_EXTERN_C
jnc_ExtensionLib* 
jnc_Unit_getLib (jnc_Unit* unit);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Unit
{
	jnc_ExtensionLib* 
	getLib ()
	{
		return jnc_Unit_getLib (this);
	}
};

#endif // _JNC_CORE

//.............................................................................
