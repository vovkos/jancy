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

#pragma once

#define _JNC_GCHEAP_H

#include "jnc_RuntimeStructs.h"

/**

\defgroup gc-heap Garbage-Collected Heap
	\ingroup runtime-subsystem
	\import{jnc_GcHeap.h}

	\brief This set of functions is used to allocate objects on GC Heap

\addtogroup gc-heap
@{

\struct jnc_GcHeap
	\verbatim

	Opaque structure used as a handle to Jancy garbage-collected heap.

	Use functions from the `Garbage-Collected Heap` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_GcDef
{
	jnc_GcDef_AllocSizeTrigger  = -1, // use period only
#ifdef _JNC_DEBUG
	jnc_GcDef_PeriodSizeTrigger = 0, // run gc on every allocation
#elif (JNC_PTR_SIZE == 4)
	jnc_GcDef_PeriodSizeTrigger = 1 * 1024 * 1024, // 1MB gc period
#else
	jnc_GcDef_PeriodSizeTrigger = 2 * 1024 * 1024, // 2MB gc period
#endif

#ifdef _JNC_DEBUG
	jnc_GcDef_DataPtrValidatorPoolSize = 1, // don't use pool, allocate every time
	jnc_GcDef_ForeignDataBoxPoolSize   = 1, // don't use pool, allocate every time
#else
	jnc_GcDef_DataPtrValidatorPoolSize = 32,
	jnc_GcDef_ForeignDataBoxPoolSize   = 16,
#endif

	jnc_GcDef_ShutdownIterationLimit   = 3,
};

typedef enum jnc_GcDef jnc_GcDef;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum jnc_GcShadowStackFrameMapOp
{
	jnc_GcShadowStackFrameMapOp_Open,
	jnc_GcShadowStackFrameMapOp_Close,
	jnc_GcShadowStackFrameMapOp_Restore,
};

typedef enum jnc_GcShadowStackFrameMapOp jnc_GcShadowStackFrameMapOp;

//..............................................................................

struct jnc_GcStats
{
	size_t m_currentAllocSize;
	size_t m_totalAllocSize;
	size_t m_peakAllocSize;
	size_t m_currentPeriodSize;
	size_t m_totalCollectCount;
	size_t m_lastCollectFreeSize;
	uint64_t m_lastCollectTime;
	uint64_t m_lastCollectTimeTaken;
	uint64_t m_totalCollectTimeTaken;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcSizeTriggers
{
	size_t m_allocSizeTrigger;
	size_t m_periodSizeTrigger;
};

//..............................................................................

JNC_EXTERN_C
jnc_Runtime*
jnc_GcHeap_getRuntime(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_getStats(
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

JNC_EXTERN_C
void
jnc_GcHeap_getSizeTriggers(
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	);

JNC_EXTERN_C
void
jnc_GcHeap_setSizeTriggers(
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	);

JNC_EXTERN_C
void
jnc_GcHeap_getStats(
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

JNC_EXTERN_C
void
jnc_GcHeap_collect(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_enterNoCollectRegion(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_leaveNoCollectRegion(
	jnc_GcHeap* gcHeap,
	bool_t canCollectNow
	);

JNC_EXTERN_C
void
jnc_GcHeap_enterWaitRegion(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_leaveWaitRegion(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_safePoint(jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_setFrameMap(
	jnc_GcHeap* gcHeap,
	jnc_GcShadowStackFrame* frame,
	jnc_GcShadowStackFrameMap* map,
	jnc_GcShadowStackFrameMapOp op
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticDestructor(
	jnc_GcHeap* gcHeap,
	jnc_StaticDestructFunc* destructFunc
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticClassDestructor(
	jnc_GcHeap* gcHeap,
	jnc_DestructFunc* destructFunc,
	jnc_IfaceHdr* iface
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticRoot(
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_allocateClass(
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_tryAllocateClass(
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateData(
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_tryAllocateData(
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateArray(
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_tryAllocateArray(
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_allocateBuffer(
	jnc_GcHeap* gcHeap,
	size_t size
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_tryAllocateBuffer(
	jnc_GcHeap* gcHeap,
	size_t size
	);

JNC_EXTERN_C
jnc_DataPtrValidator*
jnc_GcHeap_createDataPtrValidator(
	jnc_GcHeap* gcHeap,
	jnc_Box* box,
	const void* rangeBegin,
	size_t rangeLength
	);

JNC_EXTERN_C
jnc_DetachedDataBox*
jnc_GcHeap_createForeignDataBox(
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t elementCount, // -1 if not array
	const void* p,
	bool_t isCallSiteLocal
	);

JNC_EXTERN_C
jnc_DataPtr
jnc_GcHeap_createForeignBufferPtr(
	jnc_GcHeap* gcHeap,
	const void* p,
	size_t size,
	bool_t isCallSiteLocal
	);

JNC_EXTERN_C
void
jnc_GcHeap_invalidateDataPtrValidator(
	jnc_GcHeap* gcHeap,
	jnc_DataPtrValidator* validator
	);

JNC_INLINE
void
jnc_GcHeap_invalidateDataPtr(
	jnc_GcHeap* gcHeap,
	jnc_DataPtr ptr
	)
{
	jnc_GcHeap_invalidateDataPtrValidator(gcHeap, ptr.m_validator);
}

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_getDynamicLayout(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_resetDynamicLayout(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_weakMark(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_markData(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_INLINE
void
jnc_GcHeap_markDataPtr(
	jnc_GcHeap* gcHeap,
	jnc_DataPtr ptr
	);

JNC_EXTERN_C
void
jnc_GcHeap_markClass(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_INLINE
void
jnc_GcHeap_markClassPtr(
	jnc_GcHeap* gcHeap,
	jnc_IfaceHdr* iface
	);

JNC_EXTERN_C
void
jnc_GcHeap_addRoot(
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	);

JNC_EXTERN_C
void
jnc_GcHeap_addBoxToCallSite(jnc_Box* box);

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_GcHeap
{
	jnc_Runtime*
	getRuntime()
	{
		return jnc_GcHeap_getRuntime(this);
	}

	void
	getSizeTriggers(jnc_GcSizeTriggers* triggers)
	{
		jnc_GcHeap_getSizeTriggers(this, triggers);
	}

	void
	setSizeTriggers(const jnc_GcSizeTriggers* triggers)
	{
		jnc_GcHeap_setSizeTriggers(this, triggers);
	}

	void
	getStats(jnc_GcStats* stats)
	{
		jnc_GcHeap_getStats(this, stats);
	}

	void
	collect()
	{
		jnc_GcHeap_collect(this);
	}

	void
	enterNoCollectRegion()
	{
		jnc_GcHeap_enterNoCollectRegion(this);
	}

	void
	leaveNoCollectRegion(bool canCollectNow = true)
	{
		jnc_GcHeap_leaveNoCollectRegion(this, canCollectNow);
	}

	void
	enterWaitRegion()
	{
		jnc_GcHeap_enterWaitRegion(this);
	}

	void
	leaveWaitRegion()
	{
		jnc_GcHeap_leaveWaitRegion(this);
	}

	void
	safePoint()
	{
		jnc_GcHeap_safePoint(this);
	}

	void
	setFrameMap(
		jnc_GcShadowStackFrame* frame,
		jnc_GcShadowStackFrameMap* map,
		jnc_GcShadowStackFrameMapOp op
		)
	{
		jnc_GcHeap_setFrameMap(this, frame, map, op);
	}

	void
	addStaticDestructor(jnc_StaticDestructFunc* destructFunc)
	{
		jnc_GcHeap_addStaticDestructor(this, destructFunc);
	}

	void
	addStaticClassDestructor(
		jnc_DestructFunc* destructFunc,
		jnc_IfaceHdr* iface
		)
	{
		jnc_GcHeap_addStaticClassDestructor(this, destructFunc, iface);
	}

	void
	addStaticRoot(
		const void* p,
		jnc_Type* type
		)
	{
		jnc_GcHeap_addStaticRoot(this, p, type);
	}

	jnc_IfaceHdr*
	allocateClass(jnc_ClassType* type)
	{
		return jnc_GcHeap_allocateClass(this, type);
	}

	jnc_IfaceHdr*
	tryAllocateClass(jnc_ClassType* type)
	{
		return jnc_GcHeap_tryAllocateClass(this, type);
	}

	jnc_DataPtr
	allocateData(jnc_Type* type)
	{
		return jnc_GcHeap_allocateData(this, type);
	}

	jnc_DataPtr
	tryAllocateData(jnc_Type* type)
	{
		return jnc_GcHeap_tryAllocateData(this, type);
	}

	jnc_DataPtr
	allocateArray(
		jnc_Type* type,
		size_t count
		)
	{
		return jnc_GcHeap_allocateArray(this, type, count);
	}

	jnc_DataPtr
	tryAllocateArray(
		jnc_Type* type,
		size_t count
		)
	{
		return jnc_GcHeap_tryAllocateArray(this, type, count);
	}

	jnc_DataPtr
	allocateBuffer(size_t size)
	{
		return jnc_GcHeap_allocateBuffer(this, size);
	}

	jnc_DataPtr
	tryAllocateBuffer(size_t size)
	{
		return jnc_GcHeap_tryAllocateBuffer(this, size);
	}

	jnc_DataPtrValidator*
	createDataPtrValidator(
		jnc_Box* box,
		const void* rangeBegin,
		size_t rangeLength
		)
	{
		return jnc_GcHeap_createDataPtrValidator(this, box, rangeBegin, rangeLength);
	}

	jnc_DetachedDataBox*
	createForeignDataBox(
		jnc_Type* type,
		size_t elementCount,
		const void* p,
		bool isCallSiteLocal = true
		)
	{
		return jnc_GcHeap_createForeignDataBox(this, type, elementCount, p, isCallSiteLocal);
	}

	jnc_DetachedDataBox*
	createForeignDataBox(
		jnc_Type* type,
		const void* p,
		bool isCallSiteLocal = true
		)
	{
		return jnc_GcHeap_createForeignDataBox(this, type, -1, p, isCallSiteLocal);
	}

	jnc_DataPtr
	createForeignBufferPtr(
		const void* p,
		size_t size,
		bool isCallSiteLocal = true
		)
	{
		return jnc_GcHeap_createForeignBufferPtr(this, p, size, isCallSiteLocal);
	}

	void
	invalidateDataPtrValidator(jnc_DataPtrValidator* validator)
	{
		jnc_GcHeap_invalidateDataPtrValidator(this, validator);
	}

	void
	invalidateDataPtr(jnc_DataPtr ptr)
	{
		jnc_GcHeap_invalidateDataPtr(this, ptr);
	}

	jnc_IfaceHdr*
	getDynamicLayout(jnc_Box* box)
	{
		return jnc_GcHeap_getDynamicLayout(this, box);
	}

	void
	resetDynamicLayout(jnc_Box* box)
	{
		jnc_GcHeap_resetDynamicLayout(this, box);
	}

	void
	weakMark(jnc_Box* box)
	{
		jnc_GcHeap_weakMark(this, box);
	}

	void
	markData(jnc_Box* box)
	{
		jnc_GcHeap_markData(this, box);
	}

	void
	markDataPtr(jnc_DataPtr ptr)
	{
		jnc_GcHeap_markDataPtr(this, ptr);
	}

	void
	markClass(jnc_Box* box)
	{
		jnc_GcHeap_markClass(this, box);
	}

	void
	markClassPtr(jnc_IfaceHdr* iface)
	{
		jnc_GcHeap_markClassPtr(this, iface);
	}

	void
	addRoot(
		const void* p,
		jnc_Type* type
		)
	{
		jnc_GcHeap_addRoot(this, p, type);
	}

	static
	void
	addBoxToCallSite(jnc_Box* box)
	{
		jnc_GcHeap_addBoxToCallSite(box);
	}
};
#endif // _JNC_CORE

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
void
jnc_GcHeap_markDataPtr(
	jnc_GcHeap* gcHeap,
	jnc_DataPtr ptr
	)
{
	if (ptr.m_validator)
	{
		jnc_GcHeap_weakMark(gcHeap, ptr.m_validator->m_validatorBox);
		jnc_GcHeap_markData(gcHeap, ptr.m_validator->m_targetBox);
	}
}

JNC_INLINE
void
jnc_GcHeap_markClassPtr(
	jnc_GcHeap* gcHeap,
	jnc_IfaceHdr* iface
	)
{
	if (iface)
		jnc_GcHeap_markClass(gcHeap, iface->m_box);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_GcDef GcDef;

const GcDef
	GcDef_AllocSizeTrigger         = jnc_GcDef_AllocSizeTrigger,
	GcDef_PeriodSizeTrigger        = jnc_GcDef_PeriodSizeTrigger,
	GcDef_DataPtrValidatorPoolSize = jnc_GcDef_DataPtrValidatorPoolSize,
	GcDef_ForeignDataBoxPoolSize   = jnc_GcDef_ForeignDataBoxPoolSize,
	GcDef_ShutdownIterationLimit   = jnc_GcDef_ShutdownIterationLimit;

typedef jnc_GcShadowStackFrameMapOp GcShadowStackFrameMapOp;

const GcShadowStackFrameMapOp
	GcShadowStackFrameMapOp_Open    = jnc_GcShadowStackFrameMapOp_Open,
	GcShadowStackFrameMapOp_Close   = jnc_GcShadowStackFrameMapOp_Close,
	GcShadowStackFrameMapOp_Restore = jnc_GcShadowStackFrameMapOp_Restore;

typedef jnc_GcStats GcStats;
typedef jnc_GcSizeTriggers GcSizeTriggers;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
