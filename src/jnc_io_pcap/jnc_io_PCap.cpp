#include "pch.h"
#include "jnc_io_PCap.h"

namespace jnc {
namespace io {

//.............................................................................

PCap::PCap ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_ioFlags = 0;
	memset (&m_filter, 0, sizeof (m_filter));
	m_isPromiscious = false;
	m_isOpen = false;
	m_syncId = 0;
}

void
PCap::firePCapEvent (PCapEventKind eventKind)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = rt::createData <PCapEventParams> (m_runtime);
	PCapEventParams* params = (PCapEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;

	rt::callMulticast (m_onPCapEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
AXL_CDECL
PCap::setFilter (DataPtr filter)
{
	bool result = m_pcap.setFilter ((const char*) filter.m_p);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_filter = filter;
	return true;
}

bool
AXL_CDECL
PCap::openDevice (
	DataPtr deviceName,
	DataPtr filter,
	bool isPromiscious
	)
{
	bool result;

	close ();

	result =
		m_pcap.openDevice ((const char*) deviceName.m_p, DefKind_SnapshotSize, isPromiscious, 200) &&
		m_pcap.setFilter ((const char*) filter.m_p);

	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_filter = filter;
	m_isPromiscious = isPromiscious;
	m_isOpen = true;

	m_ioThreadEvent.reset ();
	m_ioFlags = 0;
	m_ioThread.start ();
	return true;
}

bool
AXL_CDECL
PCap::openFile (
	DataPtr fileName,
	DataPtr filter
	)
{
	bool result;

	close ();

	result =
		m_pcap.openFile ((const char*) fileName.m_p) &&
		m_pcap.setFilter ((const char*) filter.m_p);

	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_filter = filter;
	m_isPromiscious = false;
	m_isOpen = true;

	m_ioThreadEvent.reset ();
	m_ioFlags = IoFlag_File;
	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
PCap::close ()
{
	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);

	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId++;
}

size_t
AXL_CDECL
PCap::read (
	DataPtr ptr,
	size_t size
	)
{
	Read read;
	read.m_p = ptr.m_p;
	read.m_size = size;

	m_ioLock.lock ();
	if (m_ioFlags & IoFlag_Eof)
	{
		m_ioLock.unlock ();
		return 0;
	}

	m_readList.insertTail (&read);
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	read.m_completeEvent.wait ();
	rt::leaveWaitRegion (m_runtime);

	if (read.m_result == -1)
		ext::setError (read.m_error);

	return read.m_result;
}

void
PCap::ioThreadFunc ()
{
	for (;;)
	{
		// wait for packet

		char readBuffer [DefKind_SnapshotSize];
		size_t readResult;
		err::Error readError;

		for (;;)
		{
			readResult = m_pcap.read (readBuffer, sizeof (readBuffer));

			m_ioLock.lock ();
			if (m_ioFlags & IoFlag_Closing)
			{
				cancelAllReads_l ();
				return;
			}

			if (readResult == -2)
			{
				m_ioFlags |= IoFlag_Eof;
				cancelAllReads_l ();
				firePCapEvent (PCapEventKind_Eof);
				return;
			}

			m_ioLock.unlock ();

			if (readResult != 0) // not a timeout
			{
				if (readResult == -1)
					readError = err::getLastError ();
				break;
			}
		}

		// wait for read request

		m_ioLock.lock ();
		if (m_readList.isEmpty ())
		{
			m_ioLock.unlock ();
			firePCapEvent (PCapEventKind_ReadyRead);
			m_ioLock.lock ();
		}

		while (m_readList.isEmpty ())
		{
			m_ioLock.unlock ();
			m_ioThreadEvent.wait ();

			m_ioLock.lock ();
			if (m_ioFlags & IoFlag_Closing)
			{
				cancelAllReads_l ();
				return;
			}
		}

		// complete read request

		Read* read = m_readList.removeHead ();
		m_ioLock.unlock ();

		if (readResult != -1)
		{
			readResult = AXL_MIN (readResult, read->m_size);
			memcpy (read->m_p, readBuffer, readResult);
		}

		read->m_result = readResult;
		read->m_error = readError;
		read->m_completeEvent.signal ();
	}
}

void
PCap::cancelAllReads_l ()
{
	sl::AuxList <Read> readList;
	readList.takeOver (&m_readList);
	m_ioLock.unlock ();

	while (!readList.isEmpty ())
	{
		Read* read = m_readList.removeHead ();
		read->m_result = -1;
		read->m_error = err::SystemErrorCode_Cancelled;
		read->m_completeEvent.signal ();
	}
}

//.............................................................................

inline
uint32_t
getIpFromSockAddr (const sockaddr* sockAddr)
{
	return sockAddr && sockAddr->sa_family == AF_INET ?
#if (_AXL_ENV == AXL_ENV_WIN)
		((const sockaddr_in*) sockAddr)->sin_addr.S_un.S_addr :
#elif (_AXL_ENV == AXL_ENV_POSIX)
		((const sockaddr_in*) sockAddr)->sin_addr.s_addr :
#endif
		0;
}

void
setupPCapAddress (
	rt::Runtime* runtime,
	PCapAddress* address,
	const pcap_addr* ifaceAddr
	)
{
	for (; ifaceAddr; ifaceAddr = ifaceAddr->next)
		if (ifaceAddr->addr && ifaceAddr->addr->sa_family == AF_INET)
			break;

	if (!ifaceAddr)
		return; // no IP4 addresses found

	rt::Type* addressType = PCapAddress::getType (runtime);

	address->m_address = getIpFromSockAddr (ifaceAddr->addr);
	address->m_mask = getIpFromSockAddr (ifaceAddr->netmask);
	address->m_broadcast = getIpFromSockAddr (ifaceAddr->broadaddr);

	PCapAddress* prevAddress = address;
	for (ifaceAddr = ifaceAddr->next; ifaceAddr; ifaceAddr = ifaceAddr->next)
	{
		if (!ifaceAddr->addr || ifaceAddr->addr->sa_family != AF_INET)
			continue;

		DataPtr addressPtr = rt::allocateData (runtime, addressType);
		address = (PCapAddress*) addressPtr.m_p;
		address->m_address = getIpFromSockAddr (ifaceAddr->addr);
		address->m_mask = getIpFromSockAddr (ifaceAddr->netmask);
		address->m_broadcast = getIpFromSockAddr (ifaceAddr->broadaddr);

		prevAddress->m_nextPtr = addressPtr;
		prevAddress = address;
	}
}

DataPtr
createPCapDeviceDescList (DataPtr countPtr)
{
	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = 0;

	pcap_if* ifaceList = NULL;
	char errorBuffer [PCAP_ERRBUF_SIZE] = { 0 };
	int result = pcap_findalldevs (&ifaceList, errorBuffer);
	if (result == -1)
	{
		ext::setError (errorBuffer);
		return g_nullPtr;
	}

	if (!ifaceList)
		return g_nullPtr;

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	rt::ScopedNoCollectRegion noCollectRegion (runtime, false);

	rt::Type* deviceType = PCapDeviceDesc::getType (runtime);

	size_t count = 1;

	pcap_if* iface = ifaceList;

	DataPtr devicePtr = rt::allocateData (runtime, deviceType);
	PCapDeviceDesc* device = (PCapDeviceDesc*) devicePtr.m_p;
	device->m_namePtr = rt::strDup (iface->name);
	device->m_descriptionPtr = rt::strDup (iface->description);
	setupPCapAddress (runtime, &device->m_address, iface->addresses);

	DataPtr resultPtr = devicePtr;

	PCapDeviceDesc* prevDevice = device;
	for (iface = iface->next; iface; iface = iface->next, count++)
	{
		devicePtr = rt::allocateData (runtime, deviceType);
		device = (PCapDeviceDesc*) devicePtr.m_p;
		device->m_namePtr = rt::strDup (iface->name);
		device->m_descriptionPtr = rt::strDup (iface->description);
		setupPCapAddress (runtime, &device->m_address, iface->addresses);

		prevDevice->m_nextPtr = devicePtr;
		prevDevice = device;
	}

	pcap_freealldevs (ifaceList);

	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = count;

	return resultPtr;
}

//.............................................................................

} // namespace io
} // namespace jnc
