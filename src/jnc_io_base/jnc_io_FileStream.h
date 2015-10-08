#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

class FileStream;

//.............................................................................

enum FileStreamKind
{
	FileStreamKind_Unknown,
	FileStreamKind_Disk,
	FileStreamKind_Serial,
	FileStreamKind_Pipe,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum FileStreamEventKind
{
	FileStreamEventKind_Eof,
	FileStreamEventKind_IncomingData,
	FileStreamEventKind_ReadError,
	FileStreamEventKind_TransmitBufferOverflow,
	FileStreamEventKind_TransmitBufferReady,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct FileStreamEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.FileStreamEventParams", g_ioLibCacheSlot, IoLibTypeCacheSlot_FileStreamEventParams)
	JNC_END_TYPE_MAP ()

	FileStreamEventKind m_eventKind;
	uint_t m_syncId;
	rt::DataPtr m_errorPtr;
};

//.............................................................................

class FileStream: public rt::IfaceHdr
{
	friend class IoThread;
	friend class NamedPipe;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (FileStream, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.FileStream", g_ioLibCacheSlot, IoLibTypeCacheSlot_FileStream)
		JNC_MAP_CONSTRUCTOR (&sl::construct <FileStream>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <FileStream>)
		JNC_MAP_FUNCTION ("open",  &FileStream::open)
		JNC_MAP_FUNCTION ("close", &FileStream::close)
		JNC_MAP_FUNCTION ("clear", &FileStream::clear)
		JNC_MAP_FUNCTION ("read",  &FileStream::read)
		JNC_MAP_FUNCTION ("write", &FileStream::write)
		JNC_MAP_FUNCTION ("firePendingEvents", &FileStream::firePendingEvents)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class IoThread: public mt::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, FileStream, m_ioThread)->ioThreadFunc ();
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

#if (_AXL_ENV == AXL_ENV_WIN)
	struct Read: sl::ListLink
	{
		void* m_buffer;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		mt::Event m_completionEvent;
	};
#endif

protected:
	bool m_isOpen;
	uint_t m_syncId;
	FileStreamKind m_fileStreamKind;

	rt::ClassBox <rt::Multicast> m_onFileStreamEvent;

protected:
	rt::Runtime* m_runtime;
	axl::io::File m_file;

	mt::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;

#if (_AXL_ENV == AXL_ENV_WIN)
	mt::Event m_ioThreadEvent;
	sl::Array <char> m_readBuffer;
	size_t m_incomingDataSize;
	sl::AuxList <Read> m_readList;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	FileStream ();

	~FileStream ()
	{
		close ();
	}

	bool
	AXL_CDECL
	open (
		rt::DataPtr namePtr,
		uint_t openFlags
		);

	void
	AXL_CDECL
	close ();

	bool
	AXL_CDECL
	clear ();

	size_t
	AXL_CDECL
	read (
		rt::DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		rt::DataPtr ptr,
		size_t size
		);

	void
	AXL_CDECL
	firePendingEvents ();

protected:
	size_t
	readImpl (
		void* p,
		size_t size
		);

	void
	fireFileStreamEvent (
		FileStreamEventKind eventKind,
		const err::ErrorHdr* error = NULL
		);

	void
	ioThreadFunc ();

	void
	wakeIoThread ();

#if (_AXL_ENV == AXL_ENV_WIN)
	void
	readLoop ();
#endif
};

//.............................................................................

} // namespace io
} // namespace jnc
