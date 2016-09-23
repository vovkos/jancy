#pragma once

#define _JNC_ALIAS_H

#include "jnc_Type.h"

/// \addtogroup variable
/// @{

//.............................................................................

JNC_EXTERN_C
const char*
jnc_Alias_getInitializerString_v (jnc_Alias* alias);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Alias: jnc_ModuleItem
{
	const char*
	getInitializerString_v ()
	{
		return jnc_Alias_getInitializerString_v (this);
	}
};

#endif // _JNC_CORE

//.............................................................................

/// @}
