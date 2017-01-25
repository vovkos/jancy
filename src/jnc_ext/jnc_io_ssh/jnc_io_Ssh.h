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

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SshEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (SshChannel)

//..............................................................................

enum SshEventCode
{
	SshEventCode_TcpConnectCompleted = 0,
	SshEventCode_SshHandshakeCompleted,
	SshEventCode_SshAuthCompleted,
	SshEventCode_SshAuthError,
	SshEventCode_SshChannelOpened,
	SshEventCode_SshPtyRequested,
	SshEventCode_SshProcessStarted,
	SshEventCode_ConnectCompleted,
	SshEventCode_ConnectCancelled,
	SshEventCode_ConnectError,
	SshEventCode_Disconnected,
	SshEventCode_ReauthenticateInitiated,
	SshEventCode_ReconnectInitiated,
	SshEventCode_IncomingData,
	SshEventCode_TransmitBufferReady,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SshEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SshEventParams)

	SshEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//..............................................................................

class FreeLibSsh2Session
{
public:
	void
	operator () (LIBSSH2_SESSION* session)
	{
		if (session)
			libssh2_session_free (session);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class FreeLibSsh2Channel
{
public:
	void
	operator () (LIBSSH2_CHANNEL* channel)
	{
		if (channel)
			libssh2_channel_free (channel);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef sl::Handle <LIBSSH2_SESSION*, FreeLibSsh2Session> SshSessionHandle;
typedef sl::Handle <LIBSSH2_CHANNEL*, FreeLibSsh2Channel> SshChannelHandle;

//..............................................................................

class SshChannel: public IfaceHdr
{
	friend class IoThread;

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, SshChannel, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoFlag
	{
		IoFlag_Connected    = 0x0001,
		IoFlag_Closing      = 0x0002,
		IoFlag_Connecting   = 0x0004,
		IoFlag_AuthError    = 0x0008,
		IoFlag_IncomingData = 0x0100,
	};

	struct Read: sl::ListLink
	{
		void* m_buffer;
		size_t m_size;
		size_t m_result;
		err::Error m_error;
		sys::Event m_completionEvent;
	};

	struct ConnectParams
	{
		sl::String m_userName;
		sl::String m_password;
		sl::String m_channelType;
		sl::String m_processType;
		sl::String m_ptyType;
		uint_t m_ptyWidth;
		uint_t m_ptyHeight;
	};

protected:
	bool m_isOpen;
	uint_t m_syncId;

	ClassBox <Multicast> m_onSshChannelEvent;

protected:
	Runtime* m_runtime;

	axl::io::Socket m_socket;
	SshSessionHandle m_sshSession;
	SshChannelHandle m_sshChannel;

	jnc::io::SocketAddress m_localAddress;
	jnc::io::SocketAddress m_remoteAddress;

	ConnectParams* m_connectParams;

	sys::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;

	sl::Array <char> m_readBuffer;
	size_t m_incomingDataSize;
	sl::AuxList <Read> m_readList;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	SshChannel ();

	~SshChannel ()
	{
		close ();
	}

	axl::io::SockAddr
	JNC_CDECL
	getAddress ();

	axl::io::SockAddr
	JNC_CDECL
	getPeerAddress ();

	bool
	JNC_CDECL
	open (DataPtr address);

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	connect (
		DataPtr address,
		DataPtr userName,
		DataPtr password,
		DataPtr channelType,
		DataPtr processType,
		DataPtr ptyType,
		uint_t ptyWidth,
		uint_t ptyHeight,
		bool isSync
		);

	bool
	JNC_CDECL
	authenticate (
		DataPtr userName,
		DataPtr password
		);

	bool
	JNC_CDECL
	resizePty (
		uint_t ptyWidth,
		uint_t ptyHeight,
		bool isSync
		);

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

protected:
	void
	fireSshEvent (
		SshEventCode eventCode,
		const err::ErrorHdr* error = NULL
		);

	void
	ioThreadFunc ();

	void
	wakeIoThread ();

	void
	sleepIoThread ();

	bool
	tcpConnect ();

	bool
	sshConnect ();

	void
	sshReadLoop ();

	err::Error
	getLastSshError ();

	int
	sshAsyncLoop (int result);
};

//..............................................................................

} // namespace io
} // namespace jnc
