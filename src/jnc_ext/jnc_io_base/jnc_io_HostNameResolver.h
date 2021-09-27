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

#include "jnc_io_AsyncIoBase.h"
#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(HostNameResolver)

//..............................................................................

enum HostNameResolverEvent {
	HostNameResolverEvent_Error    = AsyncIoBaseEvent_IoError,
	HostNameResolverEvent_Resolved = 0x02,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct HostNameResolverHdr: IfaceHdr {
	DataPtr m_addressTablePtr;
	size_t m_addressCount;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HostNameResolver:
	public HostNameResolverHdr,
	public AsyncIoBase {
	friend class IoThread;

protected:
	enum IoThreadFlag {
		IoThreadFlag_Started = 0x0100,
	};

	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, HostNameResolver, m_ioThread)->ioThreadFunc();
		}
	};

protected:
	IoThread m_ioThread;
	DataPtr m_pendingAddressTablePtr;
	sl::String m_name;
	uint_t m_addrFamily;
	uint_t m_port;

public:
	~HostNameResolver() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	bool
	JNC_CDECL
	resolve(
		DataPtr namePtr,
		uint16_t addrFamily
	);

	void
	JNC_CDECL
	cancel();

	void
	JNC_CDECL
	close();

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
	) {
		return AsyncIoBase::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle) {
		return AsyncIoBase::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
	) {
		return AsyncIoBase::blockingWait(eventMask, timeout);
	}

	Promise*
	JNC_CDECL
	asyncWait(uint_t eventMask) {
		return AsyncIoBase::asyncWait(eventMask);
	}

protected:
	void
	ioThreadFunc();

	void
	complete_l(
		const axl::io::SockAddr* addressTable,
		size_t count
	);

	void
	complete(
		const axl::io::SockAddr* addressTable,
		size_t count
	) {
		m_lock.lock();
		complete_l(addressTable, count);
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
