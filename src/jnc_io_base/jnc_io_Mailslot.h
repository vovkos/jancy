#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (MailslotEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Mailslot)

//.............................................................................

enum MailslotEventCode
{
	MailslotEventCode_IncomingData,
	MailslotEventCode_IoError,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct MailslotEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (MailslotEventParams)

	MailslotEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//.............................................................................

class Mailslot: public IfaceHdr
{
	friend class IoThread;
	friend class NamedPipe;
	
protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, Mailslot, m_ioThread)->ioThreadFunc ();
		}
	};

	enum Const
	{
		Const_ReadBufferSize  = 4 * 1024,
	};

	enum IoFlag
	{
		IoFlag_Opened        = 0x0001,
		IoFlag_Closing       = 0x0002,
		IoFlag_IncomingData  = 0x0010,
		IoFlag_RemainingData = 0x0020,
	};

	struct Read: sl::ListLink
	{
		void* m_buffer;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completionEvent;
	};

protected:
	bool m_isOpen;
	uint_t m_syncId;

	ClassBox <Multicast> m_onMailslotEvent;

protected:
	Runtime* m_runtime;
	axl::io::File m_file;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;

	sys::Event m_ioThreadEvent;
	sl::Array <char> m_readBuffer;
	size_t m_incomingDataSize;
	sl::AuxList <Read> m_readList;

public:
	Mailslot ();

	~Mailslot ()
	{
		close ();
	}

	bool
	JNC_CDECL
	open (DataPtr namePtr);

	void
	JNC_CDECL
	close ();

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

protected:
	size_t
	readImpl (
		void* p,
		size_t size
		);

	void
	fireMailslotEvent (
		MailslotEventCode eventCode,
		const err::ErrorHdr* error = NULL
		);

	void
	ioThreadFunc ();

	void
	readLoop ();
};

//.............................................................................

} // namespace io
} // namespace jnc
