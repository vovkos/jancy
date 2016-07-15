#pragma once

#include "jnc_RuntimeStructs.h"
#include "jnc_GcHeap.h"

//.............................................................................

JNC_EXTERN_C
jnc_Module*
jnc_Runtime_getModule (jnc_Runtime* runtime);

JNC_EXTERN_C
jnc_GcHeap*
jnc_Runtime_getGcHeap (jnc_Runtime* runtime);

JNC_EXTERN_C
void
jnc_Runtime_initializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	);

JNC_EXTERN_C
void
jnc_Runtime_uninitializeThread (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	);

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Runtime
{
	jnc_Module*
	getModule ()
	{
		return jnc_Runtime_getModule (this);
	}

	jnc_GcHeap*
	getGcHeap ()
	{
		return jnc_Runtime_getGcHeap (this);
	}

	void
	initializeThread (jnc_ExceptionRecoverySnapshot* ers)
	{
		jnc_Runtime_initializeThread (this, ers);
	}

	void
	uninitializeThread (jnc_ExceptionRecoverySnapshot* ers)
	{
		jnc_Runtime_uninitializeThread (this, ers);
	}
};
#endif // _JNC_CORE

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Runtime*
jnc_getCurrentThreadRuntime ();

inline
jnc_GcHeap*
jnc_getCurrentThreadGcHeap ()
{
	jnc_Runtime* runtime = jnc_getCurrentThreadRuntime ();
	return runtime ? jnc_Runtime_getGcHeap (runtime) : NULL;
}

JNC_EXTERN_C
void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	);

JNC_EXTERN_C
size_t 
jnc_strLen (jnc_DataPtr ptr);

JNC_EXTERN_C
jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length = -1
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_memDup (
	const void* p,
	size_t size
	);

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

inline
Runtime*
getCurrentThreadRuntime ()
{
	return jnc_getCurrentThreadRuntime ();
}

inline
GcHeap*
getCurrentThreadGcHeap ()
{
	return jnc_getCurrentThreadGcHeap ();
}

inline 
void
primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	jnc_primeClass (box, root, type, vtable);
}

inline
size_t 
strLen (DataPtr ptr)
{
	return jnc_strLen (ptr);
}

inline
DataPtr
strDup (
	const char* p,
	size_t length = -1
	)
{
	return jnc_strDup (p, length);
}

inline
DataPtr
memDup (
	const void* p,
	size_t size
	)
{
	return jnc_memDup (p, size);
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
