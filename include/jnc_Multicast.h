#pragma once

#define _JNC_MULTICAST_H

#include "jnc_Function.h"

typedef struct jnc_Multicast jnc_Multicast;
typedef struct jnc_McSnapshot jnc_McSnapshot;

//.............................................................................

JNC_EXTERN_C
int
jnc_Multicast_isWeak (jnc_Multicast* multicast);

JNC_EXTERN_C
jnc_Type*
jnc_Multicast_getTargetType (jnc_Multicast* multicast);

JNC_EXTERN_C
jnc_ClassType*
jnc_Multicast_getSnapshotType (jnc_Multicast* multicast);

JNC_EXTERN_C
jnc_Function*
jnc_Multicast_getCallMethod (jnc_Multicast* multicast);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_BEGIN_INHERITED_STRUCT (jnc_Multicast, jnc_IfaceHdr)
	volatile intptr_t m_lock;
	jnc_DataPtr m_ptr; // array of function closure, weak or unsafe pointers
	size_t m_count;
	size_t m_maxCount;
	void* m_handleTable;

#ifdef __cplusplus
	bool
	isWeak ()
	{
		return jnc_Multicast_isWeak (this) != 0;
	}

	jnc_Type*
	getTargetType ()
	{
		return jnc_Multicast_getTargetType (this);
	}

	jnc_ClassType*
	getSnapshotType ()
	{
		return jnc_Multicast_getSnapshotType (this);
	}

	jnc_Function*
	getCallMethod ()
	{
		return jnc_Multicast_getCallMethod (this);
	}
#endif // __cplusplus
JNC_END_INHERITED_STRUCT ()

//.............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshot_getCallMethod (jnc_McSnapshot* mcSnapshot);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_BEGIN_INHERITED_STRUCT (jnc_McSnapshot, jnc_IfaceHdr)
	jnc_DataPtr m_ptr; // array of function closure or unsafe pointers
	size_t m_count;

#ifdef __cplusplus
	jnc_Function*
	getCallMethod ()
	{
		return jnc_McSnapshot_getCallMethod (this);
	}
#endif // __cplusplus
JNC_END_INHERITED_STRUCT ()


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Multicast Multicast;
typedef jnc_McSnapshot McSnapshot;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
