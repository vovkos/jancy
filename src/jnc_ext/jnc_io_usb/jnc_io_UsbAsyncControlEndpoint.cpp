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
#include "jnc_io_UsbAsyncControlEndpoint.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

UsbAsyncControlEndpoint::UsbAsyncControlEndpoint (axl::io::UsbDevice* device)
{
	m_runtime = getCurrentThreadRuntime ();
	ASSERT (m_runtime);

	m_device = device;
	m_flags = 0;
}

void
UsbAsyncControlEndpoint::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	sl::Iterator <Transfer> it = m_activeTransferList.getHead ();
	for (; it; it++)
		markTransferGcRoots (gcHeap, *it);

	it = m_completedTransferList.getHead ();
	for (; it; it++)
		markTransferGcRoots (gcHeap, *it);
}

void
UsbAsyncControlEndpoint::markTransferGcRoots (
	GcHeap* gcHeap,
	Transfer* transfer
	)
{
	gcHeap->markClass (transfer->m_completionFuncPtr.m_closure->m_box);

	if (transfer->m_inBufferPtr.m_validator)
	{
		gcHeap->weakMark (transfer->m_inBufferPtr.m_validator->m_validatorBox);
		gcHeap->markData (transfer->m_inBufferPtr.m_validator->m_targetBox);
	}
}

bool
UsbAsyncControlEndpoint::start ()
{
	ASSERT (isIdle ());

	m_flags = 0;
	m_idleEvent.signal ();

	return m_completionThread.start ();
}

void
UsbAsyncControlEndpoint::stop ()
{
	m_lock.lock ();
	m_flags |= Flag_Stop;
	m_event.signal ();
	m_lock.unlock ();

	m_completionThread.waitAndClose ();
}

bool
JNC_CDECL
UsbAsyncControlEndpoint::transfer (
	uint_t requestType,
	uint_t requestCode,
	uint_t value,
	uint_t index,
	DataPtr ptr,
	size_t size,
	uint_t timeout,
	FunctionPtr completionFuncPtr
	)
{
	bool result;

	m_lock.lock ();
	ASSERT (!m_flags & Flag_Stop);
	Transfer* transfer = m_transferPool.get ();
	m_lock.unlock ();

	result = transfer->m_usbTransfer.create ();
	libusb_control_setup* setup = transfer->m_buffer.createBuffer (sizeof (libusb_control_setup) + size);

	if (!result || !setup)
	{
		m_lock.lock ();
		m_transferPool.put (transfer);
		m_lock.unlock ();
		callCompletionFunc (completionFuncPtr, -1, err::getLastError ());
		return false;
	}

	transfer->m_usbTransfer.fillControlSetup (
		setup,
		requestType,
		requestCode,
		value,
		index,
		size
		);

	transfer->m_usbTransfer.fillControlTransfer (
		m_device->getOpenHandle (),
		setup,
		onTransferCompleted,
		transfer,
		timeout
		);

	if (requestType & LIBUSB_ENDPOINT_IN)
	{
		transfer->m_inBufferPtr = ptr;
	}
	else
	{
		transfer->m_inBufferPtr = g_nullPtr;
		memcpy (setup + 1, ptr.m_p, size);
	}

	transfer->m_completionFuncPtr = completionFuncPtr;

	m_lock.lock ();

	if (isIdle ())
		m_idleEvent.reset ();

	m_activeTransferList.insertTail (transfer);
	m_lock.unlock ();

	transfer->m_self = this;

	result = transfer->m_usbTransfer.submit ();
	if (result)
		return true;

	m_lock.lock ();
	m_activeTransferList.remove (transfer);
	m_transferPool.put (transfer);
	m_lock.unlock ();
	callCompletionFunc (completionFuncPtr, -1, err::getLastError ());
	return false;
}

void
UsbAsyncControlEndpoint::cancelTransfers ()
{
	char buffer [256];
	sl::Array <Transfer*> activeTransferArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	m_lock.lock ();
	ASSERT (!m_flags & Flag_Stop);
	m_flags |= Flag_CancelTransfers;
	m_event.signal ();
	m_lock.unlock ();

	m_idleEvent.wait ();
}

void
UsbAsyncControlEndpoint::completionThreadFunc ()
{
	for (;;)
	{
		m_event.wait ();

		m_lock.lock ();
		if (m_flags & Flag_Stop)
			break;

		if (m_flags & Flag_CancelTransfers)
		{
			m_flags &= ~Flag_CancelTransfers;
			cancelAllActiveTransfers_l ();
		}
		else
		{
			finalizeTransfers_l ();
		}
	}

	cancelAllActiveTransfers_l ();
}

void
UsbAsyncControlEndpoint::cancelAllActiveTransfers_l ()
{
	char buffer [256];
	sl::Array <Transfer*> activeTransferArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	sl::Iterator <Transfer> it = m_activeTransferList.getHead ();
	for (; it; it++)
		activeTransferArray.append (*it);

	m_lock.unlock ();

	size_t count = activeTransferArray.getCount ();
	for (size_t i = 0; i < count; i++)
		activeTransferArray [i]->m_usbTransfer.cancel (); // may fail if already completed

	// wait for all transfers to complete

	m_lock.lock ();

	while (!m_activeTransferList.isEmpty ())
	{
		m_lock.unlock ();
		m_event.wait ();
		m_lock.lock ();
	}

	finalizeTransfers_l ();
}

void
UsbAsyncControlEndpoint::finalizeTransfers_l ()
{
	sl::List <Transfer> transferList;
	sl::takeOver (&transferList, &m_completedTransferList);
	m_lock.unlock ();

	sl::Iterator <Transfer> it = transferList.getHead ();
	for (; it; it++)
	{
		switch (it->m_usbTransfer->status)
		{
		case LIBUSB_TRANSFER_COMPLETED:
			ASSERT ((size_t) it->m_usbTransfer->actual_length <= it->m_buffer.getSize () - sizeof (libusb_control_setup));

			if (it->m_inBufferPtr.m_p)
				memcpy (it->m_inBufferPtr.m_p, it->m_buffer + 1, it->m_usbTransfer->actual_length);

			callCompletionFunc (it->m_completionFuncPtr, it->m_usbTransfer->actual_length);
			break;

		case LIBUSB_TRANSFER_CANCELLED:
			callCompletionFunc (it->m_completionFuncPtr, -1, err::SystemErrorCode_Cancelled);
			break;

		default:
			callCompletionFunc (it->m_completionFuncPtr, -1, axl::io::UsbError (LIBUSB_ERROR_IO));
			break;
		}

		it->m_inBufferPtr = g_nullPtr;
		it->m_completionFuncPtr = g_nullFunctionPtr;
	}

	m_lock.lock ();
	m_transferPool.put (&transferList);

	if (isIdle ())
		m_idleEvent.signal ();
	m_lock.unlock ();
}

bool
UsbAsyncControlEndpoint::callCompletionFunc (
	FunctionPtr completionFuncPtr,
	size_t resultSize,
	const err::ErrorRef& error
	)
{
	bool result = true;

	JNC_BEGIN_CALL_SITE (m_runtime)

	DataPtr errorPtr = g_nullPtr;

	if (error)
		errorPtr = jnc::memDup (error, error->m_size);

	jnc::callVoidFunctionPtr (completionFuncPtr, resultSize, errorPtr);

	JNC_CALL_SITE_CATCH ()

	AXL_TRACE ("USB completion func failed: %s\n", err::getLastErrorDescription ().sz ());
	result = false;

	JNC_END_CALL_SITE ()

	return result;
}

void
LIBUSB_CALL
UsbAsyncControlEndpoint::onTransferCompleted (libusb_transfer* usbTransfer)
{
	Transfer* transfer = (Transfer*) usbTransfer->user_data;
	ASSERT (transfer->m_usbTransfer == usbTransfer);

	UsbAsyncControlEndpoint* self = transfer->m_self;

	self->m_lock.lock ();
	self->m_activeTransferList.remove (transfer);
	self->m_completedTransferList.insertTail (transfer);
	self->m_event.signal ();
	self->m_lock.unlock ();
}

//..............................................................................

} // namespace io
} // namespace jnc
