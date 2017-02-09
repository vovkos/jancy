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
#include "jnc_io_UsbEndpoint.h"
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDevice.h"
#include "jnc_io_UsbDesc.h"
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	UsbEndpointEventParams,
	"io.UsbEndpointEventParams",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpointEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEndpointEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	UsbEndpoint,
	"io.UsbEndpoint",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpoint,
	UsbEndpoint,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEndpoint)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <UsbEndpoint>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <UsbEndpoint>)
	JNC_MAP_FUNCTION ("close", &UsbEndpoint::close)
	JNC_MAP_FUNCTION ("write", &UsbEndpoint::write)
	JNC_MAP_FUNCTION ("read",  &UsbEndpoint::read)
	JNC_MAP_PROPERTY ("m_isEndpointEventEnabled", &UsbEndpoint::isEndpointEventEnabled, &UsbEndpoint::setEndpointEventEnabled)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

UsbEndpoint::UsbEndpoint ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_isOpen = true;
	m_syncId = 0;
	m_ioFlags = IoFlag_EndpointEventDisabled;
	m_incomingQueueLimit = 16 * 1024;
}

void
UsbEndpoint::fireEndpointEvent (
	UsbEndpointEventCode eventCode,
	const err::ErrorHdr* error
	)
{
	m_ioLock.lock ();
	if (!(m_ioFlags & IoFlag_EndpointEventDisabled))
	{
		m_ioLock.unlock ();
		fireEndpointEventImpl (eventCode, error);
	}
	else
	{
		PendingEvent* pendingEvent = AXL_MEM_NEW (PendingEvent);
		pendingEvent->m_eventCode = eventCode;
		pendingEvent->m_error = error;
		m_pendingEventList.insertTail (pendingEvent);
		m_ioLock.unlock ();
	}
}

void
UsbEndpoint::fireEndpointEventImpl (
	UsbEndpointEventCode eventCode,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE (m_runtime);

	DataPtr paramsPtr = createData <UsbEndpointEventParams> (m_runtime);
	UsbEndpointEventParams* params = (UsbEndpointEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_runtime, m_onEndpointEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

void
JNC_CDECL
UsbEndpoint::setEndpointEventEnabled (bool isEnabled)
{
	m_ioLock.lock ();
	if (!isEnabled)
	{
		if (!(m_ioFlags & IoFlag_EndpointEventDisabled))
			m_ioFlags |= IoFlag_EndpointEventDisabled;

		m_ioLock.unlock ();
		return;
	}

	if (!(m_ioFlags & IoFlag_EndpointEventDisabled))
	{
		m_ioLock.unlock ();
		return;
	}

	while (!m_pendingEventList.isEmpty  ())
	{
		PendingEvent* pendingEvent = m_pendingEventList.removeHead ();
		m_ioLock.unlock ();

		fireEndpointEventImpl (pendingEvent->m_eventCode, pendingEvent->m_error);
		AXL_MEM_DELETE (pendingEvent);

		m_ioLock.lock ();
	}

	m_ioFlags &= ~IoFlag_EndpointEventDisabled;
	m_ioLock.unlock ();
}

void
JNC_CDECL
UsbEndpoint::close ()
{
	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;

	if (m_ioFlags & IoFlag_Reading)
	{
		m_ioLock.unlock ();
		m_readTransfer.cancel ();
		m_readTransferCompleted.wait ();
	}

	m_isOpen = false;
	m_syncId++;
}

bool
UsbEndpoint::startRead ()
{
	UsbEndpointDesc* desc = (UsbEndpointDesc*) m_endpointDescPtr.m_p;
	ASSERT (desc->m_endpointId & LIBUSB_ENDPOINT_IN);

	bool result =
		m_readTransfer.create () &&
		m_readBuffer.setCount (desc->m_maxPacketSize);

	if (!result)
	{
		jnc::propagateLastError ();
		return false;
	}

	axl::io::UsbDevice* device = m_parentInterface->m_parentDevice->getDevice ();
	switch (desc->m_transferType)
	{
	case LIBUSB_TRANSFER_TYPE_BULK:
		m_readTransfer.fillBulkTransfer (
			device->getOpenHandle (),
			desc->m_endpointId,
			m_readBuffer,
			desc->m_maxPacketSize,
			onReadTransferCompleted,
			this
			);
		break;

	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		m_readTransfer.fillInterruptTransfer (
			device->getOpenHandle (),
			desc->m_endpointId,
			m_readBuffer,
			desc->m_maxPacketSize,
			onReadTransferCompleted,
			this
			);
		break;

	case LIBUSB_TRANSFER_TYPE_CONTROL:
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
	case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
		// not yet, fall through...

	default:
		err::setError (err::SystemErrorCode_NotImplemented);
		jnc::propagateLastError ();
		return false;
	}

	m_ioLock.lock ();
	result = nextReadTransfer_l ();
	if (!result)
		jnc::propagateLastError ();

	return result;
}

size_t
JNC_CDECL
UsbEndpoint::read (
	DataPtr ptr,
	size_t size
	)
{
	UsbEndpointDesc* desc = (UsbEndpointDesc*) m_endpointDescPtr.m_p;
	if (!(desc->m_endpointId & LIBUSB_ENDPOINT_IN))
	{
		err::setError ("Cannot read from a USB OUT-endpoint");
		jnc::propagateLastError ();
		return -1;
	}

	m_ioLock.lock ();
	if (!m_incomingPacketList.isEmpty ())
	{
		Packet* packet = m_incomingPacketList.removeHead ();
		ASSERT (m_totalIncomingPacketSize >= packet->m_size);
		m_totalIncomingPacketSize -= packet->m_size;

		size_t copySize = AXL_MIN (size, packet->m_size);
		memcpy (ptr.m_p, packet + 1, copySize);
		AXL_MEM_DELETE (packet);

		if (m_totalIncomingPacketSize <= m_incomingQueueLimit &&
			!(m_ioFlags & (IoFlag_Reading | IoFlag_Closing | IoFlag_Error)))
			nextReadTransfer_l ();
		else
			m_ioLock.unlock ();

		return copySize;
	}

	Read read;
	read.m_p = ptr.m_p;
	read.m_size = size;
	m_readList.insertTail (&read);
	m_ioLock.unlock ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	gcHeap->enterWaitRegion ();
	read.m_completeEvent.wait ();
	gcHeap->leaveWaitRegion ();

	if (read.m_result == -1)
	{
		setError (read.m_error);
		jnc::propagateLastError ();
	}

	return read.m_result;
}

size_t
JNC_CDECL
UsbEndpoint::write (
	DataPtr ptr,
	size_t size,
	uint_t timeout
	)
{
	UsbEndpointDesc* desc = (UsbEndpointDesc*) m_endpointDescPtr.m_p;
	if (desc->m_endpointId & LIBUSB_ENDPOINT_IN)
	{
		err::setError ("Cannot write to a USB IN-endpoint");
		jnc::propagateLastError ();
		return -1;
	}

	size_t packetCount = size / desc->m_maxPacketSize;
	size_t lastPacketSize = size % desc->m_maxPacketSize;
	const char* p = (char*) ptr.m_p;
	const char* end = p + packetCount * desc->m_maxPacketSize;
	for (; p < end; p += desc->m_maxPacketSize)
	{
		size_t result = writePacket (p, desc->m_maxPacketSize, timeout);
		if (result == -1)
		{
			jnc::propagateLastError ();
			return -1;
		}
	}

	if (lastPacketSize)
	{
		size_t result = writePacket (p, lastPacketSize, timeout);
		if (result == -1)
		{
			jnc::propagateLastError ();
			return -1;
		}
	}

	return size;
}

size_t
UsbEndpoint::writePacket (
	const void* p,
	size_t size,
	uint_t timeout
	)
{
	UsbEndpointDesc* desc = (UsbEndpointDesc*) m_endpointDescPtr.m_p;
	ASSERT (!(desc->m_endpointId & LIBUSB_ENDPOINT_IN));
	ASSERT (size <= desc->m_maxPacketSize);

	axl::io::UsbDevice* device = m_parentInterface->m_parentDevice->getDevice ();
	switch (desc->m_transferType)
	{
	case LIBUSB_TRANSFER_TYPE_BULK:
		return device->bulkTransfer (desc->m_endpointId, (void*) p, size, timeout);

	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		return device->interruptTransfer (desc->m_endpointId, (void*) p, size, timeout);

	case LIBUSB_TRANSFER_TYPE_CONTROL:
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
	case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
		// not yet, fall through...

	default:
		err::setError (err::SystemErrorCode_NotImplemented);
		return -1;
	}
}

bool
UsbEndpoint::nextReadTransfer_l ()
{
	ASSERT (!(m_ioFlags & IoFlag_Reading));
	ASSERT (m_totalIncomingPacketSize <= m_incomingQueueLimit);

	UsbEndpointDesc* desc = (UsbEndpointDesc*) m_endpointDescPtr.m_p;
	ASSERT (desc->m_endpointId & LIBUSB_ENDPOINT_IN);
	m_readTransferCompleted.reset ();
	m_ioFlags |= IoFlag_Reading;
	m_ioLock.unlock ();

	bool result = m_readTransfer.submit ();
	if (!result)
	{
		m_ioLock.lock ();
		m_ioFlags &= ~IoFlag_Reading;
		m_ioLock.unlock ();
		return false;
	}

	return true;
}

void
LIBUSB_CALL
UsbEndpoint::onReadTransferCompleted (libusb_transfer* transfer)
{
	UsbEndpoint* self = (UsbEndpoint*) transfer->user_data;
	self->m_ioLock.lock ();
	self->m_ioFlags &= ~IoFlag_Reading;

	bool result = transfer->status == LIBUSB_TRANSFER_COMPLETED;
	if (result)
	{
		Packet* packet = AXL_MEM_NEW_EXTRA (Packet, transfer->actual_length);
		packet->m_size = transfer->actual_length;
		memcpy (packet + 1, self->m_readBuffer, transfer->actual_length);

		self->m_incomingPacketList.insertTail (packet);
		self->m_totalIncomingPacketSize += transfer->actual_length;
	}

	if (self->m_totalIncomingPacketSize <= self->m_incomingQueueLimit &&
		!(self->m_ioFlags & (IoFlag_Reading | IoFlag_Closing | IoFlag_Error)))
	{
		self->nextReadTransfer_l ();
	}
	else
	{
		self->m_readTransferCompleted.signal ();
		self->m_ioLock.unlock ();
	}

	if (result)
		self->fireEndpointEvent (UsbEndpointEventCode_ReadyRead);
	else
		self->fireEndpointEvent (UsbEndpointEventCode_IoError, axl::io::UsbError (LIBUSB_ERROR_IO));
}

//..............................................................................

} // namespace io
} // namespace jnc
