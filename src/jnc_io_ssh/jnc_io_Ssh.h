#pragma once

#include "jnc_io_SshLibGlobals.h"
#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

class SshChannel;

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
	SshEventKind_TransmitBufferOverflow,
	SshEventKind_TransmitBufferReady,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SshEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.SshEventParams", g_sshLibCacheSlot, SshLibTypeCacheSlot_SshEventParams)
	JNC_END_TYPE_MAP ()

	SshEventKind m_eventKind;
	uint_t m_syncId;
	rt::DataPtr m_errorPtr;
};

//.............................................................................

class SshChannel: public rt::IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (SshChannel, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.SshChannel", g_sshLibCacheSlot, SshLibTypeCacheSlot_SshChannel)
		JNC_MAP_CONSTRUCTOR (&sl::construct <SshChannel>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <SshChannel>)
		JNC_MAP_CONST_PROPERTY ("m_address",     &SshChannel::getAddress)
		JNC_MAP_CONST_PROPERTY ("m_peerAddress", &SshChannel::getPeerAddress)
		JNC_MAP_FUNCTION ("open",         &SshChannel::open)
		JNC_MAP_FUNCTION ("close",        &SshChannel::close)
		JNC_MAP_FUNCTION ("connect",      &SshChannel::connect)
		JNC_MAP_FUNCTION ("authenticate", &SshChannel::authenticate)
		JNC_MAP_FUNCTION ("resizePty",    &SshChannel::resizePty)
		JNC_MAP_FUNCTION ("read",         &SshChannel::read)
		JNC_MAP_FUNCTION ("write",        &SshChannel::write)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class IoThread: public mt::ThreadImpl <IoThread>
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
		mt::Event m_completionEvent;
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

	rt::ClassBox <rt::Multicast> m_onSshChannelEvent;

protected:
	rt::Runtime* m_runtime;
	
	axl::io::Socket m_socket;
	SshSessionHandle m_sshSession;
	SshChannelHandle m_sshChannel;

	jnc::io::SocketAddress m_localAddress;
	jnc::io::SocketAddress m_remoteAddress;

	ConnectParams* m_connectParams;

	mt::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;

	sl::Array <char> m_readBuffer;
	size_t m_incomingDataSize;
	sl::AuxList <Read> m_readList;

#if (_AXL_ENV == AXL_ENV_WIN)
	mt::Event m_ioThreadEvent;
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
	open (rt::DataPtr address);

	void
	AXL_CDECL
	close ();

	bool
	AXL_CDECL
	connect (
		rt::DataPtr address,
		rt::DataPtr userName,
		rt::DataPtr password,
		rt::DataPtr channelType,
		rt::DataPtr processType,
		rt::DataPtr ptyType,
		uint_t ptyWidth,
		uint_t ptyHeight,
		bool isSync
		);

	bool
	AXL_CDECL
	authenticate (
		rt::DataPtr userName,
		rt::DataPtr password
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
		rt::DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		rt::DataPtr ptr,
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
