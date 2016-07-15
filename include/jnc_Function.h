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
int
jnc_Type_getSize (jnc_Type* type);

JNC_EXTERN_C
int
jnc_isMulticastWeak (jnc_Multicast* multicast);

JNC_EXTERN_C
jnc_Type*
jnc_getMulticastTargetType (jnc_Multicast* multicast);

JNC_EXTERN_C
jnc_ClassType*
jnc_getMulticastSnapshotType (jnc_Multicast* multicast);

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Function*
jnc_getMcSnapshotCallMethod (jnc_McSnapshot* mcSnapshot);

inline
void*
jnc_getMcSnapshotCallMethodMachineCode (jnc_McSnapshot* mcSnapshot)
{
	jnc_Function* callMethod = jnc_getMcSnapshotCallMethod (mcSnapshot);
	return callMethod ? jnc_Function_getMachineCode (callMethod) : NULL;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

JNC_EXTERN_C
bool
isMulticastWeak (jnc_Multicast* multicast)
{
	return jnc_isMulticastWeak (multicast) != 0;
}

inline
Type*
getMulticastTargetType (jnc_Multicast* multicast)
{
	return jnc_getMulticastTargetType (multicast);
}

JNC_EXTERN_C
ClassType*
getMulticastSnapshotType (jnc_Multicast* multicast)
{
	return jnc_getMulticastSnapshotType (multicast);
}

inline
Function*
getMulticastCallMethod (Multicast* multicast)
{
	return jnc_getMulticastCallMethod (multicast);
}

inline
void*
getMulticastCallMethodMachineCode (Multicast* multicast)
{
	return jnc_getMulticastCallMethodMachineCode (multicast);
}

inline
jnc_Function*
getMcSnapshotCallMethod (jnc_McSnapshot* mcSnapshot)
{
	return jnc_getMcSnapshotCallMethod (mcSnapshot);
}

inline
void*
getMcSnapshotCallMethodMachineCode (jnc_McSnapshot* mcSnapshot)
{
	return jnc_getMcSnapshotCallMethodMachineCode (mcSnapshot);
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
