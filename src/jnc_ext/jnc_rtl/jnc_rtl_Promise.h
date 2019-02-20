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

JNC_DECLARE_OPAQUE_CLASS_TYPE (Promise)
JNC_DECLARE_CLASS_TYPE (Promisifier)

//..............................................................................

class Promise: public IfaceHdr
{
protected:
	enum State
	{
		State_Initial   = 0,
		State_Completed = -1,
	};

	struct SyncWait: sl::ListLink
	{
		sys::Event* m_event;
	};

	struct AsyncWait: sl::ListLink
	{
		FunctionPtr m_handlerPtr;
		handle_t m_handle;
	};

public:
	size_t m_state;
	Variant m_result;
	DataPtr m_errorPtr;

protected:
	sys::Lock m_lock;

	sl::AuxList <SyncWait> m_syncWaitList;
	sl::List <AsyncWait> m_asyncWaitList;
	sl::HandleTable <AsyncWait*> m_asyncWaitMap;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	handle_t
	JNC_CDECL
	wait (FunctionPtr handlerPtr);

	bool
	JNC_CDECL
	cancelWait (handle_t handle);

	static
	Variant
	blockingWait (Promise* self)
	{
		return self->blockingWaitImpl ();
	}

	Promise*
	JNC_CDECL
	asyncWait ()
	{
		return this;
	}

protected:
	Variant
	blockingWaitImpl ();
};

//..............................................................................

class Promisifier: public Promise
{
public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap)
	{
		Promise::markOpaqueGcRoots (gcHeap);
	}

	void
	JNC_CDECL
	complete_0 ()
	{
		complete_2 (g_nullVariant, g_nullPtr);
	}

	void
	JNC_CDECL
	complete_1 (DataPtr errorPtr)
	{
		complete_2 (g_nullVariant, errorPtr);
	}

	void
	JNC_CDECL
	complete_2 (
		Variant result,
		DataPtr errorPtr
		);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
