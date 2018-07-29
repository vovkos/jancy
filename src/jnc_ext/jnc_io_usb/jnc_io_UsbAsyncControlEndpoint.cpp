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
	uint_t requestId,
	uint_t value,
	uint_t index,
	DataPtr pptr,
	size_t size,
	uint_t timeout,
	FunctionPtr onCompletedPtr
	)
{
	Transfer* transfer = AXL_MEM_NEW (Transfer);

	m_lock.lock ();
	if (isIdle ())
		m_idleEvent.reset ();

	m_activeTransferList.insertTail (transfer);
	m_lock.unlock ();

	return true;
}

void
UsbAsyncControlEndpoint::cancelTransfers ()
{
	char buffer [256];
	sl::Array <Transfer*> activeTransferArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	m_lock.lock ();
	m_flags |= Flag_CancelTransfers;
	m_event.signal ();
	m_lock.unlock ();

	m_idleEvent.wait ();
}

void
UsbAsyncControlEndpoint::completionThreadFunc ()
{
	bool result;

	for (;;)
	{
		sleepIoThread ();

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}
	}

	cancelAllActiveTransfers ();
}

void
UsbAsyncControlEndpoint::cancelAllActiveTransfers ()
{
	char buffer [256];
	sl::Array <Transfer*> activeTransferArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	m_lock.lock ();

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

	// put all completed transfer back to the free pool

	while (!m_completedTransferList.isEmpty ())
	{
		Transfer* transfer = m_completedTransferList.removeHead ();
		m_transferPool.put (transfer);
	}

	m_lock.unlock ();
}

void
LIBUSB_CALL
UsbAsyncControlEndpoint::onControlTransferCompleted (libusb_transfer* usbTransfer)
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
