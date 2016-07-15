#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

class Mailslot;

//.............................................................................

enum MailslotEventKind
{
	MailslotEventKind_IncomingData,
	MailslotEventKind_IoError,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct MailslotEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.MailslotEventParams", g_ioLibCacheSlot, IoLibCacheSlot_MailslotEventParams)
	JNC_END_TYPE_MAP ()

	MailslotEventKind m_eventKind;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//.............................................................................

class Mailslot: public IfaceHdr
{
	friend class IoThread;
	friend class NamedPipe;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Mailslot, NULL)

	JNC_BEGIN_TYPE_FUNCTION_MAP ("io.Mailslot", g_ioLibCacheSlot, IoLibCacheSlot_Mailslot)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Mailslot>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Mailslot>)
		JNC_MAP_FUNCTION ("open",  &Mailslot::open)
		JNC_MAP_FUNCTION ("close", &Mailslot::close)
		JNC_MAP_FUNCTION ("read",  &Mailslot::read)
	JNC_END_TYPE_FUNCTION_MAP ()

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
	rt::Runtime* m_runtime;
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
	AXL_CDECL
	open (DataPtr namePtr);

	void
	AXL_CDECL
	close ();

	size_t
	AXL_CDECL
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
		MailslotEventKind eventKind,
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
