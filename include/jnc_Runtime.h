#pragma once

#include "jnc_RuntimeStructs.h"

typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_ClassType jnc_ClassType;

//.............................................................................

jnc_ModuleItem*
jnc_Runtime_findModuleItem (
	jnc_Runtime* self,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	);

void
jnc_Runtime_initializeThread (
	jnc_Runtime* self,
	jnc_ExceptionRecoverySnapshot* ers
	);

void
jnc_Runtime_uninitializeThread (
	jnc_Runtime* self,
	jnc_ExceptionRecoverySnapshot* ers
	);

void
jnc_Runtime_enterNoCollectRegion (jnc_Runtime* self);

void
jnc_Runtime_leaveNoCollectRegion (
	jnc_Runtime* self,
	int canCollectNow
	);

void
jnc_Runtime_enterWaitRegion (jnc_Runtime* self);

void
jnc_Runtime_leaveWaitRegion (jnc_Runtime* self);

jnc_IfaceHdr*
jnc_Runtime_allocateClass (
	jnc_Runtime* self,
	jnc_ClassType* type
	);

jnc_DataPtr
jnc_Runtime_allocateData (
	jnc_Runtime* self,
	jnc_Type* type
	);

jnc_DataPtr
jnc_Runtime_allocateArray (
	jnc_Runtime* self,
	jnc_Type* type,
	size_t count
	);

jnc_DataPtrValidator*
jnc_Runtime_createDataPtrValidator (
	jnc_Runtime* self,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	);

void
jnc_Runtime_gcWeakMark (
	jnc_Runtime* self,	
	jnc_Box* box
	);

void
jnc_Runtime_gcMarkData (
	jnc_Runtime* self,	
	jnc_Box* box
	);

void
jnc_Runtime_gcMarkClass (
	jnc_Runtime* self,	
	jnc_Box* box
	);

#if (_AXL_ENV == AXL_ENV_WIN)
int 
jnc_Runtime_handleGcSehException (
	jnc_Runtime* self,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif // _AXL_ENV

struct jnc_Runtime
{
#ifdef __cplusplus
	jnc_ModuleItem*
	findModuleItem (
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		)
	{
		return jnc_Runtime_findModuleItem (this, name, libCacheSlot, itemCacheSlot);
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

	void
	enterNoCollectRegion ()
	{
		jnc_Runtime_enterNoCollectRegion (this);
	}

	void
	leaveNoCollectRegion (bool canCollectNow)
	{
		jnc_Runtime_leaveNoCollectRegion (this, canCollectNow);
	}

	void
	enterWaitRegion ()
	{
		jnc_Runtime_enterWaitRegion (this);
	}

	void
	leaveWaitRegion ()
	{
		jnc_Runtime_leaveWaitRegion (this);
	}

	jnc_IfaceHdr*
	allocateClass (jnc_ClassType* type)
	{
		return jnc_Runtime_allocateClass (this, type);
	}

	jnc_DataPtr
	allocateData (jnc_Type* type)
	{
		return jnc_Runtime_allocateData (this, type);
	}

	jnc_DataPtr
	allocateArray (
		jnc_Type* type,
		size_t count
		)
	{
		return jnc_Runtime_allocateArray (this, type, count);
	}

	jnc_DataPtrValidator*
	createDataPtrValidator (
		jnc_Box* box,
		void* rangeBegin,
		size_t rangeLength
		)
	{
		return jnc_Runtime_createDataPtrValidator (this, box, rangeBegin, rangeLength);
	}

	void
	gcWeakMark (jnc_Box* box)
	{
		jnc_Runtime_gcWeakMark (this, box);
	}

	void
	gcMarkData (jnc_Box* box)
	{
		jnc_Runtime_gcMarkData (this, box);
	}

	void
	gcMarkClass (
		jnc_Box* box
		)
	{
		jnc_Runtime_gcMarkClass (this, box);
	}

#	if (_AXL_ENV == AXL_ENV_WIN)
	int 
	handleGcSehException (
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		)
	{
		return jnc_Runtime_handleGcSehException (this, code, exceptionPointers);
	}
#	endif // _AXL_ENV
#endif // __cplusplus
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_Runtime*
jnc_getCurrentThreadRuntime ();

size_t 
jnc_strLen (jnc_DataPtr ptr);

jnc_DataPtr
jnc_strDup (
	const char* p,
	size_t length = -1
	);

jnc_DataPtr
jnc_memDup (
	const void* p,
	size_t size
	);

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_Runtime Runtime;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Runtime*
getCurrentThreadRuntime ()
{
	return jnc_getCurrentThreadRuntime ();
}

size_t 
strLen (DataPtr ptr)
{
	return jnc_strLen (ptr);
}

DataPtr
strDup (
	const char* p,
	size_t length = -1
	)
{
	return jnc_strDup (p, length);
}

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
