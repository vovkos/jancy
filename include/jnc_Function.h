#pragma once

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
#ifdef __cplusplus
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
#endif
};

#endif // _JNC_CORE

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Function*
jnc_getMulticastCallMethod (jnc_Multicast* multicast);

inline
void*
jnc_getMulticastCallMethodMachineCode (jnc_Multicast* multicast)
{
	jnc_Function* callMethod = jnc_getMulticastCallMethod (multicast);
	return callMethod ? jnc_Function_getMachineCode (callMethod) : NULL;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Function Function;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Function*
getMulticastCallMethod (jnc_Multicast* multicast)
{
	return jnc_getMulticastCallMethod (multicast);
}

inline
void*
getMulticastCallMethodMachineCode (jnc_Multicast* multicast)
{
	return jnc_getMulticastCallMethodMachineCode (multicast);
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
