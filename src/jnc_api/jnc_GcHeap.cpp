//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_GcHeap.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_rt_ExceptionMgr.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Runtime*
jnc_GcHeap_getRuntime (jnc_GcHeap* gcHeap)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_getRuntimeFunc (gcHeap);
}

JNC_EXTERN_C
void
jnc_GcHeap_getStats (
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	)
{
	jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_getStatsFunc (gcHeap, stats);
}

JNC_EXTERN_C
void
jnc_GcHeap_getSizeTriggers (
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	)
{
	jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_getSizeTriggersFunc (gcHeap, triggers);
}

JNC_EXTERN_C
void
jnc_GcHeap_setSizeTriggers (
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	)
{
	jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_setSizeTriggersFunc (gcHeap, triggers);
}

JNC_EXTERN_C
void
jnc_GcHeap_collect (jnc_GcHeap* gcHeap)
{
	jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_collectFunc (gcHeap);
}

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
jnc_IfaceHdr*
jnc_GcHeap_tryAllocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_tryAllocateClassFunc (gcHeap, type);
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
jnc_GcHeap_tryAllocateData (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_tryAllocateDataFunc (gcHeap, type);
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
jnc_DataPtr
jnc_GcHeap_tryAllocateArray (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_tryAllocateArrayFunc (gcHeap, type, count);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateBuffer (
	jnc_GcHeap* gcHeap,
	size_t size
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_allocateBufferFunc (gcHeap, size);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_tryAllocateBuffer (
	jnc_GcHeap* gcHeap,
	size_t size
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_tryAllocateBufferFunc (gcHeap, size);
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
jnc_IfaceHdr*
jnc_GcHeap_getDynamicLayout (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_getDynamicLayoutFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_resetDynamicLayout (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_resetDynamicLayoutFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_weakMark (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_weakMarkFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_markData (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_markDataFunc (gcHeap, box);
}

JNC_EXTERN_C
void
jnc_GcHeap_markClass (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_markClassFunc (gcHeap, box);
}

void
jnc_GcHeap_addBoxToCallSite (jnc_Box* box)
{
	jnc_g_dynamicExtensionLibHost->m_gcHeapFuncTable->m_addBoxToCallSiteFunc (box);
}

#else     // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Runtime*
jnc_GcHeap_getRuntime (jnc_GcHeap* gcHeap)
{
	return gcHeap->getRuntime ();
}

JNC_EXTERN_C
void
jnc_GcHeap_getStats (
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	)
{
	gcHeap->getStats (stats);
}

JNC_EXTERN_C
void
jnc_GcHeap_getSizeTriggers (
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	)
{
	gcHeap->getSizeTriggers (triggers);
}

JNC_EXTERN_C
void
jnc_GcHeap_setSizeTriggers (
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	)
{
	gcHeap->setSizeTriggers (*triggers);
}

JNC_EXTERN_C
void
jnc_GcHeap_collect (jnc_GcHeap* gcHeap)
{
	gcHeap->collect ();
}

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
void
jnc_GcHeap_safePoint (jnc_GcHeap* gcHeap)
{
	gcHeap->safePoint ();
}

JNC_EXTERN_C
void
jnc_GcHeap_setFrameMap (
	jnc_GcHeap* gcHeap,
	jnc_GcShadowStackFrame* frame,
	jnc_GcShadowStackFrameMap* map,
	int isOpen
	)
{
	gcHeap->setFrameMap (frame, map, isOpen != 0);
}

JNC_EXTERN_C
void
jnc_GcHeap_addStaticDestructor (
	jnc_GcHeap* gcHeap,
	jnc_StaticDestructFunc* destructFunc
	)
{
	gcHeap->addStaticDestructor (destructFunc);
}

JNC_EXTERN_C
void
jnc_GcHeap_addStaticClassDestructor (
	jnc_GcHeap* gcHeap,
	jnc_DestructFunc* destructFunc,
	jnc_IfaceHdr* iface
	)
{
	gcHeap->addStaticClassDestructor (destructFunc, iface);
}

JNC_EXTERN_C
void
jnc_GcHeap_addStaticRoot (
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	)
{
	gcHeap->addStaticRoot (p, type);
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
jnc_IfaceHdr*
jnc_GcHeap_tryAllocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	)
{
	return gcHeap->tryAllocateClass (type);
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
jnc_GcHeap_tryAllocateData (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	)
{
	return gcHeap->tryAllocateData (type);
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
jnc_DataPtr
jnc_GcHeap_tryAllocateArray (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	)
{
	return gcHeap->tryAllocateArray (type, count);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateBuffer (
	jnc_GcHeap* gcHeap,
	size_t size
	)
{
	return gcHeap->allocateBuffer (size);
}

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_tryAllocateBuffer (
	jnc_GcHeap* gcHeap,
	size_t size
	)
{
	return gcHeap->tryAllocateBuffer (size);
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
jnc_IfaceHdr*
jnc_GcHeap_getDynamicLayout (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return gcHeap->getDynamicLayout (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_resetDynamicLayout (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	return gcHeap->resetDynamicLayout (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_weakMark (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	gcHeap->weakMark (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_markData (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	gcHeap->markData (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_markClass (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	)
{
	gcHeap->markClass (box);
}

JNC_EXTERN_C
void
jnc_GcHeap_addRoot (
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	)
{
	gcHeap->addRoot (p, type);
}

void
jnc_GcHeap_addBoxToCallSite (jnc_Box* box)
{
	bool result = jnc::rt::GcHeap::addBoxIfDynamicFrame (box);
	ASSERT (result);
}

#endif    // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
