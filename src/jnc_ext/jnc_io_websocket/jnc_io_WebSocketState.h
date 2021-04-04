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

#include "jnc_io_WebSocketHandshake.h"
#include "jnc_io_WebSocketFrame.h"

namespace jnc {
namespace io {

//..............................................................................

enum WebSocketState
{
	WebSocketState_Idle = 0,
	WebSocketState_WaitingHandshake,
	WebSocketState_WaitingHandshakeResponse,
	WebSocketState_HandshakeReady,
	WebSocketState_HandshakeResponseReady,
	WebSocketState_Connected,
	WebSocketState_ControlFrameReady,
	WebSocketState_MessageReady,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class WebSocketStateMachine
{
protected:
	WebSocketState m_state;
	WebSocketHandshakeParser* m_handshakeParser;
	WebSocketFrameParser* m_frameParser;
	WebSocketHandshake m_handshake;
	WebSocketFrame m_frame;
	WebSocketMessage m_message;

	sl::String m_handshakeKey;

public:
	WebSocketStateMachine();
	~WebSocketStateMachine();

	WebSocketState
	getState()
	{
		return m_state;
	}

	const WebSocketHandshake&
	getHandshake()
	{
		return m_handshake;
	}

	const WebSocketFrame&
	getFrame()
	{
		return m_frame;
	}

	const WebSocketMessage&
	getMessage()
	{
		return m_message;
	}

	void
	waitHandshake()
	{
		reset(sl::StringRef());
	}

	void
	waitHandshakeResponse(const sl::StringRef& handshakeKey)
	{
		reset(handshakeKey);
	}

	void
	setConnectedState();

	size_t
	parse(
		const void* p,
		size_t size
		);

protected:
	void
	reset(const sl::StringRef& handshakeKey);

	bool
	processFrame();
};

//..............................................................................

} // namespace io
} // namespace jnc
