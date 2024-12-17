#pragma once

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_WarningSuppression.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(HidDevice)

//..............................................................................

enum HidDeviceOption {
	HidDeviceOption_NonBlocking  = 0x04,
	HidDeviceOption_NoReadThread = 0x08,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct HidDeviceHdr: IfaceHdr {
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	uint_t m_readTimeout;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HidDevice:
	public HidDeviceHdr,
	public AsyncIoDevice {
	friend class IoThread;

public:
	enum Def {
		Def_ReadBlockSize  = 4 * 1024,
		Def_ReadBufferSize = 1 * 1024 * 1024,
		Def_ReadTimeout    = 200,
	};

	enum IoThreadFlag {
		IoThreadFlag_Connected = 0x0010,
	};

protected:
	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, HidDevice, m_ioThread)->ioThreadFunc();
		}
	};

protected:
	axl::io::HidDevice m_device;
	IoThread m_ioThread;
	DataPtr m_deviceDescPtr;

public:
	HidDevice();

	~HidDevice() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

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

	void
	JNC_CDECL
	setReadTimeout(uint_t timeout);

	bool
	JNC_CDECL
	setOptions(uint_t options);

	static
	DataPtr
	JNC_CDECL
	getDeviceDesc(HidDevice* self);

	bool
	JNC_CDECL
	open_0(String path);

	bool
	JNC_CDECL
	open_1(
		uint16_t vid,
		uint16_t pid,
		String serialNumberPtr
	);

	void
	JNC_CDECL
	close();

	size_t
	JNC_CDECL
	getReportDescriptor(
		DataPtr ptr,
		size_t size
	) {
		return m_device.getReportDescriptor(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
	) {
		return bufferedRead(ptr, size);
	}

	size_t
	JNC_CDECL
	write(
		DataPtr ptr,
		size_t size
	) {
		return m_device.write(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	getInputReport(
		DataPtr ptr,
		size_t size
	) {
		return m_device.getInputReport(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	getFeatureReport(
		DataPtr ptr,
		size_t size
	) {
		return m_device.getFeatureReport(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	sendFeatureReport(
		DataPtr ptr,
		size_t size
	) {
		return m_device.sendFeatureReport(ptr.m_p, size);
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
	bool
	finishOpen();

	void
	ioThreadFunc();
};

//..............................................................................

} // namespace io
} // namespace jnc
