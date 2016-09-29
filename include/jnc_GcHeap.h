#pragma once

#define _JNC_GCHEAP_H

#include "jnc_RuntimeStructs.h"

/// \addtogroup gc-heap
/// @{

//.............................................................................

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
#else
	jnc_GcDef_DataPtrValidatorPoolSize = 32,
#endif

	jnc_GcDef_ShutdownIterationLimit   = 3,
};

typedef enum jnc_GcDef jnc_GcDef;

//.............................................................................

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcSizeTriggers
{
	size_t m_allocSizeTrigger;
	size_t m_periodSizeTrigger;
};

//.............................................................................

JNC_EXTERN_C
jnc_Runtime*
jnc_GcHeap_getRuntime (jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_getStats (
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

JNC_EXTERN_C
void
jnc_GcHeap_getSizeTriggers (
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	);

JNC_EXTERN_C
void
jnc_GcHeap_setSizeTriggers (
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	);

JNC_EXTERN_C
void
jnc_GcHeap_getStats (
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

JNC_EXTERN_C
void
jnc_GcHeap_collect (jnc_GcHeap* gcHeap);

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
void
jnc_GcHeap_safePoint (jnc_GcHeap* gcHeap);

JNC_EXTERN_C
void
jnc_GcHeap_setFrameMap (
	jnc_GcHeap* gcHeap,
	jnc_GcShadowStackFrame* frame,
	jnc_GcShadowStackFrameMap* map,
	int isOpen
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticDestructor (
	jnc_GcHeap* gcHeap,
	jnc_StaticDestructFunc* destructFunc
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticClassDestructor (
	jnc_GcHeap* gcHeap,
	jnc_DestructFunc* destructFunc,
	jnc_IfaceHdr* iface
	);

JNC_EXTERN_C
void
jnc_GcHeap_addStaticRoot (
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_allocateClass (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

JNC_EXTERN_C
jnc_IfaceHdr*
jnc_GcHeap_tryAllocateClass (
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
jnc_GcHeap_tryAllocateData (
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
jnc_GcHeap_tryAllocateArray (
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
jnc_DataPtr
jnc_GcHeap_tryAllocateBuffer (
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
jnc_GcHeap_weakMark (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_markData (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_markClass (
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

JNC_EXTERN_C
void
jnc_GcHeap_addRoot (
	jnc_GcHeap* gcHeap,
	const void* p,
	jnc_Type* type
	);

#if (_JNC_OS_WIN)
JNC_EXTERN_C
int
jnc_GcHeap_handleGcSehException (
	jnc_GcHeap* gcHeap,
	uint_t code,
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_GcHeap
{
	jnc_Runtime*
	getRuntime ()
	{
		return jnc_GcHeap_getRuntime (this);
	}

	void
	getSizeTriggers (jnc_GcSizeTriggers* triggers)
	{
		jnc_GcHeap_getSizeTriggers (this, triggers);
	}

	void
	setSizeTriggers (const jnc_GcSizeTriggers* triggers)
	{
		jnc_GcHeap_setSizeTriggers (this, triggers);
	}

	void
	getStats (jnc_GcStats* stats)
	{
		jnc_GcHeap_getStats (this, stats);
	}

	void
	collect ()
	{
		jnc_GcHeap_collect (this);
	}

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

	void
	safePoint ()
	{
		jnc_GcHeap_safePoint (this);
	}

	void
	setFrameMap (
		jnc_GcShadowStackFrame* frame,
		jnc_GcShadowStackFrameMap* map,
		bool isOpen
		)
	{
		jnc_GcHeap_setFrameMap (this, frame, map, isOpen);
	}

	void
	addStaticDestructor (jnc_StaticDestructFunc* destructFunc)
	{
		jnc_GcHeap_addStaticDestructor (this, destructFunc);
	}

	void
	addStaticClassDestructor (
		jnc_DestructFunc* destructFunc,
		jnc_IfaceHdr* iface
		)
	{
		jnc_GcHeap_addStaticClassDestructor (this, destructFunc, iface);
	}

	void
	addStaticRoot (
		const void* p,
		jnc_Type* type
		)
	{
		jnc_GcHeap_addStaticRoot (this, p, type);
	}

	jnc_IfaceHdr*
	allocateClass (jnc_ClassType* type)
	{
		return jnc_GcHeap_allocateClass (this, type);
	}

	jnc_IfaceHdr*
	tryAllocateClass (jnc_ClassType* type)
	{
		return jnc_GcHeap_tryAllocateClass (this, type);
	}

	jnc_DataPtr
	allocateData (jnc_Type* type)
	{
		return jnc_GcHeap_allocateData (this, type);
	}

	jnc_DataPtr
	tryAllocateData (jnc_Type* type)
	{
		return jnc_GcHeap_tryAllocateData (this, type);
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
	tryAllocateArray (
		jnc_Type* type,
		size_t count
		)
	{
		return jnc_GcHeap_tryAllocateArray (this, type, count);
	}

	jnc_DataPtr
	allocateBuffer (size_t size)
	{
		return jnc_GcHeap_allocateBuffer (this, size);
	}

	jnc_DataPtr
	tryAllocateBuffer (size_t size)
	{
		return jnc_GcHeap_tryAllocateBuffer (this, size);
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
	weakMark (jnc_Box* box)
	{
		jnc_GcHeap_weakMark (this, box);
	}

	void
	markData (jnc_Box* box)
	{
		jnc_GcHeap_markData (this, box);
	}

	void
	markClass (jnc_Box* box)
	{
		jnc_GcHeap_markClass (this, box);
	}

	void
	addRoot (
		const void* p,
		jnc_Type* type
		)
	{
		jnc_GcHeap_addRoot (this, p, type);
	}

#	if (_JNC_OS_WIN)
	int
	handleGcSehException (
		uint_t code,
		EXCEPTION_POINTERS* exceptionPointers
		)
	{
		return jnc_GcHeap_handleGcSehException (this, code, exceptionPointers);
	}
#	endif // _JNC_OS_WIN
};
#endif // _JNC_CORE

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_GcDef GcDef;

const GcDef
	GcDef_AllocSizeTrigger         = jnc_GcDef_AllocSizeTrigger,
	GcDef_PeriodSizeTrigger        = jnc_GcDef_PeriodSizeTrigger,
	GcDef_DataPtrValidatorPoolSize = jnc_GcDef_DataPtrValidatorPoolSize,
	GcDef_ShutdownIterationLimit   = jnc_GcDef_ShutdownIterationLimit;

typedef jnc_GcStats GcStats;
typedef jnc_GcSizeTriggers GcSizeTriggers;

//.............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
