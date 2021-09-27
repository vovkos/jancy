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

namespace jnc {
namespace io {

//..............................................................................

class WebSocketHandshakeParser {
public:
	enum State {
		State_Idle = 0,
		State_RequestLine,
		State_ResponseLine,
		State_HeaderLine,
		State_Completed,
	};

protected:
	enum { // limits from Apache, QWebSocket
		MaxLineLength  = 8 * 1024,
		MaxHeaderCount = 100,
	};

protected:
	WebSocketHandshake* m_handshake;
	sl::String m_line;
	State m_state;
	sl::String m_key;

public:
	WebSocketHandshakeParser(
		WebSocketHandshake* handshake,
		const sl::StringRef& key
	);

	State
	getState() {
		return m_state;
	}

	bool
	isCompleted() {
		return m_state >= State_Completed;
	}

	size_t
	parse(
		const void* p,
		size_t size
	);

protected:
	size_t
	bufferLine(
		const char* p,
		size_t size
	);

	bool
	parseRequestLine();

	bool
	parseResponseLine();

	bool
	parseHeaderLine();

	bool
	finalize();

	bool
	verifyAccept();
};

//..............................................................................

} // namespace io
} // namespace jnc
