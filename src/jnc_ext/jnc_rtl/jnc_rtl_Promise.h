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

JNC_DECLARE_CLASS_TYPE (Promise)
JNC_DECLARE_CLASS_TYPE (Promisifier)

//..............................................................................

class Promise: public IfaceHdr
{
protected:
	struct SyncWait: sl::ListLink
	{
		sys::Event* m_event;
	};

	struct AsyncWait: sl::ListLink
	{
		FunctionPtr m_handlerPtr;
		handle_t m_handle;
	};

protected:
	sys::Lock m_lock;

	sl::AuxList <SyncWait> m_syncWaitList;
	sl::List <AsyncWait> m_asyncWaitList;
	sl::List <AsyncWait> m_pendingAsyncWaitList;
	sl::HandleTable <AsyncWait*> m_asyncWaitMap;

	Variant m_result;
	DataPtr m_errorPtr;
	bool m_isCompleted;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	intptr_t
	JNC_CDECL
	wait (FunctionPtr handlerPtr);

	bool
	JNC_CDECL
	cancelWait (intptr_t handle);

	static
	Variant
	blockingWait (Promise* self);

	Promise*
	JNC_CDECL
	asyncWait ()
	{
		return this;
	}
};

//..............................................................................

class Promisifier: public Promise
{
public:
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

Promise*
createMultiPromise (
	DataPtr promiseArrayPtr,
	size_t count,
	bool waitAll
	);

//..............................................................................

} // namespace rtl
} // namespace jnc
