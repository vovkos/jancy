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

//..............................................................................

class UsbAsyncControlEndpoint
{
	friend class IoThread;

protected:
	struct Transfer: sl::ListLink
	{
		UsbAsyncControlEndpoint* m_self;
		axl::io::UsbTransfer m_usbTransfer;
		sl::Array <char> m_buffer;
	};

	enum Flag
	{
		Flag_Stop            = 0x01,
		Flag_CancelTransfers = 0x02,
	};

	class CompletionThread: public sys::ThreadImpl <CompletionThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, UsbAsyncControlEndpoint, m_completionThread)->completionThreadFunc ();
		}
	};

protected:
	CompletionThread m_completionThread;
	sys::Lock m_lock;
	volatile uint_t m_flags;
	sys::Event m_event;
	sys::NotificationEvent m_idleEvent;
	sl::StdList <Transfer> m_activeTransferList;
	sl::StdList <Transfer> m_completedTransferList;

public:
	UsbAsyncControlEndpoint ()
	{
		m_flags = 0;
	}

	~UsbAsyncControlEndpoint ()
	{
		stop ();
	}

	bool
	start ();

	void
	stop ();

	bool
	JNC_CDECL
	transfer (
		uint_t requestType,
		uint_t requestId,
		uint_t value,
		uint_t index,
		DataPtr ptr,
		size_t size,
		uint_t timeout,
		FunctionPtr onCompletedPtr
		);

	void
	cancelTransfers ();

protected:
	bool
	isIdle ()
	{
		return m_activeTransferList.isEmpty () && m_completedTransferList.isEmpty ();
	}

	void
	cancelAllActiveTransfers ();

	void
	completionThreadFunc ();

	static
	void
	LIBUSB_CALL
	onControlTransferCompleted (libusb_transfer* transfer);
};

//..............................................................................

} // namespace io
} // namespace jnc
