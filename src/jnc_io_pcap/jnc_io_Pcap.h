#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (PcapEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Pcap)
JNC_DECLARE_TYPE (PcapAddress)
JNC_DECLARE_TYPE (PcapDeviceDesc)

//.............................................................................

enum PcapEventCode
{
	PcapEventCode_ReadyRead = 0,
	PcapEventCode_Eof,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PcapEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (PcapEventParams)

	PcapEventCode m_eventCode;
	uint_t m_syncId;
};

//.............................................................................

class Pcap: public IfaceHdr
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
			containerof (this, Pcap, m_ioThread)->ioThreadFunc ();
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

	ClassBox <Multicast> m_onPcapEvent;

protected:
	Runtime* m_runtime;
	axl::io::Pcap m_pcap;
	sys::Lock m_ioLock;
	sl::AuxList <Read> m_readList;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;
	sys::Event m_ioThreadEvent;

public:
	Pcap ();

	~Pcap ()
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
	firePcapEvent (PcapEventCode eventCode);

	void
	ioThreadFunc ();

	void
	cancelAllReads_l ();
};

//.............................................................................

struct PcapAddress
{
	DataPtr m_nextPtr;

	uint32_t m_address;
	uint32_t m_mask;
	uint32_t m_broadcast;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PcapDeviceDesc
{
	DataPtr m_nextPtr;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;
	PcapAddress m_address;
	uint_t m_flags;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createPcapDeviceDescList (DataPtr countPtr);

//.............................................................................

} // namespace io
} // namespace jnc
