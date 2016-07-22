#pragma once

#include "jnc_io_FileStream.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (NamedPipe)

//.............................................................................

class NamedPipe: public IfaceHdr
{
	friend class IoThread;

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, NamedPipe, m_ioThread)->ioThreadFunc ();
		}
	};

	enum Const
	{
		Const_TxBufferSize = 512,
		Const_RxBufferSize = 512,
		Const_Timeout      = 3000, // 3 sec
		Const_MaxBackLog   = 8,
		Const_DefBackLog   = 2,
	};

	enum IoFlag
	{
		IoFlag_Opened             = 0x0001,
		IoFlag_Closing            = 0x0002,
	};

	struct Accept: sl::ListLink
	{
		HANDLE m_hPipe;
		err::Error m_error;
		sys::Event m_completionEvent;
	};

protected:
	bool m_isOpen;
	uint_t m_syncId;

	ClassBox <Multicast> m_onIncomingConnectionEvent;

protected:
	Runtime* m_runtime;
	sl::String_w m_pipeName;	
	sl::Array <axl::io::win::NamedPipe> m_pipeArray;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;
	sl::Array <size_t> m_pendingAcceptArray;
	sl::Array <size_t> m_listenArray;
	sl::AuxList <Accept> m_acceptList;
	sys::Event m_ioThreadEvent;

public:
	NamedPipe ();

	~NamedPipe ()
	{
		close ();
	}

	bool
	AXL_CDECL
	open (
		DataPtr namePtr,
		size_t backLog
		);

	void
	AXL_CDECL
	close ();

	FileStream*
	AXL_CDECL
	accept ();

protected:
	void
	ioThreadFunc ();

	void
	listenLoop ();
};

//.............................................................................

} // namespace io
} // namespace jnc
