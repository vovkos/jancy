#include "pch.h"
#include "jnc_io_FileStream.h"

namespace jnc {
namespace io {

//.............................................................................

FileStream::FileStream ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_ioFlags = 0;
#if (_AXL_ENV == AXL_ENV_WIN)
	m_incomingDataSize = 0;
#endif
	m_isOpen = false;
	m_syncId = 0;
	m_fileStreamKind = FileStreamKind_Unknown;
}

void
FileStream::wakeIoThread ()
{
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

bool
AXL_CDECL
FileStream::open (
	rt::DataPtr namePtr,
	uint_t openFlags
	)
{
	bool result;

	close ();

#if (_AXL_ENV == AXL_ENV_POSIX)
	// force asynchronous and restore blocking mode later

	result =
		m_file.open ((const char*) namePtr.m_p, openFlags | axl::io::FileFlag_Asynchronous) &&
		m_file.m_file.setBlockingMode (true);
#else
	result = m_file.open ((const char*) namePtr.m_p, openFlags | axl::io::FileFlag_Asynchronous);
#endif

	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_isOpen = true;

#if (_AXL_ENV == AXL_ENV_WIN)

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

	m_ioThreadEvent.reset ();
	m_readBuffer.setCount (Const_ReadBufferSize);
	m_incomingDataSize = 0;
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.create ();
#endif

	m_ioFlags = 0;
	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
FileStream::close ()
{
	if (!m_file.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.close ();
#endif

	m_file.close ();
	m_ioFlags = 0;
#if (_AXL_ENV == AXL_ENV_WIN)
	m_incomingDataSize = 0;
#endif
	m_isOpen = false;
	m_syncId++;
}

bool
AXL_CDECL
FileStream::clear ()
{
	return m_file.setSize (0);
}

void
AXL_CDECL
FileStream::firePendingEvents ()
{
}

void
FileStream::fireFileStreamEvent (
	FileStreamEventKind eventKind,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <FileStreamEventParams> (m_runtime);
	FileStreamEventParams* params = (FileStreamEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = rt::memDup (error, error->m_size);

	rt::callMulticast (m_onFileStreamEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

#if (_AXL_ENV == AXL_ENV_WIN)

size_t
AXL_CDECL
FileStream::read (
	rt::DataPtr ptr,
	size_t size
	)
{
	m_ioLock.lock ();
	if (m_ioFlags & IoFlag_IncomingData)
	{
		size_t result = readImpl (ptr.m_p, size);
		wakeIoThread ();
		m_ioLock.unlock ();

		return result;
	}
	else
	{
		Read read;
		read.m_buffer = ptr.m_p;
		read.m_size = size;
		m_readList.insertTail (&read);
		wakeIoThread ();
		m_ioLock.unlock ();

		rt::enterWaitRegion (m_runtime);
		read.m_completionEvent.wait ();
		rt::leaveWaitRegion (m_runtime);

		if (read.m_result == -1)
			ext::setError (read.m_error);

		return read.m_result;
	}
}

size_t
FileStream::readImpl (
	void* p,
	size_t size
	)
{
	ASSERT (m_incomingDataSize);

	size_t copySize;

	if (size < m_incomingDataSize)
	{
		copySize = size;
		m_incomingDataSize -= size;
		m_ioFlags |= IoFlag_RemainingData;
		memcpy (p, m_readBuffer, copySize);
		memmove (m_readBuffer, m_readBuffer + copySize, m_incomingDataSize);
	}
	else
	{
		copySize = m_incomingDataSize;
		m_incomingDataSize = 0;
		m_ioFlags &= ~IoFlag_IncomingData;
		memcpy (p, m_readBuffer, copySize);
	}

	return copySize;
}

size_t
AXL_CDECL
FileStream::write (
	rt::DataPtr ptr,
	size_t size
	)
{
	mt::Event completionEvent;
	OVERLAPPED overlapped = { 0 };
	overlapped.hEvent = completionEvent.m_event;

	if (m_fileStreamKind == FileStreamKind_Disk)
	{
		uint64_t offset = m_file.getSize ();
		overlapped.Offset = (DWORD) offset;
		overlapped.OffsetHigh = (DWORD) (offset >> 32);
	}

	bool_t result = m_file.m_file.write (ptr.m_p, size, NULL, &overlapped);
	size_t actualSize = result ? m_file.m_file.getOverlappedResult (&overlapped) : -1;

	if (actualSize == -1)
		ext::propagateLastError ();

	return actualSize;
}

void
FileStream::ioThreadFunc ()
{
	ASSERT (m_file.isOpen ());

	readLoop ();

	// wait for close, cancell all incoming reads

	for (;;)
	{
		m_ioLock.lock ();

		while (!m_readList.isEmpty ())
		{
			Read* read = m_readList.removeHead ();
			read->m_result = -1;
			read->m_error = err::Error (ERROR_CANCELLED);
			read->m_completionEvent.signal ();
		}

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		m_ioLock.unlock ();

		m_ioThreadEvent.wait ();
	}
}


void
FileStream::readLoop ()
{
	mt::Event completionEvent;

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		completionEvent.m_event,
	};

	uint64_t offset = 0;

	for (;;)
	{
		Read* read = NULL;
		void* readBuffer;
		size_t readSize;

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (m_ioFlags & IoFlag_RemainingData)
		{
			ASSERT (m_ioFlags & IoFlag_IncomingData);

			if (!m_readList.isEmpty ())
			{
				read = m_readList.removeHead ();
				readImpl (read->m_buffer, read->m_size);
			}
			else
			{
				fireFileStreamEvent (FileStreamEventKind_IncomingData);
			}

			m_ioFlags &= ~IoFlag_RemainingData;
			m_ioLock.unlock ();
			continue;
		}

		if (!m_readList.isEmpty ())
		{
			read = m_readList.removeHead ();
			readBuffer = read->m_buffer;
			readSize = read->m_size;
		}
		else if (!(m_ioFlags & IoFlag_IncomingData))
		{
			ASSERT (m_incomingDataSize == 0);
			readBuffer = m_readBuffer;
			readSize = m_readBuffer.getCount ();
		}
		else
		{
			readBuffer = NULL;
		}

		m_ioLock.unlock ();

		size_t waitCount = 1;

		OVERLAPPED overlapped = { 0 };

		if (readBuffer)
		{
			overlapped.hEvent = completionEvent.m_event;
			overlapped.Offset = (DWORD) offset;
			overlapped.OffsetHigh = (DWORD) (offset >> 32);

			bool_t result = m_file.m_file.read (readBuffer, readSize, NULL, &overlapped);
			if (!result)
			{
				if (read)
				{
					read->m_result = -1;
					read->m_error = err::getLastError ();
					read->m_completionEvent.signal ();
				}

				break;
			}

			waitCount = 2;
		}

		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		switch (waitResult)
		{
		case WAIT_FAILED:
			return;

		case WAIT_OBJECT_0:
			break;

		case WAIT_OBJECT_0 + 1:
			readSize = m_file.m_file.getOverlappedResult (&overlapped);
			if (readSize == -1)
			{
				err::Error error = err::getLastError ();

				if (read)
				{
					read->m_result = -1;
					read->m_error = error;
					read->m_completionEvent.signal ();
				}

				fireFileStreamEvent (FileStreamEventKind_ReadError, error);
				return;
			}

			if (read)
			{
				read->m_result = readSize;
				read->m_completionEvent.signal ();
			}
			else
			{
				m_ioLock.lock ();
				ASSERT (!(m_ioFlags & IoFlag_IncomingData));
				if (readSize)
				{
					m_ioFlags |= IoFlag_IncomingData;
					m_incomingDataSize = readSize;
				}

				m_ioLock.unlock ();

				if (readSize)
				{
					fireFileStreamEvent (FileStreamEventKind_IncomingData);
				}
				else
				{
					fireFileStreamEvent (FileStreamEventKind_Eof);
					return;
				}
			}

			offset += readSize; // advance offset
			break;
		}
	}
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

size_t
AXL_CDECL
FileStream::read (
	rt::DataPtr ptr,
	size_t size
	)
{
	size_t result = m_file.read (ptr.m_p, size);

	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingData;
	wakeIoThread ();
	m_ioLock.unlock ();

	return result;
}

size_t
AXL_CDECL
FileStream::write (
	rt::DataPtr ptr,
	size_t size
	)
{
	return m_file.write (ptr.m_p, size);
}

void
FileStream::ioThreadFunc ()
{
	ASSERT (m_file.isOpen ());

	int result;
	int selectFd = AXL_MAX (m_file.m_file, m_selfPipe.m_readFile) + 1;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };

		FD_SET (m_selfPipe.m_readFile, &readSet);

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (!(m_ioFlags & IoFlag_IncomingData)) // don't re-issue select if not handled yet
			FD_SET (m_file.m_file, &readSet);

		m_ioLock.unlock ();

		result = select (selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_selfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_file.m_file, &readSet))
		{
			size_t incomingDataSize = m_file.m_file.getIncomingDataSize ();
			if (incomingDataSize == -1 || !incomingDataSize) // error or end-of-file
			{
				fireFileStreamEvent (FileStreamEventKind_Eof);
				break;
			}

			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_ioLock.unlock ();

			fireFileStreamEvent (FileStreamEventKind_IncomingData);
		}
	}
}
#endif

//.............................................................................

} // namespace io
} // namespace jnc
