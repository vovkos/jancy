#pragma once

typedef struct jnc_Function jnc_Function;
typedef struct jnc_Multicast jnc_Multicast;

//.............................................................................

jnc_Function*
jnc_Function_getOverload (
	jnc_Function* self,
	size_t overloadIdx
	);

void*
jnc_Function_getMachineCode (jnc_Function* self);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
