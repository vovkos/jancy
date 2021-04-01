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
	WebSocketState_Handshake = 0,
	WebSocketState_HandshakeReady,
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

public:
	WebSocketStateMachine();

	~WebSocketStateMachine()
	{
		reset();
	}

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
	reset();

	void
	setConnectedState();

	size_t
	parse(
		const void* p,
		size_t size
		);

protected:
	bool
	processFrame();
};

//..............................................................................

} // namespace io
} // namespace jnc
