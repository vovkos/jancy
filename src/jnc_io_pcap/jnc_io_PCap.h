#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (PCapEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (PCap)
JNC_DECLARE_TYPE (PCapAddress)
JNC_DECLARE_TYPE (PCapDeviceDesc)

//.............................................................................

enum PCapEventKind
{
	PCapEventKind_ReadyRead = 0,
	PCapEventKind_Eof,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PCapEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (PCapEventParams)

	PCapEventKind m_eventKind;
	uint_t m_syncId;
};

//.............................................................................

class PCap: public IfaceHdr
{
	friend class IoThread;

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
	Runtime* m_runtime;
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
	JNC_CDECL
	setFilter (DataPtr filter);

	bool
	JNC_CDECL
	openDevice (
		DataPtr deviceName,
		DataPtr filter,
		bool isPromiscious = false
		);

	bool
	JNC_CDECL
	openFile (
		DataPtr fileName,
		DataPtr filter
		);

	void
	JNC_CDECL
	close ();

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
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
	DataPtr m_nextPtr;

	uint32_t m_address;
	uint32_t m_mask;
	uint32_t m_broadcast;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PCapDeviceDesc
{
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
