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

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_io_UsbDesc.h"

namespace jnc {
namespace io {

class UsbInterface;

JNC_DECLARE_OPAQUE_CLASS_TYPE (UsbEndpoint)

//..............................................................................

struct UsbEndpointHdr: IfaceHdr
{
	UsbInterface* m_parentInterface;
	DataPtr m_endpointDescPtr;

	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class UsbEndpoint:
	public UsbEndpointHdr,
	public AsyncIoDevice
{
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (UsbEndpoint)

protected:
	enum Def
	{
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 1 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = AsyncIoOption_KeepReadBlockSize | AsyncIoOption_KeepWriteBlockSize,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, UsbEndpoint, m_ioThread)->ioThreadFunc ();
		}
	};

	struct Transfer: sl::ListLink
	{
		UsbEndpoint* m_self;
		axl::io::UsbTransfer m_usbTransfer;
		sl::Array <char> m_buffer;
	};

protected:
	IoThread m_ioThread;

	mem::Pool <Transfer> m_transferPool;
	sl::StdList <Transfer> m_activeTransferList;
	sl::StdList <Transfer> m_completedTransferList;
	sl::Array <char> m_writeBlock;

public:
	UsbEndpoint ();

	~UsbEndpoint ()
	{
		close ();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots (gcHeap);
	}

	bool
	startIoThread ()
	{
		return m_ioThread.start ();
	}

	void
	JNC_CDECL
	close ();

	void
	JNC_CDECL
	setReadParallelism (uint_t count)
	{
		AsyncIoDevice::setSetting (&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	void
	JNC_CDECL
	setReadBlockSize (size_t size)
	{
		AsyncIoDevice::setSetting (&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize (size_t size)
	{
		return AsyncIoDevice::setReadBufferSize (&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize (size_t size)
	{
		return AsyncIoDevice::setWriteBufferSize (&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	void
	JNC_CDECL
	setOptions (uint_t options)
	{
		AsyncIoDevice::setSetting (&m_options, options);
	}

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

	handle_t
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait (eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait (handle_t handle)
	{
		return AsyncIoDevice::cancelWait (handle);
	}

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait (eventMask, timeout);
	}

protected:
	bool
	isInEndpoint ()
	{
		return (((UsbEndpointDesc*) m_endpointDescPtr.m_p)->m_endpointId & LIBUSB_ENDPOINT_IN) != 0;
	}

	bool
	isOutEndpoint ()
	{
		return (((UsbEndpointDesc*) m_endpointDescPtr.m_p)->m_endpointId & LIBUSB_ENDPOINT_IN) == 0;
	}

	void
	ioThreadFunc ();

	void
	cancelAllActiveTransfers ();

	void
	readLoop ();

	void
	writeLoop ();

	bool
	submitTransfer (
		Transfer* transfer,
		void* p,
		size_t size
		);

	static
	void
	LIBUSB_CALL
	onTransferCompleted (libusb_transfer* transfer);
};

//..............................................................................

} // namespace io
} // namespace jnc
