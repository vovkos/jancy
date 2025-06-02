#pragma once

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_WarningSuppression.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(UsbMonitor)

//..............................................................................

enum UsbMonOption {
	UsbMonOption_CompletedTransfersOnly = 0x02,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct UsbMonitorHdr: IfaceHdr {
	volatile uint_t m_addressFilter;
	size_t m_kernelBufferSize;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class UsbMonitor:
	public UsbMonitorHdr,
	public AsyncIoDevice {
	friend class IoThread;

public:
	enum Def {
#if (_AXL_OS_WIN)
		Def_SnapshotSize     = 64 * 1024,
#endif
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
		axl::io::win::StdOverlapped m_overlapped;
		sl::Array<char> m_buffer;
	};

	struct OverlappedIo {
		mem::Pool<OverlappedRead> m_overlappedReadPool;
		sl::List<OverlappedRead> m_activeOverlappedReadList;
	};

	typedef axl::io::win::UsbPcapTransferParser TransferParser;
#elif (_AXL_OS_LINUX)
	typedef axl::io::lnx::UsbMonTransferParser TransferParser;
#endif

	struct Transfer: sl::ListLink {
		axl::io::UsbMonTransferHdr m_hdr;
		sl::Array<axl::io::UsbMonIsoPacket> m_isoPacketArray;
		sl::Array<char> m_data;
	};

	struct TransferTracker {
		mem::Pool<Transfer> m_transferPool;
		sl::List<Transfer> m_activeTransferList;
		sl::SimpleHashTable<uint64_t, Transfer*> m_activeTransferMap;
	};

	enum TransferDataMode {
		TransferDataMode_Ignore = 0,
		TransferDataMode_AddToTransfer,
		TransferDataMode_AddToBuffer
	};

	struct TransferParserContext {
		Transfer* m_transfer;
		TransferDataMode m_dataMode;

		TransferParserContext() {
			m_transfer = NULL;
			m_dataMode = TransferDataMode_Ignore;
		}
	};

protected:
#if (_AXL_OS_WIN)
	axl::io::win::UsbPcap m_monitor;
	OverlappedIo* m_overlappedIo;
#elif (_AXL_OS_LINUX)
	axl::io::lnx::UsbMon m_monitor;
#endif
	TransferTracker* m_transferTracker;
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
	setKernelBufferSize(size_t size);

	bool
	JNC_CDECL
	setAddressFilter(uint_t address);

	bool
	JNC_CDECL
	setOptions(uint_t options);

	bool
	JNC_CDECL
	open(
		String captureDeviceName,
		size_t snapshotSize
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

	bool
	processIncomingData_l(
		TransferParser* parser,
		TransferParserContext* context,
		const void* p0,
		size_t size
	);

	bool
	parseAllTransfers_l(
		TransferParser* parser,
		TransferParserContext* context,
		const char* p,
		const char* end
	);

	bool
	parseCompletedTransfersOnly_l(
		TransferParser* parser,
		TransferParserContext* context,
		const char* p,
		const char* end
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
UsbMonitor::processIncomingData_l(
	TransferParser* parser,
	TransferParserContext* context,
	const void* p0,
	size_t size
) {
	const char* p = (char*)p0;
	const char* end = p + size;

	return m_transferTracker ?
		parseCompletedTransfersOnly_l(parser, context, p, end) :
		parseAllTransfers_l(parser, context, p, end);
}

//..............................................................................

} // namespace io
} // namespace jnc
