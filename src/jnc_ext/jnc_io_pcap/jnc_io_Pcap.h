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

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Pcap)
JNC_DECLARE_TYPE(PcapAddress)
JNC_DECLARE_TYPE(PcapDeviceDesc)

//..............................................................................

enum PcapEvent {
	PcapEvent_Eof = 0x010,
};

//..............................................................................

struct PcapHdr: IfaceHdr {
	bool m_isPromiscious;
	uint_t m_readTimeout;
	size_t m_kernelBufferSize;
	size_t m_readBufferSize;
	String m_filter;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Pcap:
	public PcapHdr,
	public AsyncIoDevice {
	friend class IoThread;

protected:
	enum Def {
		Def_IsPromiscious = false,
		Def_ReadTimeout   = 200,
		Def_SnapshotSize  = 64 * 1024,
		Def_ReadBufferSize = 64 * 1024,
	};

	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, Pcap, m_ioThread)->ioThreadFunc();
		}
	};

	struct Read: sl::ListLink {
		void* m_p;
		size_t m_size;
		size_t m_result;
		sys::Event m_completeEvent;
	};

protected:
	axl::io::Pcap m_pcap;
	IoThread m_ioThread;

public:
	Pcap();

	~Pcap() {
		close();
	}

	pcap_t*
	getPcap() {
		return m_pcap;
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	bool
	JNC_CDECL
	openDevice(String deviceName);

	bool
	JNC_CDECL
	openLive(
		String deviceName,
		String filter,
		uint_t snapshotSize,
		bool isPromiscious,
		uint_t readTimeout
	);

	bool
	JNC_CDECL
	openFile(
		String fileName,
		String filter
	);

	void
	JNC_CDECL
	close();

	int
	JNC_CDECL
	getLinkType() {
		return m_pcap.getLinkType();
	}

	void
	JNC_CDECL
	setPromiscious(bool isPromiscious);

	void
	JNC_CDECL
	setReadTimeout(uint_t timeout);

	size_t
	JNC_CDECL
	getSnapshotSize() {
		return m_pcap.getSnapshotSize();
	}

	void
	JNC_CDECL
	setSnapshotSize(size_t size);

	void
	JNC_CDECL
	setKernelBufferSize(size_t size);

	bool
	JNC_CDECL
	setReadBufferSize(size_t size) {
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	activate(String filter);

	bool
	JNC_CDECL
	setFilter(
		String filter,
		bool isOptimized,
		uint32_t netMask
	);

	size_t
	JNC_CDECL
	read(
		DataPtr dataPtr,
		size_t size,
		DataPtr timestampPtr
	);

	size_t
	JNC_CDECL
	write(
		DataPtr ptr,
		size_t size
	);

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
	finishOpen(uint_t ioThreadFlags = 0);

	void
	ioThreadFunc();
};

//..............................................................................

struct PcapAddress {
	DataPtr m_nextPtr;
	uint32_t m_address;
	uint32_t m_mask;
	uint32_t m_broadcast;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PcapDeviceDesc {
	DataPtr m_nextPtr;
	String m_name;
	String m_description;
	PcapAddress m_address;
	uint_t m_flags;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createPcapDeviceDescList(DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
