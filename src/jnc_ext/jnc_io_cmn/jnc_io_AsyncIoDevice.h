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

//..............................................................................

enum AsyncIoEvent
{
	AsyncIoEvent_IoError             = 0x0001,
	AsyncIoEvent_IncomingData        = 0x0002,
	AsyncIoEvent_ReceiveBufferFull   = 0x0004,
	AsyncIoEvent_TransmitBufferReady = 0x0008,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AsyncIoCompatibilityFlag
{
	AsyncIoCompatibilityFlag_MaintainReadBlockSize  = 0x01,
	AsyncIoCompatibilityFlag_MaintainWriteBlockSize = 0x02,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class AsyncIoDevice
{
protected:
	enum IoFlag
	{
		IoFlag_Closing = 0x0001,
	};

	struct Wait: sl::ListLink
	{
		uint_t m_mask;
	};

	struct SyncWait: Wait
	{
		sys::Event* m_event;
	};

	struct AsyncWait: Wait
	{
		FunctionPtr m_handlerPtr;
		handle_t m_handle;
	};

#if (_AXL_OS_WIN)
	struct OverlappedRead: sl::ListLink
	{
		axl::io::win::StdOverlapped m_overlapped;
		sl::Array <char> m_buffer;
	};
#endif

	struct ReadWriteMetaData: sl::ListLink
	{
		size_t m_blockSize;
		size_t m_paramSize;
	};

public:
	uint_t m_activeEvents;
	DataPtr m_ioErrorPtr;

	bool m_isOpen;

protected:
	Runtime* m_runtime;
	sys::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	sl::AuxList <SyncWait> m_syncWaitList;
	sl::StdList <AsyncWait> m_asyncWaitList;
	sl::StdList <AsyncWait> m_pendingAsyncWaitList;
	sl::HandleTable <AsyncWait*> m_asyncWaitMap;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

	// buffers are not always used, but we get a cleaner impl this way

	sl::CircularBuffer m_readBuffer;
	sl::CircularBuffer m_writeBuffer;
	sl::Array <char> m_readOverflowBuffer;

	sl::StdList <ReadWriteMetaData> m_readMetaDataList;
	sl::StdList <ReadWriteMetaData> m_writeMetaDataList;
	sl::StdList <ReadWriteMetaData> m_freeReadWriteMetaDataList;

#if (_AXL_OS_WIN)
	sl::StdList <OverlappedRead> m_activeOverlappedReadList;
	sl::StdList <OverlappedRead> m_freeOverlappedReadList;
	sl::Array <char> m_writeBlock;
#endif

public:
	AsyncIoDevice ();

protected:
	void
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	handle_t
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		);

	bool
	cancelWait (handle_t handle);

	uint_t
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		);

	void
	open ();

	void
	close ();

	void
	wakeIoThread ();

	bool
	setReadParallelismImpl (
		uint_t* p,
		uint_t count
		);

	bool
	setReadBlockSizeImpl (
		size_t* p,
		size_t size
		);

	bool
	setReadBufferSizeImpl (
		size_t* p,
		size_t size
		);

	bool
	setWriteBufferSizeImpl (
		size_t* p,
		size_t size
		);

	void
	cancelAllWaits ();

	size_t
	processWaitLists_l ();

	void
	setEvents (uint_t events);

	void
	setErrorEvent (
		uint_t event,
		const err::Error& error
		);

	void
	setIoErrorEvent (const err::Error& error)
	{
		setErrorEvent (AsyncIoEvent_IoError, error);
	}

	void
	setIoErrorEvent ()
	{
		setErrorEvent (AsyncIoEvent_IoError, err::getLastError ());
	}

	bool
	isReadBufferValid ();

	bool
	addToReadBuffer (
		const void* p,
		size_t size,
		const uint_t* flags
		);

	size_t
	bufferedRead (
		DataPtr ptr,
		size_t size
		);

	size_t
	bufferedWrite (
		DataPtr ptr,
		size_t size,
		const uint_t* flags
		);

#if (_JNC_OS_WIN)
	OverlappedRead*
	createOverlappedRead ()
	{
		return !m_freeOverlappedReadList.isEmpty () ?
			m_freeOverlappedReadList.removeHead () :
			AXL_MEM_NEW (OverlappedRead);
	}
#endif

	ReadWriteMetaData*
	createReadWriteMetaData (size_t paramSize = 0)
	{
		return !m_freeReadWriteMetaDataList.isEmpty () &&
			m_freeReadWriteMetaDataList.getHead ()->m_paramSize >= paramSize ?
			m_freeReadWriteMetaDataList.removeHead () :
			AXL_MEM_NEW_EXTRA (ReadWriteMetaData, paramSize);
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
