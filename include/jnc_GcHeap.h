#pragma once

#define _JNC_GCHEAP_H

#include "jnc_RuntimeStructs.h"

//.............................................................................

JNC_EXTERN_C
void
jnc_GcHeap_enterNoCollectRegion (jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_leaveNoCollectRegion (
	jnc_GcHeap* gcHeap,
	int canCollectNow
	);

JNC_EXTERN_C
void
jnc_GcHeap_enterWaitRegion (jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_leaveWaitRegion (jnc_GcHeap* gcHeap);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_allocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateData (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateArray (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateBuffer (
	jnc_GcHeap* gcHeap,
	size_t size
	);

JNC_EXTERN_C
jnc_DataPtrValidator*
jnc_GcHeap_createDataPtrValidator (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	);

JNC_EXTERN_C
void
jnc_GcHeap_gcWeakMark (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkData (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkClass (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

#if (_AXL_ENV == AXL_ENV_WIN)
JNC_EXTERN_C
int 
jnc_GcHeap_handleGcSehException (
	jnc_GcHeap* gcHeap,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif // _AXL_ENV

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_GcHeap
{
	void
	enterNoCollectRegion ()
	{
		jnc_GcHeap_enterNoCollectRegion (this);
	}

	void
	leaveNoCollectRegion (bool canCollectNow)
	{
		jnc_GcHeap_leaveNoCollectRegion (this, canCollectNow);
	}

	void
	enterWaitRegion ()
	{
		jnc_GcHeap_enterWaitRegion (this);
	}

	void
	leaveWaitRegion ()
	{
		jnc_GcHeap_leaveWaitRegion (this);
	}

	jnc_IfaceHdr*
	allocateClass (jnc_ClassType* type)
	{
		return jnc_GcHeap_allocateClass (this, type);
	}

	jnc_DataPtr
	allocateData (jnc_Type* type)
	{
		return jnc_GcHeap_allocateData (this, type);
	}

	jnc_DataPtr
	allocateArray (
		jnc_Type* type,
		size_t count
		)
	{
		return jnc_GcHeap_allocateArray (this, type, count);
	}

	jnc_DataPtr
	allocateBuffer (size_t size)
	{
		return jnc_GcHeap_allocateBuffer (this, size);
	}

	jnc_DataPtrValidator*
	createDataPtrValidator (
		jnc_Box* box,
		void* rangeBegin,
		size_t rangeLength
		)
	{
		return jnc_GcHeap_createDataPtrValidator (this, box, rangeBegin, rangeLength);
	}

	void
	gcWeakMark (jnc_Box* box)
	{
		jnc_GcHeap_gcWeakMark (this, box);
	}

	void
	gcMarkData (jnc_Box* box)
	{
		jnc_GcHeap_gcMarkData (this, box);
	}

	void
	gcMarkClass (
		jnc_Box* box
		)
	{
		jnc_GcHeap_gcMarkClass (this, box);
	}

#	if (_AXL_ENV == AXL_ENV_WIN)
	int 
	handleGcSehException (
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		)
	{
		return jnc_GcHeap_handleGcSehException (this, code, exceptionPointers);
	}
#	endif // _AXL_ENV
};
#endif // _JNC_CORE

//.............................................................................
