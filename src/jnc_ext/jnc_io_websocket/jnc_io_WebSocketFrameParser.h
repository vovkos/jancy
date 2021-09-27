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

#include "jnc_io_WebSocketFrame.h"

namespace jnc {
namespace io {

//..............................................................................

class WebSocketFrameParser {
public:
	enum State {
		State_Header = 0,
		State_PayloadLength16,
		State_PayloadLength64,
		State_MaskKey,
		State_Payload,
		State_Completed,
	};

protected:
	enum Limits: uint64_t {
#if (JNC_PTR_SIZE == 8)
		MaxPayloadLength = 0x7fffffffffffffffULL,
#else
		MaxPayloadLength = 0x7fffffff,
#endif
	};

protected:
	WebSocketFrame* m_frame;
	sl::Array<char> m_buffer;
	State m_state;

public:
	WebSocketFrameParser(WebSocketFrame* frame) {
		m_frame = frame;
		m_state = State_Header;
	}

	State
	getState() {
		return m_state;
	}

	bool
	isCompleted() {
		return m_state >= State_Completed;
	}

	void
	reset();

	size_t
	parse(
		const void* p,
		size_t size
	);

protected:
	State
	getPayloadState() {
		return m_frame->m_payloadLength ? State_Payload : State_Completed;
	}

	State
	getMaskState() {
		return m_frame->m_mask ? State_MaskKey : getPayloadState();
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
