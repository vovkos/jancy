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

JNC_DECLARE_OPAQUE_CLASS_TYPE (AsyncIoDevice)

//..............................................................................

enum AsyncIoEvent
{
	AsyncIoEvent_IoError             = 0x0001,
	AsyncIoEvent_IncomingData        = 0x0002,
	AsyncIoEvent_ReceiveBufferFull   = 0x0004,
	AsyncIoEvent_TransmitBufferReady = 0x0008,
	AsyncIoEvent_BufferMask          = 0x0007,
};

class AsyncIoDevice: public IfaceHdr
{
protected:
	enum IoFlag
	{
		IoFlag_Closing = 0x0001,
		IoFlag_Writing = 0x0002,
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
	};

#if (_AXL_OS_WIN)
	struct AsyncReadBlock: sl::ListLink
	{
		axl::io::win::StdOverlapped m_overlapped;
		sl::Array <char> m_buffer;
	};
#endif

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
	sl::CircularBuffer m_readOverflowBuffer;
	sl::CircularBuffer m_writeBuffer;

#if (_AXL_OS_WIN)
	sl::StdList <AsyncReadBlock> m_activeReadBlockList;
	sl::StdList <AsyncReadBlock> m_freeReadBlockList;
	sl::Array <char> m_writeBlock;
#endif

public:
	AsyncIoDevice ();

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	handle_t 
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		);

	bool
	JNC_CDECL
	cancelWait (handle_t handle);

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		);

protected:
	void
	open ();

	void
	close ();

	void
	wakeIoThread ();

	void
	cancelAllWaits ();

	size_t
	processWaitLists_l ();

	void
	setIoErrorEvent (const err::Error& error);

	void
	setIoErrorEvent ()
	{
		setIoErrorEvent (err::getLastError ());
	}

	size_t
	bufferedRead (
		DataPtr ptr,
		size_t size
		);

	size_t
	bufferedWrite (
		DataPtr ptr,
		size_t size
		);

	bool
	addToReadBuffer (
		const void* p, 
		size_t size
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
