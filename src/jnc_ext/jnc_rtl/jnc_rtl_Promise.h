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

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Promise)
JNC_DECLARE_CLASS_TYPE(Promisifier)

//..............................................................................

class Promise: public IfaceHdr
{
protected:
	enum State
	{
		State_Initial   = 0,
		State_Completed = -1,
	};

	enum AsyncWaitKind
	{
		AsyncWaitKind_NoArgs,
		AsyncWaitKind_ErrorArg,
		AsyncWaitKind_ResultErrorArgs,
	};

	struct AsyncWait: sl::ListLink
	{
		AsyncWaitKind m_waitKind;
		FunctionPtr m_handlerPtr;
		uintptr_t m_handle;
	};

	struct SyncWait: sl::ListLink
	{
		sys::Event* m_event;
	};

public:
	size_t m_state;
	IfaceHdr* m_scheduler;
	Promise* m_pendingPromise;
	GcShadowStackFrame* m_gcShadowStackFrame;
	Variant m_result;
	DataPtr m_errorPtr;

protected:
	sys::Lock m_lock;

	sl::AuxList<SyncWait> m_syncWaitList;
	sl::List<AsyncWait> m_asyncWaitList;
	sl::HandleTable<AsyncWait*> m_asyncWaitMap;

#if (JNC_PTR_SIZE == 4)
	char m_padding[4]; // ensure the same layout regardless of pack factor
#endif

public:
	Promise();

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	uintptr_t
	JNC_CDECL
	wait_0(FunctionPtr handlerPtr);

	uintptr_t
	JNC_CDECL
	wait_1(FunctionPtr handlerPtr);

	uintptr_t
	JNC_CDECL
	wait_2(FunctionPtr handlerPtr);

	bool
	JNC_CDECL
	cancelWait(uintptr_t handle);

	static
	Variant
	blockingWait(Promise* self)
	{
		return self->blockingWaitImpl();
	}

	Promise*
	JNC_CDECL
	asyncWait()
	{
		return this;
	}

protected:
	uintptr_t
	addAsyncWait_l(
		AsyncWaitKind waitKind,
		FunctionPtr handlerPtr
		);

	Variant
	blockingWaitImpl();
};

//..............................................................................

class Promisifier: public Promise
{
public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap)
	{
		Promise::markOpaqueGcRoots(gcHeap);
	}

	void
	JNC_CDECL
	complete_0()
	{
		complete_2(g_nullVariant, g_nullDataPtr);
	}

	void
	JNC_CDECL
	complete_1(DataPtr errorPtr)
	{
		complete_2(g_nullVariant, errorPtr);
	}

	void
	JNC_CDECL
	complete_2(
		Variant result,
		DataPtr errorPtr
		);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
