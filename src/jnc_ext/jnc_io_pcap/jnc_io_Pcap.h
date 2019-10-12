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

enum PcapEvent
{
	PcapEvent_Eof = 0x010,
};

//..............................................................................

struct PcapHdr: IfaceHdr
{
	bool m_isPromiscious;
	uint_t m_readTimeout;
	size_t m_readBufferSize;
	DataPtr m_filterPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Pcap:
	public PcapHdr,
	public AsyncIoDevice
{
	friend class IoThread;

protected:
	enum Def
	{
		Def_ReadBufferSize  = 16 * 1024,
	};

	class IoThread: public sys::ThreadImpl<IoThread>
	{
	public:
		void
		threadFunc()
		{
			containerof(this, Pcap, m_ioThread)->ioThreadFunc();
		}
	};

	struct Read: sl::ListLink
	{
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

	~Pcap()
	{
		close();
	}

	pcap_t*
	getPcap()
	{
		return m_pcap;
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	int
	JNC_CDECL
	getLinkType()
	{
		return m_pcap.getLinkType();
	}

	size_t
	JNC_CDECL
	getSnapshotSize()
	{
		return m_pcap.getSnapshotSize();
	}

	bool
	JNC_CDECL
	openDevice(
		DataPtr deviceNamePtr,
		DataPtr filterPtr,
		uint_t snapshotSize,
		bool isPromiscious,
		uint_t readTimeout
		);

	bool
	JNC_CDECL
	openFile(
		DataPtr fileNamePtr,
		DataPtr filterPtr
		);

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	setReadBufferSize(size_t size)
	{
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setFilter(
		DataPtr filter,
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

protected:
	bool
	finishOpen();

	void
	ioThreadFunc();
};

//..............................................................................

struct PcapAddress
{
	DataPtr m_nextPtr;

	uint32_t m_address;
	uint32_t m_mask;
	uint32_t m_broadcast;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PcapDeviceDesc
{
	DataPtr m_nextPtr;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;
	PcapAddress m_address;
	uint_t m_flags;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createPcapDeviceDescList(DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
