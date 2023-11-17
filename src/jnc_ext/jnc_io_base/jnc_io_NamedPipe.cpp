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
#include "jnc_io_NamedPipe.h"
#include "jnc_io_IoLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	NamedPipe,
	"io.NamedPipe",
	g_ioLibGuid,
	IoLibCacheSlot_NamedPipe,
	NamedPipe,
	&NamedPipe::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(NamedPipe)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<NamedPipe>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<NamedPipe>)

	JNC_MAP_AUTOGET_PROPERTY("m_backLogLimit",    &NamedPipe::setBackLogLimit)
	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism", &NamedPipe::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &NamedPipe::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &NamedPipe::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &NamedPipe::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &NamedPipe::setOptions)

	JNC_MAP_FUNCTION("open",         &NamedPipe::open)
	JNC_MAP_FUNCTION("close",        &NamedPipe::close)
	JNC_MAP_FUNCTION("accept",       &NamedPipe::accept)
	JNC_MAP_FUNCTION("wait",         &NamedPipe::wait)
	JNC_MAP_FUNCTION("cancelWait",   &NamedPipe::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &NamedPipe::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

NamedPipe::NamedPipe() {
	m_backLogLimit = Def_BackLogLimit;
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
}

bool
JNC_CDECL
NamedPipe::open(String name0) {
	bool result;

	close();

	if (!requireIoLibCapability(IoLibCapability_NamedPipe))
		return false;

	m_pipeName = L"\\\\.\\pipe\\";

	sl::StringRef name = name0 >> toAxl;
	if (name.isPrefix("\\\\.\\pipe\\"))
		m_pipeName += name.getSubString(m_pipeName.getLength());
	else
		m_pipeName += name;

	uint_t pipeMode = m_options & FileStreamOption_MessageNamedPipe ?
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE :
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;

	sl::List<OverlappedConnect> pipeList;
	for (size_t i = 0; i < m_backLogLimit; i++) {
		axl::io::win::NamedPipe pipe;
		result = pipe.create(
			m_pipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			pipeMode,
			PIPE_UNLIMITED_INSTANCES,
			m_writeBufferSize,
			m_readBufferSize,
			Def_Timeout,
			NULL
		);

		if (!result)
			return false;

		OverlappedConnect* connect = new OverlappedConnect;
		sl::takeOver(&connect->m_pipe, &pipe);
		pipeList.insertTail(connect);
	}

	ASSERT(!m_overlappedIo);
	m_overlappedIo = new OverlappedIo;
	sl::takeOver(&m_overlappedIo->m_pipeList, &pipeList);

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
NamedPipe::close() {
	if (!m_isOpen)
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	AsyncIoDevice::close();

	m_pipeName.clear();

	sl::Iterator<IncomingConnection> it = m_pendingIncomingConnectionList.getHead();
	for (; it; it++)
		it->m_pipe.close();

	m_incomingConnectionPool.put(&m_pendingIncomingConnectionList);

	ASSERT(m_overlappedIo);
	delete m_overlappedIo;
	m_overlappedIo = NULL;
}

bool
JNC_CDECL
NamedPipe::setOptions(uint_t options) {
	if (!m_isOpen) {
		m_options = options;
		return true;
	}

	if ((options & FileStreamOption_MessageNamedPipe) !=
		(m_options & FileStreamOption_MessageNamedPipe)) {
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	m_lock.lock();
	m_options = options;
	wakeIoThread();
	m_lock.unlock();
	return true;
}

FileStream*
JNC_CDECL
NamedPipe::accept(bool isSuspended) {
	m_lock.lock();
	if (m_pendingIncomingConnectionList.isEmpty()) {
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return NULL;
	}

	IncomingConnection* connection = m_pendingIncomingConnectionList.removeHead();
	HANDLE h = connection->m_pipe.detach();
	m_incomingConnectionPool.put(connection);
	wakeIoThread();
	m_lock.unlock();

	FileStream* fileStream = jnc::createClass<FileStream> (m_runtime);
	fileStream->m_file.m_file.attach(h);
	fileStream->setReadParallelism(m_readParallelism);
	fileStream->setReadBlockSize(m_readBlockSize);
	fileStream->setReadBufferSize(m_readBufferSize);
	fileStream->setWriteBufferSize(m_writeBufferSize);
	fileStream->setOptions(m_options);
	fileStream->m_overlappedIo = new FileStream::OverlappedIo;
	fileStream->m_isOpen = true;

	if (isSuspended)
		fileStream->m_ioThreadFlags |= IoThreadFlag_Suspended;

	fileStream->m_ioThread.start();
	return fileStream;
}

void
NamedPipe::ioThreadFunc() {
	ASSERT(m_overlappedIo);

	bool result;

	HANDLE waitTable[2] = {
		m_ioThreadEvent.m_event,
		NULL, // placeholder for connect completion event
	};

	size_t waitCount = 1; // always 1 or 2

	// start connect on the initial pipe list

	while (!m_overlappedIo->m_pipeList.isEmpty()) {
		OverlappedConnect* connect = m_overlappedIo->m_pipeList.removeHead();
		result = connect->m_pipe.overlappedConnect(&connect->m_overlapped);
		if (!result) {
			setIoErrorEvent();
			return;
		}

		m_overlappedIo->m_activeOverlappedConnectList.insertTail(connect);
	}

	m_ioThreadEvent.signal(); // do initial update of active events

	for (;;) {
		DWORD waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED) {
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedConnectList.isEmpty()) {
			OverlappedConnect* connect = *m_overlappedIo->m_activeOverlappedConnectList.getHead();
			result = connect->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			dword_t actualSize;
			result = connect->m_pipe.getOverlappedResult(&connect->m_overlapped, &actualSize);
			if (!result) {
				setIoErrorEvent();
				return;
			}

			m_overlappedIo->m_activeOverlappedConnectList.remove(connect);
			m_overlappedIo->m_overlappedConnectPool.put(connect);
			connect->m_overlapped.m_completionEvent.reset();

			m_lock.lock();
			IncomingConnection* connection = m_incomingConnectionPool.get();
			sl::takeOver(&connection->m_pipe, &connect->m_pipe);
			m_pendingIncomingConnectionList.insertTail(connection);
			m_lock.unlock();
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		if (!m_pendingIncomingConnectionList.isEmpty())
			m_activeEvents |= NamedPipeEvent_IncomingConnection;

		// take snapshots before releasing the lock

		size_t backLogLimit = m_backLogLimit;

		uint_t pipeMode = m_options & FileStreamOption_MessageNamedPipe ?
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE :
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		size_t backLogCount =
			m_pendingIncomingConnectionList.getCount() +
			m_overlappedIo->m_activeOverlappedConnectList.getCount();

		if (backLogCount < backLogLimit) {
			size_t newPipeCount = backLogLimit - backLogCount;
			for (size_t i = 0; i < newPipeCount; i++) {
				axl::io::win::NamedPipe pipe;
				result = pipe.create(
					m_pipeName,
					PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
					pipeMode,
					PIPE_UNLIMITED_INSTANCES,
					m_writeBufferSize,
					m_readBufferSize,
					Def_Timeout,
					NULL
				);

				if (!result) {
					setIoErrorEvent();
					return;
				}

				OverlappedConnect* connect = m_overlappedIo->m_overlappedConnectPool.get();
				sl::takeOver(&connect->m_pipe, &pipe);

				result = connect->m_pipe.overlappedConnect(&connect->m_overlapped);
				if (!result) {
					m_overlappedIo->m_overlappedConnectPool.put(connect);
					setIoErrorEvent();
					return;
				}

				m_overlappedIo->m_activeOverlappedConnectList.insertTail(connect);
			}
		}

		if (m_overlappedIo->m_activeOverlappedConnectList.isEmpty()) {
			waitCount = 1;
		} else {
			OverlappedConnect* connect = *m_overlappedIo->m_activeOverlappedConnectList.getHead();
			waitTable[1] = connect->m_overlapped.m_completionEvent.m_event;
			waitCount = 2;
		}
	}
}

//..............................................................................

} // namespace io
} // namespace jnc
