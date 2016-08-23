#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (FileStreamEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (FileStream)

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
	FileStreamEventKind_IoError,
	FileStreamEventKind_TransmitBufferReady,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct FileStreamEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (FileStreamEventParams)

	FileStreamEventKind m_eventKind;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//.............................................................................

class FileStream: public IfaceHdr
{
	friend class IoThread;
	friend class NamedPipe;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (FileStream)

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
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
		IoFlag_WriteOnly     = 0x0004,
		IoFlag_IncomingData  = 0x0010,
		IoFlag_RemainingData = 0x0020,
	};

#if (_JNC_ENV == JNC_ENV_WIN)
	struct Read: sl::ListLink
	{
		void* m_buffer;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completionEvent;
	};
#endif

protected:
	bool m_isOpen;
	uint_t m_syncId;
	FileStreamKind m_fileStreamKind;

	ClassBox <Multicast> m_onFileStreamEvent;

protected:
	Runtime* m_runtime;
	axl::io::File m_file;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;

#if (_JNC_ENV == JNC_ENV_WIN)
	sys::Event m_ioThreadEvent;
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
	JNC_CDECL
	open (
		DataPtr namePtr,
		uint_t openFlags
		);

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	clear ();

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
		);

	void
	JNC_CDECL
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

#if (_JNC_ENV == JNC_ENV_WIN)
	void
	readLoop ();
#endif
};

//.............................................................................

} // namespace io
} // namespace jnc
