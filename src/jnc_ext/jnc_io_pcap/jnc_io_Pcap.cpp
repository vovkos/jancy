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
#include "jnc_io_Pcap.h"
#include "jnc_io_PcapLib.h"
#include "jnc_Error.h"

#if (_JNC_OS_POSIX)
#	include "jnc_io_PcapSignalMgr.h"
#endif

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Pcap,
	"io.Pcap",
	g_pcapLibGuid,
	PcapLibCacheSlot_Pcap,
	Pcap,
	&Pcap::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Pcap)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<Pcap>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Pcap>)

	JNC_MAP_CONST_PROPERTY("m_dlt", &Pcap::getDlt)
	JNC_MAP_CONST_PROPERTY("m_linkType", &Pcap::getLinkType)
	JNC_MAP_AUTOGET_PROPERTY("m_isPromiscious", &Pcap::setPromiscious)
	JNC_MAP_AUTOGET_PROPERTY("m_readTimeout", &Pcap::setReadTimeout)
	JNC_MAP_PROPERTY("m_snapshotSize", &Pcap::getSnapshotSize, &Pcap::setSnapshotSize)
	JNC_MAP_AUTOGET_PROPERTY("m_kernelBufferSize", &Pcap::setKernelBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &Pcap::setReadBufferSize)

	JNC_MAP_FUNCTION("openDevice", &Pcap::openDevice)
	JNC_MAP_FUNCTION("openLive", &Pcap::openLive)
	JNC_MAP_FUNCTION("openFile", &Pcap::openFile)
	JNC_MAP_FUNCTION("close", &Pcap::close)
	JNC_MAP_FUNCTION("activate", &Pcap::activate)
	JNC_MAP_FUNCTION("setFilter", &Pcap::setFilter)
	JNC_MAP_FUNCTION("write", &Pcap::write)
	JNC_MAP_FUNCTION("read", &Pcap::read)
	JNC_MAP_FUNCTION("wait", &Pcap::wait)
	JNC_MAP_FUNCTION("cancelWait", &Pcap::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &Pcap::blockingWait)
	JNC_MAP_FUNCTION("asyncWait", &Pcap::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	PcapAddress,
	"io.PcapAddress",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapAddress
)

JNC_BEGIN_TYPE_FUNCTION_MAP(PcapAddress)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	PcapDeviceDesc,
	"io.PcapDeviceDesc",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapDeviceDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(PcapDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Pcap::Pcap() {
	m_readBufferSize = Def_ReadBufferSize;
	m_readBuffer.setBufferSize(Def_ReadBufferSize);
}

bool
JNC_CDECL
Pcap::openDevice(String deviceName) {
	close();

	return
		requirePcapCapability() &&
		m_pcap.openDevice(deviceName >> toAxl) &&
		m_pcap.setPromiscious(Def_IsPromiscious) &&
		m_pcap.setPromiscious(Def_IsPromiscious) &&
		m_pcap.setTimeout(Def_ReadTimeout) &&
		finishOpen(IoThreadFlag_Suspended);
}

bool
JNC_CDECL
Pcap::openLive(
	String deviceName,
	String filter,
	uint_t snapshotSize,
	bool isPromiscious,
	uint_t readTimeout
) {
	close();

	bool result =
		requirePcapCapability() &&
		m_pcap.openLive(
			deviceName >> toAxl,
			snapshotSize,
			isPromiscious,
			readTimeout
		) &&
		setFilter(filter, true, PCAP_NETMASK_UNKNOWN);

	if (!result)
		return false;

	m_isPromiscious = isPromiscious;
	m_readTimeout = readTimeout;
	return finishOpen();
}

bool
JNC_CDECL
Pcap::openFile(
	String fileName,
	String filter
) {
	close();

	return
		requirePcapCapability() &&
		m_pcap.openFile(fileName >> toAxl) &&
		setFilter(filter, true, PCAP_NETMASK_UNKNOWN) &&
		finishOpen();
}

bool
Pcap::finishOpen(uint_t ioThreadFlags) {
	AsyncIoDevice::open();

	m_options |= AsyncIoDeviceOption_KeepReadWriteBlockSize;
	m_ioThreadFlags |= ioThreadFlags | IoThreadFlag_Datagram;
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
Pcap::close() {
	if (!m_pcap.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

#if (_JNC_OS_POSIX)
	sl::getSimpleSingleton<PcapSignalMgr>()->install();
	::pthread_kill((pthread_t)m_ioThread.getThreadId(), PcapSignalMgr::Signal);
#endif

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

#if (_JNC_OS_POSIX)
	sl::getSimpleSingleton<PcapSignalMgr>()->uninstall();
#endif

	m_pcap.close();

	AsyncIoDevice::close();

	m_kernelBufferSize = 0;
	m_isPromiscious = false;
	m_readTimeout = 0;
	m_filter = g_nullString;
}

void
JNC_CDECL
Pcap::setPromiscious(bool isPromiscious) {
	bool result = m_pcap.setPromiscious(isPromiscious);
	if (!result)
		dynamicThrow();

	m_isPromiscious = isPromiscious;
}

void
JNC_CDECL
Pcap::setReadTimeout(uint_t timeout) {
	bool result = m_pcap.setTimeout(timeout);
	if (!result)
		dynamicThrow();

	m_readTimeout = timeout;
}

void
JNC_CDECL
Pcap::setSnapshotSize(size_t size) {
	bool result = m_pcap.setSnapshotSize(size);
	if (!result)
		dynamicThrow();
}

void
JNC_CDECL
Pcap::setKernelBufferSize(size_t size) {
	bool result = m_pcap.setBufferSize(size);
	if (!result)
		dynamicThrow();

	m_kernelBufferSize = size;
}

bool
JNC_CDECL
Pcap::activate(String filter) {
	bool result =
		m_pcap.activate() &&
		setFilter(filter, true, PCAP_NETMASK_UNKNOWN);

	if (!result)
		return false;

	unsuspendIoThread();
	return true;
}

bool
JNC_CDECL
Pcap::setFilter(
	String filter0,
	bool isOptimized,
	uint32_t netMask
) {
	sl::StringRef filter = filter0 >> toAxl;
	bool result = m_pcap.setFilter(filter, isOptimized, netMask);
	if (!result)
		return false;

	m_filter = allocateString(filter);
	return true;
}

size_t
JNC_CDECL
Pcap::read(
	DataPtr dataPtr,
	size_t size,
	DataPtr timestampPtr
) {
	size_t result;

	if (!timestampPtr.m_p) {
		result = bufferedRead(dataPtr, size);
	} else {
		char buffer[256];
		sl::Array<char> params(rc::BufKind_Stack, buffer, sizeof(buffer));
		result = bufferedRead(dataPtr, size, &params);

		ASSERT(params.getCount() == sizeof(uint64_t));
		*(uint64_t*)timestampPtr.m_p = *(uint64_t*)params.p();
	}

	return result;
}

size_t
JNC_CDECL
Pcap::write(
	DataPtr ptr,
	size_t size
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	return m_pcap.write(ptr.m_p, size);
}

void
Pcap::ioThreadFunc() {
	ASSERT(m_pcap.isOpen());

#if (_JNC_OS_POSIX)
	sys::setTlsPtrSlotValue<Pcap>(this);
#endif

	m_lock.lock();

	for (;;) {
		// check for IoThreadFlag_Closing now; we might have
		// called ::pthread_kill before setting the TLS slot

		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		if (!(m_ioThreadFlags & IoThreadFlag_Suspended))
			break;

		m_lock.unlock();
		sleepIoThread();
		m_lock.lock();
	}

	m_lock.unlock();

	size_t snapshotSize = m_pcap.getSnapshotSize();

	sl::Array<char> readBuffer;
	readBuffer.setCount(snapshotSize);
	char* p = readBuffer.p();

	for (;;) {
		uint64_t timestamp;

		size_t readResult = m_pcap.read(p, snapshotSize, &timestamp);
		if ((intptr_t)readResult < 0) {
			readResult == PCAP_ERROR_BREAK ?
				setEvents(PcapEvent_Eof) :
				setIoErrorEvent(m_pcap.getLastErrorDescription());

			break;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		if (readResult == 0) { // timeout
			m_lock.unlock();
			continue;
		}

		while (m_readBuffer.isFull()) {
			m_lock.unlock();
			sleepIoThread();
			m_lock.lock();

			if (m_ioThreadFlags & IoThreadFlag_Closing) {
				m_lock.unlock();
				return;
			}
		}

		addToReadBuffer(readBuffer, readResult, &timestamp, sizeof(timestamp));

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

//..............................................................................

JNC_INLINE
uint32_t
getIpFromSockAddr(const sockaddr* sockAddr) {
	return sockAddr && sockAddr->sa_family == AF_INET ?
#if (_JNC_OS_WIN)
		((const sockaddr_in*)sockAddr)->sin_addr.S_un.S_addr :
#elif (_JNC_OS_POSIX)
		((const sockaddr_in*)sockAddr)->sin_addr.s_addr :
#endif
		0;
}

void
setupPcapAddress(
	Runtime* runtime,
	PcapAddress* address,
	const pcap_addr* ifaceAddr
) {
	for (; ifaceAddr; ifaceAddr = ifaceAddr->next)
		if (ifaceAddr->addr && ifaceAddr->addr->sa_family == AF_INET)
			break;

	if (!ifaceAddr)
		return; // no IP4 addresses found

	GcHeap* gcHeap = runtime->getGcHeap();
	Type* addressType = PcapAddress_getType(runtime->getModule());

	address->m_address = getIpFromSockAddr(ifaceAddr->addr);
	address->m_mask = getIpFromSockAddr(ifaceAddr->netmask);
	address->m_broadcast = getIpFromSockAddr(ifaceAddr->broadaddr);

	PcapAddress* prevAddress = address;
	for (ifaceAddr = ifaceAddr->next; ifaceAddr; ifaceAddr = ifaceAddr->next) {
		if (!ifaceAddr->addr || ifaceAddr->addr->sa_family != AF_INET)
			continue;

		DataPtr addressPtr = gcHeap->allocateData(addressType);
		address = (PcapAddress*)addressPtr.m_p;
		address->m_address = getIpFromSockAddr(ifaceAddr->addr);
		address->m_mask = getIpFromSockAddr(ifaceAddr->netmask);
		address->m_broadcast = getIpFromSockAddr(ifaceAddr->broadaddr);

		prevAddress->m_nextPtr = addressPtr;
		prevAddress = address;
	}
}

DataPtr
createPcapDeviceDescList(DataPtr countPtr) {
	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = 0;

	pcap_if* ifaceList = NULL;
	char errorBuffer[PCAP_ERRBUF_SIZE] = { 0 };
	int result = pcap_findalldevs(&ifaceList, errorBuffer);
	if (result == -1) {
		err::setError(errorBuffer);
		return g_nullDataPtr;
	}

	if (!ifaceList)
		return g_nullDataPtr;

	Runtime* runtime = getCurrentThreadRuntime();
	GcHeap* gcHeap = runtime->getGcHeap();
	Type* deviceType = PcapDeviceDesc_getType(runtime->getModule());
	NoCollectRegion noCollectRegion(gcHeap, false);

	size_t count = 1;

	pcap_if* iface = ifaceList;

	DataPtr devicePtr = gcHeap->allocateData(deviceType);
	PcapDeviceDesc* device = (PcapDeviceDesc*)devicePtr.m_p;
	device->m_name = allocateString(iface->name);
	device->m_description = allocateString(iface->description);
	setupPcapAddress(runtime, &device->m_address, iface->addresses);

	DataPtr resultPtr = devicePtr;

	PcapDeviceDesc* prevDevice = device;
	for (iface = iface->next; iface; iface = iface->next, count++) {
		devicePtr = gcHeap->allocateData(deviceType);
		device = (PcapDeviceDesc*)devicePtr.m_p;
		device->m_name = allocateString(iface->name);
		device->m_description = allocateString(iface->description);
		setupPcapAddress(runtime, &device->m_address, iface->addresses);

		prevDevice->m_nextPtr = devicePtr;
		prevDevice = device;
	}

	pcap_freealldevs(ifaceList);

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
