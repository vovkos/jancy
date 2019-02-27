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
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	UsbEndpoint,
	"io.UsbEndpoint",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpoint,
	UsbEndpoint,
	&UsbEndpoint::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbEndpoint)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<UsbEndpoint>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<UsbEndpoint>)

	JNC_MAP_AUTOGET_PROPERTY("m_transferTimeout", &UsbEndpoint::setTransferTimeout)
	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism", &UsbEndpoint::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &UsbEndpoint::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &UsbEndpoint::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &UsbEndpoint::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &UsbEndpoint::setOptions)

	JNC_MAP_FUNCTION("close",        &UsbEndpoint::close)
	JNC_MAP_FUNCTION("write",        &UsbEndpoint::write)
	JNC_MAP_FUNCTION("read",         &UsbEndpoint::read)
	JNC_MAP_FUNCTION("wait",         &UsbEndpoint::wait)
	JNC_MAP_FUNCTION("cancelWait",   &UsbEndpoint::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &UsbEndpoint::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbEndpoint::UsbEndpoint()
{
	m_transferTimeout = Def_TransferTimeout;
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

bool
UsbEndpoint::open()
{
	AsyncIoDevice::open();
	wakeIoThread();
	return m_ioThread.start();
}

void
JNC_CDECL
UsbEndpoint::close()
{
	if (!m_isOpen)
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	AsyncIoDevice::close();
}

size_t
JNC_CDECL
UsbEndpoint::read(
	DataPtr ptr,
	size_t size
	)
{
	if (isOutEndpoint())
	{
		err::setError("Cannot read from a USB OUT-endpoint");
		return -1;
	}

	return bufferedRead(ptr, size);
}

size_t
JNC_CDECL
UsbEndpoint::write(
	DataPtr ptr,
	size_t size
	)
{
	if (isInEndpoint())
	{
		err::setError("Cannot write to a USB IN-endpoint");
		return -1;
	}

	size_t result = bufferedWrite(ptr, size);

	// clear write-completed event

	AXL_TODO("combine into a single atomic operation (need to implement AsyncIoDevice::bufferWriteImpl_l)")

	m_lock.lock();

	if (!m_writeBuffer.isEmpty() || !m_activeTransferList.isEmpty())
		m_activeEvents &= ~UsbEndpointEvent_WriteCompleted;

	m_lock.unlock();

	return result;
}

void
UsbEndpoint::ioThreadFunc()
{
	if (isInEndpoint())
		readLoop();
	else
		writeLoop();

	cancelAllActiveTransfers();
}

void
UsbEndpoint::cancelAllActiveTransfers()
{
	char buffer[256];
	sl::Array<Transfer*> activeTransferArray(ref::BufKind_Stack, buffer, sizeof(buffer));

	m_lock.lock();

	sl::Iterator<Transfer> it = m_activeTransferList.getHead();
	for (; it; it++)
		activeTransferArray.append(*it);

	m_lock.unlock();

	size_t count = activeTransferArray.getCount();
	for (size_t i = 0; i < count; i++)
		activeTransferArray[i]->m_usbTransfer.cancel(); // may fail if already completed

	// wait for all transfers to complete

	m_lock.lock();

	while (!m_activeTransferList.isEmpty())
	{
		m_lock.unlock();
		sleepIoThread();
		m_lock.lock();
	}

	// put all completed transfer back to the free pool

	while (!m_completedTransferList.isEmpty())
	{
		Transfer* transfer = m_completedTransferList.removeHead();
		m_transferPool.put(transfer);
	}

	m_lock.unlock();
}

void
UsbEndpoint::readLoop()
{
	ASSERT(isInEndpoint());

	bool result;

	UsbEndpointDesc* desc = (UsbEndpointDesc*)m_endpointDescPtr.m_p;

	for (;;)
	{
		sleepIoThread();

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			break;
		}

		while (!m_completedTransferList.isEmpty())
		{
			Transfer* transfer = m_completedTransferList.removeHead();

			if (transfer->m_usbTransfer->status != LIBUSB_TRANSFER_COMPLETED &&
				transfer->m_usbTransfer->status != LIBUSB_TRANSFER_TIMED_OUT && // <- not really an error
				transfer->m_usbTransfer->status != LIBUSB_TRANSFER_CANCELLED    // <- not really an error
				)
			{
				m_transferPool.put(transfer);
				setIoErrorEvent_l(axl::io::UsbError(LIBUSB_ERROR_IO));
				return;
			}

			addToReadBuffer(transfer->m_buffer, transfer->m_usbTransfer->actual_length);
			m_transferPool.put(transfer);
		}

		size_t activeReadCount = m_activeTransferList.getCount();
		if (!m_readBuffer.isFull() && activeReadCount < m_readParallelism)
		{
			UsbEndpointDesc* endpointDesc = (UsbEndpointDesc*)m_endpointDescPtr.m_p;
			size_t readBlockSize = m_readBlockSize;
			size_t newReadCount = m_readParallelism - activeReadCount;
			uint_t timeout = m_transferTimeout;

			for (size_t i = 0; i < newReadCount; i++)
			{
				Transfer* transfer = m_transferPool.get();
				result = transfer->m_buffer.setCount(readBlockSize);
				if (!result)
				{
					m_transferPool.put(transfer);
					setIoErrorEvent_l();
					return;
				}

				transfer->m_isCompletedOutOfOrder = false;
				m_activeTransferList.insertTail(transfer); // put it on active list prior to submission
				m_lock.unlock();

				result = submitTransfer(transfer, transfer->m_buffer, transfer->m_buffer.getCount(), timeout);

				m_lock.lock();

				if (!result)
				{
					m_activeTransferList.remove(transfer);
					m_transferPool.put(transfer);
					setIoErrorEvent_l();
					return;
				}
			}
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

void
UsbEndpoint::writeLoop()
{
	ASSERT(isOutEndpoint());

	bool result;

	for (;;)
	{
		sleepIoThread();

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			break;
		}

		if (!m_completedTransferList.isEmpty())
		{
			Transfer* transfer = m_completedTransferList.removeHead();

			if (transfer->m_usbTransfer->status != LIBUSB_TRANSFER_COMPLETED)
			{
				m_transferPool.put(transfer);
				setIoErrorEvent_l(axl::io::UsbError(LIBUSB_ERROR_IO));
				break;
			}

			if ((size_t)transfer->m_usbTransfer->actual_length < m_writeBlock.getCount()) // shouldn't happen, actually
				m_writeBlock.remove(0, transfer->m_usbTransfer->actual_length);
			else
				m_writeBlock.clear();

			m_transferPool.put(transfer);
			ASSERT(m_completedTransferList.isEmpty()); // we never submit more than one
		}

		getNextWriteBlock(&m_writeBlock);

		if (m_activeTransferList.isEmpty() && !m_writeBlock.isEmpty())
		{
			uint_t timeout = m_transferTimeout;

			Transfer* transfer = m_transferPool.get();
			transfer->m_isCompletedOutOfOrder = false;
			m_activeTransferList.insertTail(transfer); // put it on active list prior to submission
			m_lock.unlock();

			result = submitTransfer(transfer, m_writeBlock, m_writeBlock.getCount(), timeout);

			m_lock.lock();

			if (!result)
			{
				m_activeTransferList.remove(transfer);
				m_transferPool.put(transfer);
				setIoErrorEvent_l();
				break;
			}
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		if (m_writeBuffer.isEmpty() && m_activeTransferList.isEmpty())
			m_activeEvents |= UsbEndpointEvent_WriteCompleted;
		else
			m_activeEvents &= ~UsbEndpointEvent_WriteCompleted;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

bool
UsbEndpoint::submitTransfer(
	Transfer* transfer,
	void* p,
	size_t size,
	uint_t timeout
	)
{
	UsbEndpointDesc* desc = (UsbEndpointDesc*)m_endpointDescPtr.m_p;

	bool result = transfer->m_usbTransfer.create();
	if (!result)
		return false;

	axl::io::UsbDevice* device = m_parentInterface->m_parentDevice->getDevice();
	switch(desc->m_transferType)
	{
	case LIBUSB_TRANSFER_TYPE_BULK:
		transfer->m_usbTransfer.fillBulkTransfer(
			device->getOpenHandle(),
			desc->m_endpointId,
			p,
			size,
			onTransferCompleted,
			transfer,
			timeout
			);

		break;

	case LIBUSB_TRANSFER_TYPE_INTERRUPT:
		transfer->m_usbTransfer.fillInterruptTransfer(
			device->getOpenHandle(),
			desc->m_endpointId,
			p,
			size,
			onTransferCompleted,
			transfer,
			timeout
			);

		break;

	case LIBUSB_TRANSFER_TYPE_CONTROL:
	case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
	case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
		// not yet, fall through...

	default:
		err::setError(err::SystemErrorCode_NotImplemented);
		return false;
	}

	transfer->m_self = this;

	return transfer->m_usbTransfer.submit();
}

void
LIBUSB_CALL
UsbEndpoint::onTransferCompleted(libusb_transfer* usbTransfer)
{
	Transfer* transfer = (Transfer*)usbTransfer->user_data;
	ASSERT(transfer->m_usbTransfer == usbTransfer);

	UsbEndpoint* self = transfer->m_self;

	self->m_lock.lock();

	// surprisingly enough, libusb may deliver completions out-of-order

	if (transfer != *self->m_activeTransferList.getHead())
	{
		transfer->m_isCompletedOutOfOrder = true;
	}
	else
	{
		do
		{
			self->m_activeTransferList.remove(transfer);
			self->m_completedTransferList.insertTail(transfer);
			transfer = *self->m_activeTransferList.getHead();
		} while (transfer && transfer->m_isCompletedOutOfOrder);

		self->wakeIoThread();
	}

	self->m_lock.unlock();
}

//..............................................................................

} // namespace io
} // namespace jnc
