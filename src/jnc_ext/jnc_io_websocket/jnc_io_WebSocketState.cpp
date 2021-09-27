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

#include "pch.h"
#include "jnc_io_WebSocketState.h"
#include "jnc_io_WebSocketHandshakeParser.h"
#include "jnc_io_WebSocketFrameParser.h"

namespace jnc {
namespace io {

//..............................................................................

WebSocketStateMachine::WebSocketStateMachine() {
	m_state = WebSocketState_Idle;
	m_handshakeParser = NULL;
	m_frameParser = NULL;
}

WebSocketStateMachine::~WebSocketStateMachine() {
	if (m_handshakeParser)
		AXL_MEM_DELETE(m_handshakeParser);

	if (m_frameParser)
		AXL_MEM_DELETE(m_frameParser);
}

void
WebSocketStateMachine::setHandshake(
	WebSocketHandshake* handshake,
	const sl::StringRef& handshakeKey
) {
	if (m_handshakeParser) {
		AXL_MEM_DELETE(m_handshakeParser);
		m_handshakeParser = NULL;
	}

	if (m_frameParser) {
		AXL_MEM_DELETE(m_frameParser);
		m_frameParser = NULL;
	}

	m_handshake = handshake;
	m_frame.clear();
	m_message.clear();
	m_handshakeKey = handshakeKey;

	m_state = handshakeKey.isEmpty() ?
		WebSocketState_WaitingHandshake :
		WebSocketState_WaitingHandshakeResponse;
}

void
WebSocketStateMachine::setConnectedState() {
	ASSERT(
		m_state == WebSocketState_HandshakeReady ||
		m_state == WebSocketState_HandshakeResponseReady ||
		m_state == WebSocketState_ControlFrameReady ||
		m_state == WebSocketState_MessageReady
	);

	if (m_frameParser)
		m_frameParser->reset();

	m_frame.clear();
	m_message.clear();
	m_state = WebSocketState_Connected;
}

size_t
WebSocketStateMachine::parse(
	const void* p0,
	size_t size
) {
	size_t result;

	if (m_state == WebSocketState_WaitingHandshake ||
		m_state == WebSocketState_WaitingHandshakeResponse) {
		if (!m_handshakeParser)
			m_handshakeParser = AXL_MEM_NEW_ARGS(WebSocketHandshakeParser, (m_handshake, m_handshakeKey));

		result = m_handshakeParser->parse(p0, size);
		if (result == -1)
			return -1;

		if (m_handshakeParser->isCompleted()) {
			m_state = (WebSocketState)(m_state + 2); // WebSocketState_HandshakeReady/ResponseReady
			AXL_MEM_DELETE(m_handshakeParser);
			m_handshakeParser = NULL;
		}

		return result;
	}

	ASSERT(m_state == WebSocketState_Connected); // wrong use otherwise -- missing setConnectedState()

	if (!m_frameParser)
		m_frameParser = AXL_MEM_NEW_ARGS(WebSocketFrameParser, (&m_frame));

	const char* p = (char*)p0;
	const char* end = p + size;

	while (p < end && m_state == WebSocketState_Connected) {
		result = m_frameParser->parse(p, end - p);
		if (result == -1)
			return -1;

		p += result;

		if (!m_frameParser->isCompleted())
			return size;

		m_frameParser->reset();
		result = processFrame();
		if (!result)
			return -1;
	}

	return p - (char*)p0;
}

bool
WebSocketStateMachine::processFrame() {
	if (m_frame.m_opcode >= WebSocketFrameOpcode_FirstControl) {
		m_state = WebSocketState_ControlFrameReady;
		return true;
	}

	if (m_frame.m_opcode == WebSocketFrameOpcode_Continuation) {
		if (m_message.m_opcode == WebSocketFrameOpcode_Undefined)
			return err::fail("continuation frame without initial data frame");

		m_message.m_frameCount++;
		m_message.m_payload.append(m_frame.m_payload);
	} else {
		if (m_message.m_opcode != WebSocketFrameOpcode_Undefined)
			return err::fail("incomplete fragmented message");

		m_message.m_opcode = (WebSocketFrameOpcode)m_frame.m_opcode;
		m_message.m_frameCount = 1;
		m_message.m_payload = m_frame.m_payload;
	}

	if (m_frame.m_fin)
		m_state = WebSocketState_MessageReady;

	return true;
}

//..............................................................................

} // namespace io
} // namespace jnc
