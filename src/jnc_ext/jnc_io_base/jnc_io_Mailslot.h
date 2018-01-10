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

#if (!_AXL_OS_WIN)
#	error This file is Windows-only
#endif

#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Mailslot)

//..............................................................................

struct MailslotHdr: IfaceHdr
{
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Mailslot:
	public MailslotHdr,
	public AsyncIoDevice
{
	friend class IoThread;
	friend class NamedPipe;

protected:
	enum Def
	{
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 1 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_Options         = 0,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, Mailslot, m_ioThread)->ioThreadFunc ();
		}
	};

	struct OverlappedIo
	{
		mem::Pool <OverlappedRead> m_overlappedReadPool;
		sl::StdList <OverlappedRead> m_activeOverlappedReadList;
	};

protected:
	axl::io::File m_file;

	IoThread m_ioThread;
	OverlappedIo* m_overlappedIo;

public:
	Mailslot ();

	~Mailslot ()
	{
		close ();
	}

	bool
	JNC_CDECL
	open (DataPtr namePtr);

	void
	JNC_CDECL
	close ();

	void
	JNC_CDECL
	setReadParallelism (uint_t count)
	{
		AsyncIoDevice::setSetting (&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	void
	JNC_CDECL
	setReadBlockSize (size_t size)
	{
		AsyncIoDevice::setSetting (&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize (size_t size)
	{
		return AsyncIoDevice::setReadBufferSize (&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	void
	JNC_CDECL
	setOptions (uint_t options)
	{
		AsyncIoDevice::setSetting (&m_options, options);
	}

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedRead (ptr, size);
	}

	handle_t
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait (eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait (handle_t handle)
	{
		return AsyncIoDevice::cancelWait (handle);
	}

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait (eventMask, timeout);
	}

protected:
	void
	ioThreadFunc ();
};

//..............................................................................

} // namespace io
} // namespace jnc
