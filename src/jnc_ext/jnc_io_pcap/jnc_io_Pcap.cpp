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

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Pcap,
	"io.Pcap",
	g_pcapLibGuid,
	PcapLibCacheSlot_Pcap,
	Pcap,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Pcap)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Pcap>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Pcap>)

	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &Pcap::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize", &Pcap::setWriteBufferSize)

	JNC_MAP_FUNCTION ("openDevice",   &Pcap::openDevice)
	JNC_MAP_FUNCTION ("openFile",     &Pcap::openFile)
	JNC_MAP_FUNCTION ("close",        &Pcap::close)
	JNC_MAP_FUNCTION ("setFilter",    &Pcap::setFilter)
	JNC_MAP_FUNCTION ("write",        &Pcap::write)
	JNC_MAP_FUNCTION ("read",         &Pcap::read)
	JNC_MAP_FUNCTION ("wait",         &Pcap::wait)
	JNC_MAP_FUNCTION ("cancelWait",   &Pcap::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait", &Pcap::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	PcapAddress,
	"io.PcapAddress",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapAddress
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (PcapAddress)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	PcapDeviceDesc,
	"io.PcapDeviceDesc",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapDeviceDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (PcapDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

Pcap::Pcap ()
{
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
	m_writeBuffer.setBufferSize (Def_WriteBufferSize);
}

bool
JNC_CDECL
Pcap::openDevice (
	DataPtr deviceNamePtr,
	DataPtr filterPtr,
	uint_t snapshotSize,
	bool isPromiscious,
	uint_t readTimeout
	)
{
	bool result;

	const char* deviceName = (const char*) deviceNamePtr.m_p;
	const char* filter = (const char*) filterPtr.m_p;

	close ();

	result =
		m_pcap.openDevice (deviceName, snapshotSize, isPromiscious, readTimeout) &&
		m_pcap.setFilter (filter);

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_snapshotSize = snapshotSize;
	m_isPromiscious = isPromiscious;
	m_readTimeout = readTimeout;
	m_filterPtr = strDup (filter);

	return finishOpen ();
}

bool
JNC_CDECL
Pcap::openFile (
	DataPtr fileNamePtr,
	DataPtr filterPtr
	)
{
	bool result;

	const char* fileName = (const char*) fileNamePtr.m_p;
	const char* filter = (const char*) filterPtr.m_p;

	close ();

	result =
		m_pcap.openFile (fileName) &&
		m_pcap.setFilter (filter);

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_filterPtr = strDup (filter);

	return finishOpen ();
}

bool
Pcap::finishOpen ()
{
	AsyncIoDevice::open ();

	m_ioThreadFlags |= IoThreadFlag_Datagram;
	m_options |= AsyncIoOption_KeepReadBlockSize | AsyncIoOption_KeepWriteBlockSize;

	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
Pcap::close ()
{
	if (!m_pcap.isOpen ())
		return;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_pcap.close ();

	AsyncIoDevice::close ();
	m_snapshotSize = 0;
	m_isPromiscious = false;
	m_readTimeout = 0;
}

bool
JNC_CDECL
Pcap::setFilter (DataPtr filterPtr)
{
	const char* filter = (const char*) filterPtr.m_p;

	bool result = m_pcap.setFilter (filter);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_filterPtr = strDup (filter);
	return true;
}

size_t
JNC_CDECL
Pcap::read (
	DataPtr dataPtr,
	size_t size,
	DataPtr timestampPtr
	)
{
	size_t result;

	if (!timestampPtr.m_p)
	{
		result = bufferedRead (dataPtr, size);
	}
	else
	{
		char buffer [256];
		sl::Array <char> params (ref::BufKind_Stack, buffer, sizeof (buffer));
		result = bufferedRead (dataPtr, size, &params);

		ASSERT (params.getCount () == sizeof (uint64_t));
		*(uint64_t*) timestampPtr.m_p = *(uint64_t*) params.p ();
	}

	return result;
}

void
Pcap::ioThreadFunc ()
{
	ASSERT (m_pcap.isOpen ());

	sl::Array <char> readBuffer;
	readBuffer.setCount (m_snapshotSize);

	for (;;)
	{
		uint64_t timestamp;

		size_t readResult = m_pcap.read (readBuffer, m_snapshotSize, &timestamp);
		if (readResult == -1)
		{
			setIoErrorEvent ();
			break;
		}

		if (readResult == -2)
		{
			setEvents (PcapEvent_Eof);
			break;
		}

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		if (readResult == 0) // timeout
		{
			m_lock.unlock ();
			continue;
		}

		while (m_readBuffer.isFull ())
		{
			m_lock.unlock ();
			sleepIoThread ();
			m_lock.lock ();

			if (m_ioThreadFlags & IoThreadFlag_Closing)
			{
				m_lock.unlock ();
				return;
			}
		}

		addToReadBuffer (readBuffer, readResult, &timestamp, sizeof (timestamp));

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents ();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();
	}
}

//..............................................................................

JNC_INLINE
uint32_t
getIpFromSockAddr (const sockaddr* sockAddr)
{
	return sockAddr && sockAddr->sa_family == AF_INET ?
#if (_JNC_OS_WIN)
		((const sockaddr_in*) sockAddr)->sin_addr.S_un.S_addr :
#elif (_JNC_OS_POSIX)
		((const sockaddr_in*) sockAddr)->sin_addr.s_addr :
#endif
		0;
}

void
setupPcapAddress (
	Runtime* runtime,
	PcapAddress* address,
	const pcap_addr* ifaceAddr
	)
{
	for (; ifaceAddr; ifaceAddr = ifaceAddr->next)
		if (ifaceAddr->addr && ifaceAddr->addr->sa_family == AF_INET)
			break;

	if (!ifaceAddr)
		return; // no IP4 addresses found

	GcHeap* gcHeap = runtime->getGcHeap ();
	Type* addressType = PcapAddress_getType (runtime->getModule ());

	address->m_address = getIpFromSockAddr (ifaceAddr->addr);
	address->m_mask = getIpFromSockAddr (ifaceAddr->netmask);
	address->m_broadcast = getIpFromSockAddr (ifaceAddr->broadaddr);

	PcapAddress* prevAddress = address;
	for (ifaceAddr = ifaceAddr->next; ifaceAddr; ifaceAddr = ifaceAddr->next)
	{
		if (!ifaceAddr->addr || ifaceAddr->addr->sa_family != AF_INET)
			continue;

		DataPtr addressPtr = gcHeap->allocateData (addressType);
		address = (PcapAddress*) addressPtr.m_p;
		address->m_address = getIpFromSockAddr (ifaceAddr->addr);
		address->m_mask = getIpFromSockAddr (ifaceAddr->netmask);
		address->m_broadcast = getIpFromSockAddr (ifaceAddr->broadaddr);

		prevAddress->m_nextPtr = addressPtr;
		prevAddress = address;
	}
}

DataPtr
createPcapDeviceDescList (DataPtr countPtr)
{
	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = 0;

	pcap_if* ifaceList = NULL;
	char errorBuffer [PCAP_ERRBUF_SIZE] = { 0 };
	int result = pcap_findalldevs (&ifaceList, errorBuffer);
	if (result == -1)
	{
		err::setError (errorBuffer);
		return g_nullPtr;
	}

	if (!ifaceList)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ScopedNoCollectRegion noCollectRegion (runtime, false);

	GcHeap* gcHeap = runtime->getGcHeap ();
	Type* deviceType = PcapDeviceDesc_getType (runtime->getModule ());

	size_t count = 1;

	pcap_if* iface = ifaceList;

	DataPtr devicePtr = gcHeap->allocateData (deviceType);
	PcapDeviceDesc* device = (PcapDeviceDesc*) devicePtr.m_p;
	device->m_namePtr = strDup (iface->name);
	device->m_descriptionPtr = strDup (iface->description);
	setupPcapAddress (runtime, &device->m_address, iface->addresses);

	DataPtr resultPtr = devicePtr;

	PcapDeviceDesc* prevDevice = device;
	for (iface = iface->next; iface; iface = iface->next, count++)
	{
		devicePtr = gcHeap->allocateData (deviceType);
		device = (PcapDeviceDesc*) devicePtr.m_p;
		device->m_namePtr = strDup (iface->name);
		device->m_descriptionPtr = strDup (iface->description);
		setupPcapAddress (runtime, &device->m_address, iface->addresses);

		prevDevice->m_nextPtr = devicePtr;
		prevDevice = device;
	}

	pcap_freealldevs (ifaceList);

	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
