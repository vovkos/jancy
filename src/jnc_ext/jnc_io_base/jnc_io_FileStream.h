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

#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(FileStream)

//..............................................................................

enum FileStreamKind
{
	FileStreamKind_Unknown,
	FileStreamKind_Disk,
	FileStreamKind_Serial,
	FileStreamKind_Pipe,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum FileStreamOption
{
	FileStreamOption_MessageNamedPipe = 0x04,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum FileStreamEvent
{
	FileStreamEvent_Eof = 0x0010,
};

//..............................................................................

struct FileStreamHdr: IfaceHdr
{
	FileStreamKind m_fileStreamKind;

	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class FileStream:
	public FileStreamHdr,
	public AsyncIoDevice
{
	friend class IoThread;
	friend class NamedPipe;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(FileStream)

protected:
	enum Def
	{
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 1 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = 0,
	};

	class IoThread: public sys::ThreadImpl<IoThread>
	{
	public:
		void
		threadFunc()
		{
			containerof(this, FileStream, m_ioThread)->ioThreadFunc();
		}
	};

#if (_AXL_OS_WIN)
	struct OverlappedIo
	{
		mem::Pool<OverlappedRead> m_overlappedReadPool;
		sl::List<OverlappedRead> m_activeOverlappedReadList;
		axl::io::win::StdOverlapped m_writeOverlapped;
		sl::Array<char> m_writeBlock;
		uint64_t m_readOffset;
		uint64_t m_writeOffset;

		OverlappedIo()
		{
			m_readOffset = 0;
			m_writeOffset = 0;
		}
	};
#endif

protected:
	axl::io::File m_file;
	uint_t m_openFlags;
	IoThread m_ioThread;

#if (_AXL_OS_WIN)
	OverlappedIo* m_overlappedIo;
#endif

public:
	FileStream();

	~FileStream()
	{
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	void
	JNC_CDECL
	setReadParallelism(uint_t count)
	{
		AsyncIoDevice::setSetting(&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	void
	JNC_CDECL
	setReadBlockSize(size_t size)
	{
		AsyncIoDevice::setSetting(&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize(size_t size)
	{
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize(size_t size)
	{
		return AsyncIoDevice::setWriteBufferSize(&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	bool
	JNC_CDECL
	setOptions(uint_t options);

	bool
	JNC_CDECL
	open(
		DataPtr namePtr,
		uint_t openFlags
		);

	void
	JNC_CDECL
	close();

	void
	JNC_CDECL
	unsuspend()
	{
		suspendIoThread(false);
	}

	bool
	JNC_CDECL
	clear();

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	write(
		DataPtr ptr,
		size_t size
		);

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle)
	{
		return AsyncIoDevice::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait(eventMask, timeout);
	}

protected:
	void
	ioThreadFunc();
};

//..............................................................................

} // namespace io
} // namespace jnc
