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

#include "pch.h"
#include "jnc_io_FileStream.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	FileStream,
	"io.FileStream",
	g_ioLibGuid,
	IoLibCacheSlot_FileStream,
	FileStream,
	&FileStream::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (FileStream)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <FileStream>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <FileStream>)

	JNC_MAP_AUTOGET_PROPERTY ("m_readParallelism", &FileStream::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",   &FileStream::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &FileStream::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize", &FileStream::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_options",         &FileStream::setOptions)

	JNC_MAP_FUNCTION ("open",         &FileStream::open)
	JNC_MAP_FUNCTION ("close",        &FileStream::close)
	JNC_MAP_FUNCTION ("clear",        &FileStream::clear)
	JNC_MAP_FUNCTION ("read",         &FileStream::read)
	JNC_MAP_FUNCTION ("write",        &FileStream::write)
	JNC_MAP_FUNCTION ("wait",         &FileStream::wait)
	JNC_MAP_FUNCTION ("cancelWait",   &FileStream::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait", &FileStream::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

FileStream::FileStream ()
{
	m_fileStreamKind = FileStreamKind_Unknown;
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
	m_writeBuffer.setBufferSize (Def_WriteBufferSize);
}

bool
JNC_CDECL
FileStream::open (
	DataPtr namePtr,
	uint_t openFlags
	)
{
	bool result;

	close ();

	result = m_file.open ((const char*) namePtr.m_p, openFlags | axl::io::FileFlag_Asynchronous);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

#if (_JNC_OS_WIN)
	dword_t pipeMode = 0;
	if (m_options & FileStreamOption_MessageNamedPipe)
	{
		pipeMode = PIPE_READMODE_MESSAGE;
		m_options |= AsyncIoOption_KeepReadBlockSize;
	}

	result = ::SetNamedPipeHandleState (m_file.m_file, &pipeMode, NULL, NULL) != 0;
	if (!result)
	{
		err::setLastSystemError ();
		propagateLastError ();
		return false;
	}

	dword_t type = ::GetFileType (m_file.m_file);
	switch (type)
	{
	case FILE_TYPE_CHAR:
		m_fileStreamKind = FileStreamKind_Serial;
		break;

	case FILE_TYPE_DISK:
		m_fileStreamKind = FileStreamKind_Disk;
		break;

	case FILE_TYPE_PIPE:
		m_fileStreamKind = FileStreamKind_Pipe;
		break;

	default:
		m_fileStreamKind = FileStreamKind_Unknown;
	};

	ASSERT (!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW (OverlappedIo);
#endif

	m_openFlags = openFlags;

	AsyncIoDevice::open ();
	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
FileStream::close ()
{
	if (!m_file.isOpen ())
		return;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_file.close ();
	AsyncIoDevice::close ();

#if (_AXL_OS_WIN)
	if (m_overlappedIo)
	{
		AXL_MEM_DELETE (m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

bool
JNC_CDECL
FileStream::setOptions (uint_t options)
{
	if (!m_isOpen)
	{
		m_options = options;
		return true;
	}

#if (_AXL_OS_WIN)
	if ((options & FileStreamOption_MessageNamedPipe) !=
		(m_options & FileStreamOption_MessageNamedPipe))
	{
		dword_t pipeMode = 0;

		if (options & FileStreamOption_MessageNamedPipe)
		{
			pipeMode = PIPE_READMODE_MESSAGE;
			options |= AsyncIoOption_KeepReadBlockSize;
		}

		bool result = ::SetNamedPipeHandleState (m_file.m_file, &pipeMode, NULL, NULL) != 0;
		if (!result)
		{
			err::setLastSystemError ();
			propagateLastError ();
			return false;
		}
	}
#endif

	m_lock.lock ();
	m_options = options;
	wakeIoThread ();
	m_lock.unlock ();
	return true;
}

bool
JNC_CDECL
FileStream::clear ()
{
	bool result = m_file.setSize (0);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	return true;
}

#if (_JNC_OS_WIN)

void
FileStream::ioThreadFunc ()
{
	ASSERT (m_file.isOpen () && m_overlappedIo);

	bool result;

	HANDLE waitTable [3] =
	{
		m_ioThreadEvent.m_event,
		m_overlappedIo->m_writeOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 2; // always 2 or 3

	bool isWritingFile = false;

	m_ioThreadEvent.signal (); // do initial update of active events

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedReadList.isEmpty ())
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead ();
			result = read->m_overlapped.m_completionEvent.wait (0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_file.m_file.getOverlappedResult (&read->m_overlapped, &actualSize);
			if (!result)
			{
				err::Error error = err::getLastError ();
				if (error->m_code == ERROR_HANDLE_EOF)
					setEvents (FileStreamEvent_Eof);
				else
					setIoErrorEvent ();

				return;
			}

			m_overlappedIo->m_activeOverlappedReadList.remove (read);

			// only the main read buffer must be lock-protected

			m_lock.lock ();
			addToReadBuffer (read->m_buffer, actualSize);
			m_lock.unlock ();

			read->m_overlapped.m_completionEvent.reset ();
			m_overlappedIo->m_overlappedReadPool.put (read);
		}

		if (isWritingFile && m_overlappedIo->m_writeOverlapped.m_completionEvent.wait (0))
		{
			ASSERT (isWritingFile);

			dword_t actualSize;
			result = m_file.m_file.getOverlappedResult (&m_overlappedIo->m_writeOverlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			if (actualSize < m_overlappedIo->m_writeBlock.getCount ()) // shouldn't happen, actually (unless with a non-standard driver)
				m_overlappedIo->m_writeBlock.remove (0, actualSize);
			else
				m_overlappedIo->m_writeBlock.clear ();

			m_overlappedIo->m_writeOverlapped.m_completionEvent.reset ();
			isWritingFile = false;
		}

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		getNextWriteBlock (&m_overlappedIo->m_writeBlock);
		updateReadWriteBufferEvents ();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull ();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

		if (!isWritingFile && !m_overlappedIo->m_writeBlock.isEmpty ())
		{
			result = m_file.m_file.overlappedWrite (
				m_overlappedIo->m_writeBlock,
				m_overlappedIo->m_writeBlock.getCount (),
				&m_overlappedIo->m_writeOverlapped
				);

			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			isWritingFile = true;
		}

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedReadList.getCount ();
		if (!(m_openFlags & axl::io::FileFlag_WriteOnly) && !isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;
			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get ();

				result =
					read->m_buffer.setCount (readBlockSize) &&
					m_file.m_file.overlappedRead (read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result)
				{
					setIoErrorEvent ();
					return;
				}

				m_overlappedIo->m_activeOverlappedReadList.insertTail (read);
			}
		}

		if (m_overlappedIo->m_activeOverlappedReadList.isEmpty ())
		{
			waitCount = 2;
		}
		else
		{
			// wait-table may already hold correct value -- but there's no harm in writing it over

			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead ();
			waitTable [2] = read->m_overlapped.m_completionEvent.m_event;
			waitCount = 3;
		}
	}
}

#elif (_JNC_OS_POSIX)

void
FileStream::ioThreadFunc ()
{
	ASSERT (m_file.isOpen ());

	int result;
	int selectFd = AXL_MAX (m_file.m_file, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array <char> readBlock;
	sl::Array <char> writeBlock;

	readBlock.setCount (Def_ReadBlockSize);

	bool canReadSerial = false;
	bool canWriteSerial = false;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadSerial)
			FD_SET (m_file.m_file, &readSet);

		if (!canWriteSerial)
			FD_SET (m_file.m_file, &writeSet);

		result = ::select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_file.m_file, &readSet))
			canReadSerial = true;

		if (FD_ISSET (m_file.m_file, &writeSet))
			canWriteSerial = true;

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount (m_readBlockSize); // update read block size

		while (canReadSerial && !m_readBuffer.isFull ())
		{
			ssize_t actualSize = ::read (m_file.m_file, readBlock, readBlock.getCount ());
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canReadSerial = false;
				}
				else
				{
					setIoErrorEvent_l (err::Errno (errno));
					return;
				}
			}
			else
			{
				addToReadBuffer (readBlock, actualSize);
			}
		}

		while (canWriteSerial)
		{
			getNextWriteBlock (&writeBlock);
			if (writeBlock.isEmpty ())
				break;

			size_t blockSize = writeBlock.getCount ();
			ssize_t actualSize = ::write (m_file.m_file, writeBlock, blockSize);
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canWriteSerial = false;
				}
				else if (actualSize < 0)
				{
					setIoErrorEvent_l (err::Errno ((int) actualSize));
					return;
				}
			}
			else if ((size_t) actualSize < blockSize)
			{
				writeBlock.remove (0, actualSize);
			}
			else
			{
				writeBlock.clear ();
			}
		}

		updateReadWriteBufferEvents ();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
