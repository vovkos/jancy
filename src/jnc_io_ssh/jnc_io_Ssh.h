#pragma once

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SshEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (SshChannel)

//.............................................................................

enum SshEventKind
{
	SshEventKind_TcpConnectCompleted = 0,
	SshEventKind_SshHandshakeCompleted,
	SshEventKind_SshAuthCompleted,
	SshEventKind_SshAuthError,
	SshEventKind_SshChannelOpened,
	SshEventKind_SshPtyRequested,
	SshEventKind_SshProcessStarted,
	SshEventKind_ConnectCompleted,
	SshEventKind_ConnectCancelled,
	SshEventKind_ConnectError,
	SshEventKind_Disconnected,
	SshEventKind_ReauthenticateInitiated,
	SshEventKind_ReconnectInitiated,
	SshEventKind_IncomingData,
	SshEventKind_TransmitBufferReady,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SshEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SshEventParams)

	SshEventKind m_eventKind;
	uint_t m_syncId;
	DataPtr m_errorPtr;
};

//.............................................................................

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef sl::Handle <LIBSSH2_SESSION*, FreeLibSsh2Session> SshSessionHandle;
typedef sl::Handle <LIBSSH2_CHANNEL*, FreeLibSsh2Channel> SshChannelHandle;

//.............................................................................

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
			AXL_CONTAINING_RECORD (this, SshChannel, m_ioThread)->ioThreadFunc ();
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

#if (_AXL_ENV == AXL_ENV_WIN)
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
	AXL_CDECL
	getAddress ();

	axl::io::SockAddr
	AXL_CDECL
	getPeerAddress ();

	bool
	AXL_CDECL
	open (DataPtr address);

	void
	AXL_CDECL
	close ();

	bool
	AXL_CDECL
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
	AXL_CDECL
	authenticate (
		DataPtr userName,
		DataPtr password
		);

	bool
	AXL_CDECL
	resizePty (
		uint_t ptyWidth,
		uint_t ptyHeight,
		bool isSync
		);

	size_t
	AXL_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		DataPtr ptr,
		size_t size
		);

protected:
	void
	fireSshEvent (
		SshEventKind eventKind,
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

//.............................................................................

} // namespace io
} // namespace jnc
