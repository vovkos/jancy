#pragma once

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_WarningSuppression.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(DeviceMonitor)

//..............................................................................

struct DeviceMonitorHdr: IfaceHdr
{
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_pendingNotifySizeLimit;

	DataPtr m_deviceNamePtr;
	DataPtr m_fileNameFilterPtr;

	bool m_isConnected;
	bool m_isEnabled;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DeviceMonitor:
	public DeviceMonitorHdr,
	public AsyncIoDevice
{
	friend class IoThread;

public:
	enum Def
	{
		Def_ReadParallelism        = 4,
		Def_ReadBlockSize          = 4 * 1024,
		Def_ReadBufferSize         = 16 * 1024,
		Def_PendingNotifySizeLimit = 1 * 1024 * 1024,
	};

	enum IoThreadFlag
	{
		IoThreadFlag_Connected = 0x0010,
	};

protected:
	class IoThread: public sys::ThreadImpl<IoThread>
	{
	public:
		void
		threadFunc()
		{
			containerof(this, DeviceMonitor, m_ioThread)->ioThreadFunc();
		}
	};

#if (_AXL_OS_WIN)
	struct OverlappedIo
	{
		mem::Pool<OverlappedRead> m_overlappedReadPool;
		sl::List<OverlappedRead> m_activeOverlappedReadList;
	};
#endif

protected:
	dm::Monitor m_monitor;
	IoThread m_ioThread;

#if (_AXL_OS_WIN)
	OverlappedIo* m_overlappedIo;
#endif

public:
	DeviceMonitor();

	~DeviceMonitor()
	{
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	void
	JNC_CDECL
	setReadParallelism(uint_t count)
	{
		AsyncIoDevice::setSetting(&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	void
	JNC_CDECL
	setReadBlockSize(size_t size)
	{
		AsyncIoDevice::setSetting(&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize(size_t size)
	{
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setPendingNotifySizeLimit(size_t limit);

	bool
	JNC_CDECL
	setFileNameFilter(DataPtr filterPtr);

	bool
	JNC_CDECL
	setEnabled(bool isEnabled);

	bool
	JNC_CDECL
	open();

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	connect(DataPtr deviceNamePtr);

	bool
	JNC_CDECL
	setIoctlDescTable(
		DataPtr ioctlDescPtr,
		size_t count
		);

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedRead(ptr, size);
	}

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle)
	{
		return AsyncIoDevice::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait(eventMask, timeout);
	}

	Promise*
	JNC_CDECL
	asyncWait(uint_t eventMask)
	{
		return AsyncIoDevice::asyncWait(eventMask);
	}

protected:
	void
	ioThreadFunc();

	bool
	connectLoop();
};

//..............................................................................

} // namespace io
} // namespace jnc
