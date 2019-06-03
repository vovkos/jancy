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

#define _JNC_RUNTIMESTRUCTS_H

#include "jnc_Def.h"

/**

\defgroup runtime-structs Runtime Structures
	\ingroup runtime-subsystem
	\import{jnc_RuntimeStructs.h}

\addtogroup runtime-structs
@{

*/

typedef struct jnc_Box jnc_Box;
typedef struct jnc_DataBox jnc_DataBox;
typedef struct jnc_DetachedDataBox jnc_DetachedDataBox;
typedef struct jnc_DataPtrValidator jnc_DataPtrValidator;
typedef struct jnc_DataPtr jnc_DataPtr;
typedef struct jnc_FunctionPtr jnc_FunctionPtr;
typedef struct jnc_PropertyPtr jnc_PropertyPtr;
typedef struct jnc_IfaceHdr jnc_IfaceHdr;
typedef struct jnc_Multicast jnc_Multicast;
typedef struct jnc_McSnapshot jnc_McSnapshot;
typedef struct jnc_Scheduler jnc_Scheduler;
typedef struct jnc_SchedulerVtable jnc_SchedulerVtable;
typedef struct jnc_Reactor jnc_Reactor;
typedef struct jnc_FmtLiteral jnc_FmtLiteral;
typedef struct jnc_GcShadowStackFrame jnc_GcShadowStackFrame;
typedef struct jnc_GcShadowStackFrameMapBuffer jnc_GcShadowStackFrameMapBuffer;
typedef struct jnc_GcMutatorThread jnc_GcMutatorThread;
typedef struct jnc_OpaqueClassTypeInfo jnc_OpaqueClassTypeInfo;
typedef struct jnc_SjljFrame jnc_SjljFrame;
typedef struct jnc_Tls jnc_Tls;
typedef struct jnc_TlsVariableTable jnc_TlsVariableTable;
typedef struct jnc_CallSite jnc_CallSite;

//..............................................................................

// every class instance, every gc-allocated block and every static variable
// whose address has been used by a safe pointer needs a box

enum jnc_BoxFlag
{
	jnc_BoxFlag_WeakMark        = 0x0001,
	jnc_BoxFlag_ClosureWeakMark = 0x0002,
	jnc_BoxFlag_DataMark        = 0x0004,
	jnc_BoxFlag_ClassMark       = 0x0008,
	jnc_BoxFlag_Destructed      = 0x0010,
	jnc_BoxFlag_Static          = 0x0020,
	jnc_BoxFlag_DynamicArray    = 0x0040,
	jnc_BoxFlag_Detached        = 0x0080,
	jnc_BoxFlag_CallSiteLocal   = 0x0100,
	jnc_BoxFlag_Invalid         = 0x0200,
	jnc_BoxFlag_MarkMask        = 0x000f,
};

typedef enum jnc_BoxFlag jnc_BoxFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Box
{
	jnc_Type* m_type;

	uintptr_t m_flags      : 10;

#if (JNC_PTR_BITS == 64)
	uintptr_t m_rootOffset : 54;
#else
	uintptr_t m_rootOffset : 22; // more than enough
#endif
};

//..............................................................................

// structure backing up fat data pointer, e.g.:
// int* p;

struct jnc_DataPtr
{
	void* m_p;
	jnc_DataPtrValidator* m_validator;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// metainfo used for range checking, dynamic casts etc

struct jnc_DataPtrValidator
{
	jnc_Box* m_validatorBox;
	jnc_Box* m_targetBox;
	const void* m_rangeBegin;
	const void* m_rangeEnd;

#ifdef __cplusplus
	size_t
	getRangeLength()
	{
		return (char*)m_rangeEnd - (char*)m_rangeBegin;
	}
#endif
};

//..............................................................................

// structure backing up fat function pointers, e.g.:
// int function* pfTest (int, int);
// int function weak* pfTest (int, int);

struct jnc_FunctionPtr
{
	void* m_p;
	jnc_IfaceHdr* m_closure;
};

//..............................................................................

// structure backing up property pointers, e.g.:
// int property* pxTest;
// int property weak* pxTest;

struct jnc_PropertyPtr
{
	const void* const* m_vtable;
	jnc_IfaceHdr* m_closure;
};

//..............................................................................

// specialized boxes

struct jnc_DataBox
{
	jnc_Box m_box;
	jnc_DataPtrValidator m_validator;

	// followed by actual data
};

struct jnc_DetachedDataBox
{
	jnc_Box m_box;
	jnc_DataPtrValidator m_validator;
	void* m_p; // detached from the actual data (for static & foreign data)
};

//..............................................................................

// header of class iface

struct jnc_IfaceHdr
{
	const void* m_vtable;
	jnc_Box* m_box;

	// followed by parents, then by iface data fields
};

//..............................................................................

// structure backing up multicasts, e.g.:
// multicast f ();

struct jnc_Multicast
{
	jnc_IfaceHdr m_ifaceHdr;
	volatile intptr_t m_lock;
	jnc_DataPtr m_ptr; // array of function closure, weak or unsafe pointers
	size_t m_count;
	size_t m_maxCount;
	void* m_handleTable;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// multicast.getSnapshot method returns an instance of this class:

struct jnc_McSnapshot
{
	jnc_IfaceHdr m_ifaceHdr;
	jnc_DataPtr m_ptr; // array of function closure or unsafe pointers
	size_t m_count;
};

//..............................................................................

// scheduler

typedef
void
jnc_Scheduler_ScheduleFunc(
	jnc_Scheduler* scheduler,
	jnc_FunctionPtr functionPtr
	);

struct jnc_SchedulerVtable
{
	jnc_Scheduler_ScheduleFunc* m_scheduleFunc;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Scheduler
{
	jnc_IfaceHdr m_ifaceHdr;
};

//..............................................................................

// each reactor is represented by this class:

struct jnc_Reactor
{
	jnc_IfaceHdr m_ifaceHdr;
	size_t m_activationCountLimit; // freely adjustible

	// implementation is opaque (see jnc::rtl::ReactorImpl)
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ReactorClosure
{
	jnc_IfaceHdr m_ifaceHdr;
	jnc_Reactor* m_self;
	void* m_binding; // jnc::rtl::ReactorImpl::Binding*
};

//..............................................................................

// structure backing up formatting literal

struct jnc_FmtLiteral
{
	jnc_DataPtr m_ptr;
	size_t m_length;
	size_t m_maxLength;
};

//..............................................................................

struct jnc_GcShadowStackFrame
{
	jnc_GcShadowStackFrame* m_prev;
	jnc_GcShadowStackFrameMap* m_map;
	void** m_gcRootArray; // stack array
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcShadowStackFrameMapBuffer
{
	jnc_ListLink m_link;
	jnc_GcShadowStackFrameMap* m_prev;
	intptr_t m_mapKind;
	intptr_t m_gcRootArray[3];     // maps to jnc::ct::GcShadowStackFrameMap::m_gcRootArray
	intptr_t m_gcRootTypeArray[3]; // maps to jnc::ct::GcShadowStackFrameMap::m_gcRootTypeArray
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcMutatorThread
{
	jnc_ListLink m_link;
	uint64_t m_threadId;
	volatile bool_t m_isSafePoint;
	volatile size_t m_waitRegionLevel;
	size_t m_noCollectRegionLevel;
	jnc_DataPtrValidator* m_dataPtrValidatorPoolBegin;
	jnc_DataPtrValidator* m_dataPtrValidatorPoolEnd;
	jnc_DetachedDataBox* m_foreignDataBoxPoolBegin;
	jnc_DetachedDataBox* m_foreignDataBoxPoolEnd;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef
void
jnc_MarkOpaqueGcRootsFunc(
	jnc_IfaceHdr* iface,
	jnc_GcHeap* gcHeap
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_OpaqueClassTypeInfo
{
	size_t m_size;
	jnc_MarkOpaqueGcRootsFunc* m_markOpaqueGcRootsFunc;
	bool_t m_isNonCreatable;
};

//..............................................................................

struct jnc_SjljFrame
{
	jmp_buf m_jmpBuf;
};

//..............................................................................

struct jnc_Tls
{
	jnc_ListLink m_link;
	jnc_Tls* m_prevTls;
	jnc_Runtime* m_runtime;
	size_t m_initializeLevel;
	jnc_GcMutatorThread m_gcMutatorThread;

	// followed by jnc_TlsVariableTable
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_TlsVariableTable
{
	jnc_SjljFrame* m_sjljFrame;
	jnc_GcShadowStackFrame* m_gcShadowStackTop;
	jnc_IfaceHdr* m_asyncScheduler;

	// followed by user-defined TLS variables
};

//..............................................................................

struct jnc_CallSite
{
	size_t m_initializeLevel;
	size_t m_waitRegionLevel;
	size_t m_noCollectRegionLevel;
	jnc_GcShadowStackFrame m_gcShadowStackDynamicFrame;
	jnc_GcShadowStackFrameMapBuffer m_gcShadowStackDynamicFrameMap;
	intptr_t m_result;
};

//..............................................................................

typedef
void
jnc_StaticConstructFunc();

typedef
void
jnc_StaticDestructFunc();

typedef
void
jnc_ConstructFunc(jnc_IfaceHdr* iface);

typedef
void
jnc_DestructFunc(jnc_IfaceHdr* iface);

//..............................................................................

JNC_SELECT_ANY jnc_DataPtr jnc_g_nullDataPtr = { 0 };
JNC_SELECT_ANY jnc_FunctionPtr jnc_g_nullFunctionPtr = { 0 };
JNC_SELECT_ANY jnc_FunctionPtr jnc_g_nullPropertyPtr = { 0 };

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_BoxFlag BoxFlag;

const BoxFlag
	BoxFlag_WeakMark        = jnc_BoxFlag_WeakMark,
	BoxFlag_ClosureWeakMark = jnc_BoxFlag_ClosureWeakMark,
	BoxFlag_DataMark        = jnc_BoxFlag_DataMark,
	BoxFlag_ClassMark       = jnc_BoxFlag_ClassMark,
	BoxFlag_Destructed      = jnc_BoxFlag_Destructed,
	BoxFlag_Static          = jnc_BoxFlag_Static,
	BoxFlag_DynamicArray    = jnc_BoxFlag_DynamicArray,
	BoxFlag_Detached        = jnc_BoxFlag_Detached,
	BoxFlag_CallSiteLocal   = jnc_BoxFlag_CallSiteLocal,
	BoxFlag_Invalid         = jnc_BoxFlag_Invalid,
	BoxFlag_MarkMask        = jnc_BoxFlag_MarkMask;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef jnc_GcShadowStackFrameMap GcShadowStackFrameMap;
typedef jnc_Box Box;
typedef jnc_DataBox DataBox;
typedef jnc_DetachedDataBox DetachedDataBox;
typedef jnc_DataPtrValidator DataPtrValidator;
typedef jnc_DataPtr DataPtr;
typedef jnc_FunctionPtr FunctionPtr;
typedef jnc_PropertyPtr PropertyPtr;
typedef jnc_IfaceHdr IfaceHdr;
typedef jnc_Multicast Multicast;
typedef jnc_McSnapshot McSnapshot;
typedef jnc_Scheduler Scheduler;
typedef jnc_SchedulerVtable SchedulerVtable;
typedef jnc_Reactor Reactor;
typedef jnc_ReactorClosure ReactorClosure;
typedef jnc_FmtLiteral FmtLiteral;
typedef jnc_GcShadowStackFrame GcShadowStackFrame;
typedef jnc_GcShadowStackFrameMapBuffer GcShadowStackFrameMapBuffer;
typedef jnc_GcMutatorThread GcMutatorThread;
typedef jnc_MarkOpaqueGcRootsFunc MarkOpaqueGcRootsFunc;
typedef jnc_OpaqueClassTypeInfo OpaqueClassTypeInfo;
typedef jnc_SjljFrame SjljFrame;
typedef jnc_Tls Tls;
typedef jnc_TlsVariableTable TlsVariableTable;
typedef jnc_CallSite CallSite;
typedef jnc_StaticConstructFunc StaticConstructFunc;
typedef jnc_StaticDestructFunc StaticDestructFunc;
typedef jnc_ConstructFunc ConstructFunc;
typedef jnc_DestructFunc DestructFunc;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_SELECT_ANY DataPtr g_nullDataPtr = { 0 };
JNC_SELECT_ANY FunctionPtr g_nullFunctionPtr = { 0 };
JNC_SELECT_ANY PropertyPtr g_nullPropertyPtr = { 0 };

//..............................................................................

class GetGcMutatorThreadLink
{
public:
	ListLink*
	operator () (GcMutatorThread* thread)
	{
		return &thread->m_link;
	}
};

class GetTlsLink
{
public:
	ListLink*
	operator () (Tls* tls)
	{
		return &tls->m_link;
	}
};

//..............................................................................

// iface inside a box

// since in C++03 we can't get alignof (T), we need to provide multiple boxes for
// all kinds of alignments (1, 2, 4, 8)

template <typename T>
class ClassBoxBase: public Box
{
public:
	T* p()
	{
		return (T*)(this + 1);
	}

	operator T* ()
	{
		return p();
	}

	T* operator -> ()
	{
		return p();
	}
};

#pragma pack(push, 1)

template <typename T>
class ClassBox_align1: public ClassBoxBase<T>
{
protected:
	char m_buffer[sizeof(T)];
} JNC_GCC_MSC_STRUCT;

#pragma pack(2)

template <typename T>
class ClassBox_align2: public ClassBoxBase<T>
{
protected:
	char m_buffer[sizeof(T)];
} JNC_GCC_MSC_STRUCT;

#pragma pack(4)

template <typename T>
class ClassBox_align4: public ClassBoxBase<T>
{
protected:
	char m_buffer[sizeof(T)];
} JNC_GCC_MSC_STRUCT;

#pragma pack(8)

template <typename T>
class ClassBox_align8: public ClassBoxBase<T>
{
protected:
#if (JNC_PTR_SIZE == 8) // 8-byte alignment will be forced by Box/IfaceHdr
	char m_buffer[sizeof(T)];
#else
	union
	{
		/// \unnamed{union}
		char m_buffer[sizeof(T)];
		int64_t m_align8; // otherwise, we need an 8-byte field
	} JNC_GCC_MSC_STRUCT;
#endif
} JNC_GCC_MSC_STRUCT;

#pragma pack(pop)

// default alignment

template <typename T>
class ClassBox: public ClassBoxBase<T>
{
protected:
	char m_buffer[sizeof(T)];
};

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
