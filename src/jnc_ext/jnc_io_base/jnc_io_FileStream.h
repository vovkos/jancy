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

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (FileStreamEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (FileStream)

//..............................................................................

enum FileStreamKind
{
	FileStreamKind_Unknown,
	FileStreamKind_Disk,
	FileStreamKind_Serial,
	FileStreamKind_Pipe,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum FileStreamEventCode
{
	FileStreamEventCode_Eof,
	FileStreamEventCode_IncomingData,
	FileStreamEventCode_IoError,
	FileStreamEventCode_TransmitBufferReady,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct FileStreamEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (FileStreamEventParams)

	FileStreamEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum NamedPipeMode
{
	NamedPipeMode_Stream,
	NamedPipeMode_Message,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ReadNamedPipeMessageResult
{
	ReadNamedPipeMessageResult_Error    = -1,
	ReadNamedPipeMessageResult_MoreData = 0,
	ReadNamedPipeMessageResult_Success  = 1,
};

//..............................................................................

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
			containerof (this, FileStream, m_ioThread)->ioThreadFunc ();
		}
	};

	enum Const
	{
		Const_ReadBufferSize  = 4 * 1024,
	};

	enum IoFlag
	{
		IoFlag_Opened                  = 0x0001,
		IoFlag_WriteOnly               = 0x0002,
		IoFlag_FileStreamEventDisabled = 0x0004,
		IoFlag_Closing                 = 0x0010,
		IoFlag_IncomingData            = 0x0020,
		IoFlag_RemainingData           = 0x0040,
		IoFlag_IncompleteMessage       = 0x0080,
	};

	struct PendingEvent: sl::ListLink
	{
		FileStreamEventCode m_eventCode;
		err::Error m_error;
	};

#if (_JNC_OS_WIN)
	struct Read: sl::ListLink
	{
		void* m_buffer;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completionEvent;
		bool m_isIncompleteMessage;
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
	sl::StdList <PendingEvent> m_pendingEventList;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
	sl::AuxList <Read> m_readList;
	sl::Array <char> m_readBuffer;
	size_t m_incomingDataSize;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	FileStream ();

	~FileStream ()
	{
		close ();
	}

	NamedPipeMode
	JNC_CDECL
	getNamedPipeReadMode ();

	bool
	JNC_CDECL
	setNamedPipeReadMode (NamedPipeMode mode);

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

	ReadNamedPipeMessageResult
	JNC_CDECL
	readNamedPipeMessage (
		DataPtr ptr,
		size_t size,
		DataPtr actualSizePtr
		);

	bool
	JNC_CDECL
	isFileStreamEventEnabled ()
	{
		return !(m_ioFlags & IoFlag_FileStreamEventDisabled);
	}

	void
	JNC_CDECL
	setFileStreamEventEnabled (bool isEnabled);

protected:
	size_t
	readImpl (
		void* p,
		size_t size
		);

	void
	fireFileStreamEvent (
		FileStreamEventCode eventCode,
		const err::ErrorHdr* error = NULL
		);

	void
	fireFileStreamEventImpl (
		FileStreamEventCode eventCode,
		const err::ErrorHdr* error
		);

	void
	ioThreadFunc ();

	void
	wakeIoThread ();

#if (_JNC_OS_WIN)
	void
	readLoop ();
#endif
};

//..............................................................................

} // namespace io
} // namespace jnc
