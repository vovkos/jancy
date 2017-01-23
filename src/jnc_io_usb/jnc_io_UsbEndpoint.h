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

namespace jnc {
namespace io {

class UsbInterface;

JNC_DECLARE_TYPE (UsbEndpointEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (UsbEndpoint)

//..............................................................................

enum UsbEndpointEventCode
{
	UsbEndpointEventCode_ReadyRead = 0,
	UsbEndpointEventCode_IoError,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct UsbEndpointEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (UsbEndpointEventParams)

	UsbEndpointEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//..............................................................................

class UsbEndpoint: public IfaceHdr
{
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (UsbEndpoint)

protected:
	enum IoFlag
	{
		IoFlag_Closing               = 0x0001,
		IoFlag_Reading               = 0x0002,
		IoFlag_Error                 = 0x0004,
		IoFlag_EndpointEventDisabled = 0x0008,
	};

	struct Read: sl::ListLink
	{
		void* m_p;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completeEvent;
	};

	struct Packet: sl::ListLink
	{
		size_t m_size;
		// followed by packet data
	};

	struct PendingEvent: sl::ListLink
	{
		UsbEndpointEventCode m_eventCode;
		err::Error m_error;
	};

public:
	UsbInterface* m_parentInterface;
	DataPtr m_endpointDescPtr;
	size_t m_incomingQueueLimit;
	bool m_isOpen;
	uint_t m_syncId;
	ClassBox <Multicast> m_onEndpointEvent;

protected:
	Runtime* m_runtime;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	sl::AuxList <Read> m_readList;
	sl::StdList <Packet> m_incomingPacketList;
	sl::StdList <PendingEvent> m_pendingEventList;
	size_t m_totalIncomingPacketSize;
	axl::io::UsbTransfer m_readTransfer;
	sl::Array <char> m_readBuffer;
	axl::sys::NotificationEvent m_readTransferCompleted;

public:
	UsbEndpoint ();

	~UsbEndpoint ()
	{
		close ();
	}

	void
	JNC_CDECL
	close ();

	bool
	startRead ();

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		);

	bool
	JNC_CDECL
	isEndpointEventEnabled ()
	{
		return !(m_ioFlags & IoFlag_EndpointEventDisabled);
	}

	void
	JNC_CDECL
	setEndpointEventEnabled (bool isEnabled);

protected:
	void
	fireEndpointEvent (
		UsbEndpointEventCode eventCode,
		const err::ErrorHdr* error = NULL
		);

	void
	fireEndpointEventImpl (
		UsbEndpointEventCode eventCode,
		const err::ErrorHdr* error
		);

	size_t
	writePacket (
		const void* p,
		size_t size // must be <= m_endpointDesc->m_maxPacketSize
		);

	bool
	nextReadTransfer_l ();

	static
	void
	LIBUSB_CALL
	onReadTransferCompleted (libusb_transfer* transfer);
};

//..............................................................................

} // namespace io
} // namespace jnc
