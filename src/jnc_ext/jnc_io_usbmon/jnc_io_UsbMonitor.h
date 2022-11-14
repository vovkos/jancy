#pragma once

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_WarningSuppression.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(UsbMonitor)

//..............................................................................

enum UsbMonitorOption {
	UsbMonitorOption_MessageMode            = 0x02,
	UsbMonitorOption_CompletedTransfersOnly = 0x04,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct UsbMonitorHdr: IfaceHdr {
	size_t m_snapshotLength;
	size_t m_kernelBufferSize;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_addressFilter;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class UsbMonitor:
	public UsbMonitorHdr,
	public AsyncIoDevice {
	friend class IoThread;

public:
	enum Def {
		Def_SnapshotLength   = 64 * 1024,
		Def_KernelBufferSize = 512 * 1024,
		Def_ReadBlockSize    = 128 * 1024,
		Def_ReadBufferSize   = 1 * 1024 * 1024,
	};

	enum IoThreadFlag {
		IoThreadFlag_Connected = 0x0010,
	};

protected:
	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, UsbMonitor, m_ioThread)->ioThreadFunc();
		}
	};

#if (_AXL_OS_WIN)
	struct OverlappedRead: sl::ListLink {
		axl::io::win::UsbMonitor::Overlapped m_overlapped;
		sl::Array<char> m_buffer;
	};

	struct OverlappedIo {
		mem::Pool<OverlappedRead> m_overlappedReadPool;
		sl::List<OverlappedRead> m_activeOverlappedReadList;
	};
#endif

protected:
#if (_AXL_OS_WIN)
	axl::io::win::UsbMonitor m_monitor;
	OverlappedIo* m_overlappedIo;
#elif (_AXL_OS_LINUX)
	axl::io::lnx::UsbMonitor m_monitor;
#endif

	IoThread m_ioThread;

public:
	UsbMonitor();

	~UsbMonitor() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap) {
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	void
	JNC_CDECL
	setReadBlockSize(size_t size) {
		AsyncIoDevice::setSetting(&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize(size_t size) {
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setKernelBufferSize(size_t size) {
		return m_monitor.setKernelBufferSize(size);
	}

	bool
	JNC_CDECL
	setAddressFilter(uint_t address) {
#if (_AXL_OS_WIN)
		return m_monitor.setFilter(address);
#elif (_AXL_OS_LINUX)
		m_monitor.m_addressFilter = address;
		return true;
#endif
	}

	bool
	JNC_CDECL
	open(
		DataPtr captureDeviceNamePtr,
		size_t snapshotLength
	);

	void
	JNC_CDECL
	close();

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
	) {
		return bufferedRead(ptr, size);
	}

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
	) {
		return AsyncIoDevice::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle) {
		return AsyncIoDevice::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
	) {
		return AsyncIoDevice::blockingWait(eventMask, timeout);
	}

	Promise*
	JNC_CDECL
	asyncWait(uint_t eventMask) {
		return AsyncIoDevice::asyncWait(eventMask);
	}

protected:
	void
	ioThreadFunc();
};

//..............................................................................

} // namespace io
} // namespace jnc
