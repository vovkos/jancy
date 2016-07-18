#pragma once

#define _JNC_FUNCTION_H

#include "jnc_RuntimeStructs.h"

//.............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_Function_getOverload (
	jnc_Function* function,
	size_t overloadIdx
	);

JNC_EXTERN_C
void*
jnc_Function_getMachineCode (jnc_Function* function);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Function
{
	jnc_Function*
	getOverload (size_t overloadIdx)
	{
		return jnc_Function_getOverload (this, overloadIdx);
	}

	void*
	getMachineCode ()
	{
		return jnc_Function_getMachineCode (this);
	}
};

#endif // _JNC_CORE

//.............................................................................
