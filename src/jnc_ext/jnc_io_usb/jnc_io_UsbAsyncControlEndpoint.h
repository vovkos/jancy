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

class UsbAsyncControlEndpoint {
	friend class IoThread;

protected:
	struct Transfer: sl::ListLink {
		UsbAsyncControlEndpoint* m_self;
		axl::io::UsbTransfer m_usbTransfer;
		axl::rc::Buf<libusb_control_setup> m_buffer;
		jnc::DataPtr m_inBufferPtr;
		jnc::FunctionPtr m_completionFuncPtr;
	};

	enum Flag {
		Flag_Stop            = 0x01,
		Flag_CancelTransfers = 0x02,
	};

	class CompletionThread: public sys::ThreadImpl<CompletionThread> {
	public:
		void
		threadFunc() {
			containerof(this, UsbAsyncControlEndpoint, m_completionThread)->completionThreadFunc();
		}
	};

protected:
	Runtime* m_runtime;
	axl::io::UsbDevice* m_device;

	CompletionThread m_completionThread;
	sys::Lock m_lock;
	volatile uint_t m_flags;
	sys::Event m_event;
	sys::NotificationEvent m_idleEvent;

	mem::Pool<Transfer> m_transferPool;
	sl::List<Transfer> m_activeTransferList;
	sl::List<Transfer> m_completedTransferList;

public:
	UsbAsyncControlEndpoint(axl::io::UsbDevice* device);

	~UsbAsyncControlEndpoint() {
		stop();
	}

	bool
	start();

	void
	stop();

	void
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	bool
	JNC_CDECL
	transfer(
		uint_t requestType,
		uint_t requestCode,
		uint_t value,
		uint_t index,
		DataPtr ptr,
		size_t size,
		uint_t timeout,
		FunctionPtr completionFuncPtr
	);

	void
	cancelTransfers();

protected:
	bool
	isIdle() {
		return m_activeTransferList.isEmpty() && m_completedTransferList.isEmpty();
	}

	static
	void
	markTransferGcRoots(
		GcHeap* gcHeap,
		Transfer* transfer
	);

	void
	completionThreadFunc();

	void
	cancelAllActiveTransfers_l();

	void
	finalizeTransfers_l();

	bool
	callCompletionFunc(
		FunctionPtr completionFuncPtr,
		size_t resultSize,
		const err::ErrorRef& error = err::ErrorRef()
	);

	static
	void
	LIBUSB_CALL
	onTransferCompleted(libusb_transfer* transfer);
};

//..............................................................................

} // namespace io
} // namespace jnc
