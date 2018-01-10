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

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (SocketAddressResolver)

//..............................................................................

class SocketAddressResolver: public IfaceHdr
{
	friend class IoThread;

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, SocketAddressResolver, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoThreadFlag
	{
		IoThreadFlag_Running = 0x0001,
		IoThreadFlag_Closing = 0x0010,
	};

	struct Req: sl::ListLink
	{
		sl::String m_name;
		uint_t m_port;
		uint_t m_addrFamily;
		uintptr_t m_id;

		FunctionPtr m_completionFuncPtr;
	};

protected:
	Runtime* m_runtime;

	sys::Lock m_lock;
	uint_t m_ioThreadFlags;
	IoThread m_ioThread;
	sl::StdList <Req> m_pendingReqList;
	sl::StdList <Req> m_activeReqList;
	sl::HandleTable <Req*> m_reqMap;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	SocketAddressResolver ();
	~SocketAddressResolver ();

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	uintptr_t
	JNC_CDECL
	resolve (
		DataPtr namePtr,
		uint16_t addrFamily,
		FunctionPtr completionFuncPtr
		);

	bool
	JNC_CDECL
	cancel (uintptr_t id);

	void
	JNC_CDECL
	cancelAll ();

protected:
	void
	callCompletionFunc (
		FunctionPtr completionFuncPtr,
		const axl::io::SockAddr* addressTable,
		size_t addressCount,
		const err::ErrorHdr* error = NULL
		);

	void
	callCompletionFunc (
		FunctionPtr completionFuncPtr,
		const err::Error& error
		)
	{
		callCompletionFunc (completionFuncPtr, NULL, 0, error);
	}

	void
	processReq (Req* req);

	void
	stopIoThread ();

	void
	ioThreadFunc ();

	void
	wakeIoThread ();
};

//..............................................................................

} // namespace io
} // namespace jnc
