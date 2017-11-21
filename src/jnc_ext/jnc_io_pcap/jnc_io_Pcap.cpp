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

JNC_DEFINE_TYPE (
	PcapEventParams,
	"io.PcapEventParams",
	g_pcapLibGuid,
	PcapLibCacheSlot_PcapEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (PcapEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	JNC_MAP_FUNCTION ("openDevice",  &Pcap::openDevice)
	JNC_MAP_FUNCTION ("openFile",    &Pcap::openFile)
	JNC_MAP_FUNCTION ("close",       &Pcap::close)
	JNC_MAP_FUNCTION ("setFilter",   &Pcap::setFilter)
	JNC_MAP_FUNCTION ("write",       &Pcap::write)
	JNC_MAP_FUNCTION ("read",        &Pcap::read)
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
	m_runtime = getCurrentThreadRuntime ();
	m_ioFlags = 0;
	memset (&m_filter, 0, sizeof (m_filter));
	m_isPromiscious = false;
	m_isOpen = false;
	m_syncId = 0;
}

void
Pcap::firePcapEvent (
	PcapEventCode eventCode,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE (m_runtime);

	DataPtr paramsPtr = createData <PcapEventParams> (m_runtime);
	PcapEventParams* params = (PcapEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_onPcapEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
JNC_CDECL
Pcap::setFilter (DataPtr filter)
{
	bool result = m_pcap.setFilter ((const char*) filter.m_p);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_filter = filter;
	return true;
}

bool
JNC_CDECL
Pcap::openDevice (
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
		propagateLastError ();
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
JNC_CDECL
Pcap::openFile (
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
		propagateLastError ();
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
JNC_CDECL
Pcap::close ()
{
	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId++;
}

size_t
JNC_CDECL
Pcap::read (
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

	if (m_ioFlags & IoFlag_IoError)
	{
		m_ioLock.unlock ();
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return -1;
	}

	m_readList.insertTail (&read);
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	read.m_completeEvent.wait ();
	gcHeap->leaveWaitRegion ();

	if (read.m_result == -1)
		err::setError (err::SystemErrorCode_Cancelled);

	return read.m_result;
}

void
Pcap::ioThreadFunc ()
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
			if (readResult == -1)
				readError = err::getLastError ();

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
				firePcapEvent (PcapEventCode_Eof);
				return;
			}

			m_ioLock.unlock ();

			if (readResult != 0) // not a timeout
			{
				if (readResult == -1)
				{
					m_ioFlags |= IoFlag_IoError;
					cancelAllReads_l ();
					firePcapEvent (PcapEventCode_IoError, readError);
					return;
				}

				break;
			}
		}

		// wait for read request

		m_ioLock.lock ();
		if (m_readList.isEmpty ())
		{
			m_ioLock.unlock ();
			firePcapEvent (PcapEventCode_ReadyRead);
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
		read->m_completeEvent.signal ();
	}
}

void
Pcap::cancelAllReads_l ()
{
	sl::AuxList <Read> readList;
	readList.takeOver (&m_readList);
	m_ioLock.unlock ();

	while (!readList.isEmpty ())
	{
		Read* read = m_readList.removeHead ();
		read->m_result = -1;
		read->m_completeEvent.signal ();
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
