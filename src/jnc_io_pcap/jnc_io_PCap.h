#pragma once

#include "jnc_io_PCapLibGlobals.h"

namespace jnc {
namespace io {

//.............................................................................

enum PCapEventKind
{
	PCapEventKind_ReadyRead = 0,
	PCapEventKind_Eof,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PCapEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.PCapEventParams", g_pcapLibCacheSlot, PCapLibTypeCacheSlot_PCapEventParams)
	JNC_END_TYPE_MAP ()

	PCapEventKind m_eventKind;
	uint_t m_syncId;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class PCap: public IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (PCap, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.PCap", g_pcapLibCacheSlot, PCapLibTypeCacheSlot_PCap)
		JNC_MAP_CONSTRUCTOR (&sl::construct <PCap>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <PCap>)
		JNC_MAP_FUNCTION ("openDevice",  &PCap::openDevice)
		JNC_MAP_FUNCTION ("openFile",    &PCap::openFile)
		JNC_MAP_FUNCTION ("close",       &PCap::close)
		JNC_MAP_FUNCTION ("setFilter",   &PCap::setFilter)
		JNC_MAP_FUNCTION ("write",       &PCap::write)
		JNC_MAP_FUNCTION ("read",        &PCap::read)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	enum DefKind
	{
		DefKind_SnapshotSize = 2 * 1024,
	};

	enum IoFlag
	{
		IoFlag_Closing = 0x0001,
		IoFlag_File    = 0x0010,
		IoFlag_Eof     = 0x0020,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, PCap, m_ioThread)->ioThreadFunc ();
		}
	};

	struct Read: sl::ListLink
	{
		void* m_p;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completeEvent;
	};

protected:
	DataPtr m_filter;
	bool m_isPromiscious;
	bool m_isOpen;
	uint_t m_syncId;

	ClassBox <Multicast> m_onPCapEvent;

protected:
	rt::Runtime* m_runtime;
	axl::io::PCap m_pcap;
	sys::Lock m_ioLock;
	sl::AuxList <Read> m_readList;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;
	sys::Event m_ioThreadEvent;

public:
	PCap ();

	~PCap ()
	{
		close ();
	}

	bool
	AXL_CDECL
	setFilter (DataPtr filter);

	bool
	AXL_CDECL
	openDevice (
		DataPtr deviceName,
		DataPtr filter,
		bool isPromiscious = false
		);

	bool
	AXL_CDECL
	openFile (
		DataPtr fileName,
		DataPtr filter
		);

	void
	AXL_CDECL
	close ();

	size_t
	AXL_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		DataPtr ptr,
		size_t size
		)
	{
		return m_pcap.write (ptr.m_p, size);
	}

protected:
	void
	firePCapEvent (PCapEventKind eventKind);

	void
	ioThreadFunc ();

	void
	cancelAllReads_l ();
};

//.............................................................................

struct PCapAddress
{
	JNC_BEGIN_TYPE_MAP ("io.PCapAddress", g_pcapLibCacheSlot, PCapLibTypeCacheSlot_PCapAddress)
	JNC_END_TYPE_MAP ()

	DataPtr m_nextPtr;

	uint32_t m_address;
	uint32_t m_mask;
	uint32_t m_broadcast;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PCapDeviceDesc
{
	JNC_BEGIN_TYPE_MAP ("io.PCapDeviceDesc", g_pcapLibCacheSlot, PCapLibTypeCacheSlot_PCapDeviceDesc)
	JNC_END_TYPE_MAP ()

	DataPtr m_nextPtr;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;
	PCapAddress m_address;
	uint_t m_flags;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createPCapDeviceDescList (DataPtr countPtr);

//.............................................................................

} // namespace io
} // namespace jnc
