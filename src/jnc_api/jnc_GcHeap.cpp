#include "pch.h"
#include "jnc_GcHeap.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_rt_GcHeap.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
void
jnc_GcHeap_enterNoCollectRegion (jnc_GcHeap* gcHeap)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_enterNoCollectRegionFunc (gcHeap);
}

JNC_EXTERN_C
void
jnc_GcHeap_leaveNoCollectRegion (
	jnc_GcHeap* gcHeap,
	int canCollectNow
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_leaveNoCollectRegionFunc (gcHeap, canCollectNow);
}

JNC_EXTERN_C
void
jnc_GcHeap_enterWaitRegion (jnc_GcHeap* gcHeap)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_enterWaitRegionFunc (gcHeap);
}

JNC_EXTERN_C
void
jnc_GcHeap_leaveWaitRegion (jnc_GcHeap* gcHeap)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_leaveWaitRegionFunc (gcHeap);
}

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_allocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_allocateClassFunc (gcHeap, type);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateData (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_allocateDataFunc (gcHeap, type);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateArray (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_allocateArrayFunc (gcHeap, type, count);
}

JNC_EXTERN_C
jnc_DataPtrValidator*
jnc_GcHeap_createDataPtrValidator (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_createDataPtrValidatorFunc (gcHeap, box, rangeBegin, rangeLength);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcWeakMark (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_gcWeakMarkFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkData (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_gcMarkDataFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkClass (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_gcMarkClassFunc (gcHeap, box);
}

#	if (_AXL_ENV == AXL_ENV_WIN)
JNC_EXTERN_C
int 
jnc_GcHeap_handleGcSehException (
	jnc_GcHeap* gcHeap,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_handleGcSehExceptionFunc (gcHeap, code, exceptionPointers);
}
#	endif // _AXL_ENV
#else     // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
void
jnc_GcHeap_enterNoCollectRegion (jnc_GcHeap* gcHeap)
{
	gcHeap->enterNoCollectRegion ();
}

JNC_EXTERN_C
void
jnc_GcHeap_leaveNoCollectRegion (
	jnc_GcHeap* gcHeap,
	int canCollectNow
	)
{
	gcHeap->leaveNoCollectRegion (canCollectNow != 0);
}

JNC_EXTERN_C
void
jnc_GcHeap_enterWaitRegion (jnc_GcHeap* gcHeap)
{
	gcHeap->enterWaitRegion ();
}

JNC_EXTERN_C
void
jnc_GcHeap_leaveWaitRegion (jnc_GcHeap* gcHeap)
{
	gcHeap->leaveWaitRegion ();
}

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_allocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	)
{
	return gcHeap->allocateClass (type);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateData (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	)
{
	return gcHeap->allocateData (type);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateArray (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	)
{
	return gcHeap->allocateArray (type, count);
}

JNC_EXTERN_C
jnc_DataPtrValidator*
jnc_GcHeap_createDataPtrValidator (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	return gcHeap->createDataPtrValidator (box, rangeBegin, rangeLength);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcWeakMark (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	gcHeap->weakMark (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkData (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	gcHeap->markData (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_gcMarkClass (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	)
{
	gcHeap->markClass (box);
}

#if (_AXL_ENV == AXL_ENV_WIN)
JNC_EXTERN_C
int 
jnc_GcHeap_handleGcSehException (
	jnc_GcHeap* gcHeap,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	)
{
	return gcHeap->handleSehException (code, exceptionPointers);
}
#	endif // _AXL_ENV
#endif    // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
