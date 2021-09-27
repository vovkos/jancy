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
#include "jnc_io_HostNameResolver.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HostNameResolver,
	"io.HostNameResolver",
	g_ioLibGuid,
	IoLibCacheSlot_HostNameResolver,
	HostNameResolver,
	&HostNameResolver::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HostNameResolver)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HostNameResolver>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HostNameResolver>)
	JNC_MAP_FUNCTION("resolve", &HostNameResolver::resolve)
	JNC_MAP_FUNCTION("cancel", &HostNameResolver::cancel)
	JNC_MAP_FUNCTION("close", &HostNameResolver::close)
	JNC_MAP_FUNCTION("wait", &HostNameResolver::wait)
	JNC_MAP_FUNCTION("cancelWait", &HostNameResolver::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &HostNameResolver::blockingWait)
	JNC_MAP_FUNCTION("asyncWait", &HostNameResolver::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
HostNameResolver::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	AsyncIoBase::markOpaqueGcRoots(gcHeap);
	gcHeap->markDataPtr(m_pendingAddressTablePtr);
}

bool
JNC_CDECL
HostNameResolver::resolve(
	DataPtr namePtr,
	uint16_t addrFamily
) {
	cancel();

	if (!requireIoLibCapability(IoLibCapability_HostNameResolver))
		return false;

	if (!m_isOpen)
		AsyncIoBase::open();

	const char* name = (const char*)namePtr.m_p;

	axl::io::SockAddr sockAddr;
	bool result = sockAddr.parse(name);
	if (result) {
		complete(&sockAddr, 1);
		return true;
	}

	sl::String nameString;
	uint_t port;

	const char* p = strchr(name, ':');
	if (!p) {
		nameString = name;
		port = 0;
	} else {
		char* end;
		port = (ushort_t)strtol(p + 1, &end, 10);
		if (end == p) {
			setIoErrorEvent("invalid port string");
			return false;
		}

		nameString.copy(name, p - name);
	}

	m_lock.lock();
	m_name = nameString;
	m_addrFamily = addrFamily;
	m_port = sl::swapByteOrder16(port);

	if (m_ioThreadFlags & IoThreadFlag_Started) {
		wakeIoThread();
		m_lock.unlock();
		return true;
	}

	m_lock.unlock();

	m_ioThreadFlags |= IoThreadFlag_Started;
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
HostNameResolver::cancel() {
	m_lock.lock();
	m_name.clear();
	// currently, no need to wakeIoThread() -- ::getaddrinfo is not cancellable anyway
	// later, at least on Windows, we may switch to the cancellable ::GetAddrInfoEx
	m_activeEvents = 0;
	m_lock.unlock();
}

void
JNC_CDECL
HostNameResolver::close() {
	m_lock.lock();
	m_name.clear();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	m_ioThreadFlags &= ~IoThreadFlag_Started;
	m_activeEvents = 0;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	AsyncIoBase::close();
}

void
HostNameResolver::ioThreadFunc() {
	wakeIoThread();

	sl::Array<axl::io::SockAddr> addrArray;

	for (;;) {
		sleepIoThread();

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		if (m_name.isEmpty()) {
			m_lock.unlock();
			continue;
		}

		sl::StringRef name = m_name;
		uint_t addrFamily = m_addrFamily;
		uint_t port = m_port;
		m_lock.unlock();

		bool result = axl::io::resolveHostName(&addrArray, name, addrFamily);

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		if (name != m_name ||
			addrFamily != m_addrFamily ||
			port != m_port)
			m_lock.unlock(); // cancelled
		else if (!result)
			setIoErrorEvent_l();
		else
			complete_l(addrArray, addrArray.getCount());
	}
}

void
HostNameResolver::complete_l(
	const axl::io::SockAddr* addressTable,
	size_t count
) {
	sl::StringRef name = m_name;
	uint_t addrFamily = m_addrFamily;
	uint_t port = m_port;
	m_lock.unlock();

	JNC_BEGIN_CALL_SITE(m_runtime)
	m_pendingAddressTablePtr = memDup(addressTable, count * sizeof(axl::io::SockAddr));
	JNC_END_CALL_SITE()

	m_lock.lock();

	if (name != m_name ||
		addrFamily != m_addrFamily ||
		port != m_port) {
		m_lock.unlock(); // cancelled during memDup
		return;
	}

	SocketAddress* p = (SocketAddress*)m_pendingAddressTablePtr.m_p;
	SocketAddress* end = p + count;
	for (; p < end; p++) {
		int family = ((axl::io::SockAddr*)p)->m_addr.sa_family;
		p->m_family = family == AF_INET6 ? AddressFamily_Ip6 : family;
		p->m_port = port;
	}

	m_addressTablePtr = m_pendingAddressTablePtr;
	m_addressCount = count;
	m_pendingAddressTablePtr = g_nullDataPtr;
	setEvents_l(HostNameResolverEvent_Resolved);
}

//..............................................................................

} // namespace io
} // namespace jnc
