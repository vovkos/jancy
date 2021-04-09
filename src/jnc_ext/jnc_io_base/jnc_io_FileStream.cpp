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

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	FileStream,
	"io.FileStream",
	g_ioLibGuid,
	IoLibCacheSlot_FileStream,
	FileStream,
	&FileStream::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(FileStream)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<FileStream>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<FileStream>)

	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism", &FileStream::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &FileStream::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &FileStream::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &FileStream::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &FileStream::setOptions)
	JNC_MAP_CONST_PROPERTY("m_osHandle",          &FileStream::getOsHandle)
	JNC_MAP_CONST_PROPERTY("m_kind",              &FileStream::getKind)

	JNC_MAP_FUNCTION("open",         &FileStream::open)
	JNC_MAP_FUNCTION("close",        &FileStream::close)
	JNC_MAP_FUNCTION("unsuspend",    &FileStream::unsuspend)
	JNC_MAP_FUNCTION("clear",        &FileStream::clear)
	JNC_MAP_FUNCTION("read",         &FileStream::read)
	JNC_MAP_FUNCTION("write",        &FileStream::write)
	JNC_MAP_FUNCTION("wait",         &FileStream::wait)
	JNC_MAP_FUNCTION("cancelWait",   &FileStream::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &FileStream::blockingWait)
	JNC_MAP_FUNCTION("asyncWait",    &FileStream::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
FileStream::IoThread::threadFunc()
{
	FileStream* self = containerof(this, FileStream, m_ioThread);

	self->ioThreadFunc();

	if (self->m_finalizeIoThreadFunc)
		self->m_finalizeIoThreadFunc(self);
}

FileStream::FileStream()
{
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);

	m_writeFile = &m_file;
	m_finalizeIoThreadFunc = NULL;
	m_openFlags = 0;

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
}

bool
JNC_CDECL
FileStream::open(
	DataPtr namePtr,
	uint_t openFlags
	)
{
	bool result;

	close();

	if (!requireIoLibCapability(IoLibCapability_FileStream))
		return false;

	const char* name = (const char*)namePtr.m_p;
	openFlags |= axl::io::FileFlag_Asynchronous;

#if (_JNC_OS_WIN)
	if (m_options & FileStreamOption_MessageNamedPipe)
		result =
			m_file.open(name, openFlags | axl::io::FileFlag_WriteAttributes) ||
			m_file.open(name, openFlags);
	else
		result = m_file.open(name, openFlags);

	if (!result)
		return false;

	if ((m_options & FileStreamOption_MessageNamedPipe) &&
		m_file.m_file.getType() == FILE_TYPE_PIPE)
	{
		dword_t pipeMode = PIPE_READMODE_MESSAGE;
		result = ::SetNamedPipeHandleState(m_file.m_file, &pipeMode, NULL, NULL) != 0;
		if (!result)
		{
			err::setLastSystemError();
			return false;
		}

		m_options |= AsyncIoDeviceOption_KeepReadBlockSize;
	}

	ASSERT(!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW(OverlappedIo);
#else
	result = m_file.open((const char*) namePtr.m_p, openFlags);
	if (!result)
		return false;
#endif

	m_openFlags = openFlags;

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
FileStream::close()
{
	if (!m_file.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	m_file.close();
	AsyncIoDevice::close();

#if (_AXL_OS_WIN)
	if (m_overlappedIo)
	{
		AXL_MEM_DELETE(m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

bool
JNC_CDECL
FileStream::setOptions(uint_t options)
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
			options |= AsyncIoDeviceOption_KeepReadBlockSize;
		}

		bool result = ::SetNamedPipeHandleState(m_file.m_file, &pipeMode, NULL, NULL) != 0;
		if (!result)
		{
			err::setLastSystemError();
			return false;
		}
	}
#endif

	m_lock.lock();
	m_options = options;
	wakeIoThread();
	m_lock.unlock();
	return true;
}

bool
JNC_CDECL
FileStream::clear()
{
	bool result = m_file.setSize(0);
	if (!result)
		return false;

#if (_AXL_OS_WIN)
	m_lock.lock();
	m_overlappedIo->m_readOffset = 0;
	m_overlappedIo->m_writeOffset = 0;
	m_lock.unlock();
#endif

	return true;
}

size_t
JNC_CDECL
FileStream::read(
	DataPtr ptr,
	size_t size
	)
{
	if (m_openFlags & axl::io::FileFlag_WriteOnly)
	{
		err::setError(err::SystemErrorCode_AccessDenied);
		return -1;
	}

	return bufferedRead(ptr, size);
}

size_t
JNC_CDECL
FileStream::write(
	DataPtr ptr,
	size_t size
	)
{
	if (m_openFlags & axl::io::FileFlag_ReadOnly)
	{
		err::setError(err::SystemErrorCode_AccessDenied);
		return -1;
	}

	return bufferedWrite(ptr, size);
}

#if (_JNC_OS_WIN)

void
FileStream::ioThreadFunc()
{
	ASSERT(m_file.isOpen() && m_overlappedIo);

	bool result;

	HANDLE waitTable[3] =
	{
		m_ioThreadEvent.m_event,
		m_overlappedIo->m_writeOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 2; // always 2 or 3

	bool isWritingFile = false;
	bool isDiskFile = ::GetFileType(m_file.m_file) == FILE_TYPE_DISK; // need to advance read offsets
	bool isDiskWriteFile = ::GetFileType(m_writeFile->m_file) == FILE_TYPE_DISK; // need to advance write offsets

	m_ioThreadEvent.signal(); // do initial update of active events

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedReadList.isEmpty())
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			result = read->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_file.m_file.getOverlappedResult(&read->m_overlapped, &actualSize);
			if (!result)
			{
				handleIoError(&m_file);
				return;
			}

			if (!actualSize)
			{
				setEvents(FileStreamEvent_Eof);
				return;
			}

			m_overlappedIo->m_activeOverlappedReadList.remove(read);
			m_overlappedIo->m_overlappedReadPool.put(read);
			read->m_overlapped.m_completionEvent.reset();

			// only the main read buffer must be lock-protected

			m_lock.lock();
			addToReadBuffer(read->m_buffer, actualSize);
			m_overlappedIo->m_readOffset += actualSize;
			m_lock.unlock();
		}

		if (isWritingFile && m_overlappedIo->m_writeOverlapped.m_completionEvent.wait(0))
		{
			ASSERT(isWritingFile);

			dword_t actualSize;
			result = m_writeFile->m_file.getOverlappedResult(&m_overlappedIo->m_writeOverlapped, &actualSize);
			if (!result)
			{
				handleIoError(m_writeFile);
				return;
			}

			if (actualSize < m_overlappedIo->m_writeBlock.getCount()) // shouldn't happen, actually (unless with a non-standard driver)
				m_overlappedIo->m_writeBlock.remove(0, actualSize);
			else
				m_overlappedIo->m_writeBlock.clear();

			m_overlappedIo->m_writeOverlapped.m_completionEvent.reset();
			m_overlappedIo->m_writeOffset += actualSize;

			isWritingFile = false;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			break;
		}

		if (m_ioThreadFlags & IoThreadFlag_Suspended)
		{
			m_lock.unlock();
			continue;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		getNextWriteBlock(&m_overlappedIo->m_writeBlock);
		updateReadWriteBufferEvents();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull();
		size_t readParallelism = isDiskFile ? 1 : m_readParallelism;
		size_t readBlockSize = m_readBlockSize;
		uint64_t writeOffset = m_overlappedIo->m_writeOffset;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		if (!isWritingFile && !m_overlappedIo->m_writeBlock.isEmpty())
		{
			if (isDiskWriteFile)
			{
				m_overlappedIo->m_writeOverlapped.Offset = (uint32_t)writeOffset;
				m_overlappedIo->m_writeOverlapped.OffsetHigh = (uint32_t)(writeOffset >> 32);
			}

			result = m_writeFile->m_file.overlappedWrite(
				m_overlappedIo->m_writeBlock,
				m_overlappedIo->m_writeBlock.getCount(),
				&m_overlappedIo->m_writeOverlapped
				);

			if (!result)
			{
				handleIoError(m_writeFile);
				break;
			}

			isWritingFile = true;
		}

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedReadList.getCount();
		if (!(m_openFlags & axl::io::FileFlag_WriteOnly) && !isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;
			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get();

				if (isDiskFile)
				{
					read->m_overlapped.Offset = (uint32_t)m_overlappedIo->m_readOffset;
					read->m_overlapped.OffsetHigh = (uint32_t)(m_overlappedIo->m_readOffset >> 32);
				}

				result =
					read->m_buffer.setCount(readBlockSize) &&
					m_file.m_file.overlappedRead(read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result)
				{
					m_overlappedIo->m_overlappedReadPool.put(read);
					handleIoError(&m_file);
					return;
				}

				m_overlappedIo->m_activeOverlappedReadList.insertTail(read);
			}
		}

		if (m_overlappedIo->m_activeOverlappedReadList.isEmpty())
		{
			waitCount = 2;
		}
		else
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			waitTable[2] = read->m_overlapped.m_completionEvent.m_event;
			waitCount = 3;
		}
	}
}

void
FileStream::handleIoError(axl::io::File* file)
{
	err::Error error = err::getLastError();

	if (error->m_code == ERROR_HANDLE_EOF ||
		error->m_code == ERROR_BROKEN_PIPE && file->m_file.getType() == FILE_TYPE_PIPE)
		setEvents(FileStreamEvent_Eof);
	else
		setIoErrorEvent();
}

#elif (_JNC_OS_POSIX)

void
FileStream::ioThreadFunc()
{
	ASSERT(m_file.isOpen());

	int result;
	int maxFd = AXL_MAX(m_file.m_file, m_writeFile->m_file);
	int selectFd = AXL_MAX(maxFd, (int)m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;

	readBlock.setCount(Def_ReadBlockSize);

	bool isReadOnly = (m_openFlags & axl::io::FileFlag_ReadOnly) != 0;
	bool isWriteOnly = (m_openFlags & axl::io::FileFlag_WriteOnly) != 0;

	bool canReadFile = false;
	bool canWriteFile = false;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadFile && !isWriteOnly)
			FD_SET(m_file.m_file, &readSet);

		if (!canWriteFile && !isReadOnly)
			FD_SET(m_writeFile->m_file, &writeSet);

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_file.m_file, &readSet))
			canReadFile = true;

		if (FD_ISSET(m_writeFile->m_file, &writeSet))
			canWriteFile = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return;
		}

		if (m_ioThreadFlags & IoThreadFlag_Suspended)
		{
			m_lock.unlock();
			continue;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadFile && !m_readBuffer.isFull())
		{
			ssize_t actualSize = ::read(m_file.m_file, readBlock, readBlock.getCount());
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canReadFile = false;
				}
				else
				{
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			}
			else if (actualSize == 0)
			{
				setEvents_l(FileStreamEvent_Eof);
				return;
			}
			else
			{
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteFile)
		{
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = ::write(m_writeFile->m_file, writeBlock, blockSize);
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canWriteFile = false;
				}
				else if (actualSize < 0)
				{
					setIoErrorEvent_l(err::Errno((int)actualSize));
					return;
				}
			}
			else if ((size_t)actualSize < blockSize)
			{
				writeBlock.remove(0, actualSize);
			}
			else
			{
				writeBlock.clear();
			}
		}

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
