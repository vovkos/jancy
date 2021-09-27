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

namespace jnc {
namespace io {

//..............................................................................

enum WebSocketFrameOpcode {
	WebSocketFrameOpcode_Continuation = 0x0,
	WebSocketFrameOpcode_Text         = 0x1,
	WebSocketFrameOpcode_Binary       = 0x2,
	WebSocketFrameOpcode_Close        = 0x8,
	WebSocketFrameOpcode_Ping         = 0x9,
	WebSocketFrameOpcode_Pong         = 0xa,
	WebSocketFrameOpcode_FirstControl = WebSocketFrameOpcode_Close,
	WebSocketFrameOpcode_Undefined    = -1,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketFrameHdr {
	union {
		struct {
			uchar_t m_opcode     : 4;
			uchar_t m_reserved   : 3;
			uchar_t m_fin        : 1;
			uchar_t m_lengthCode : 7;
			uchar_t m_mask       : 1;
		};

		ushort_t m_header;
	};
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketFrame: WebSocketFrameHdr {
	size_t m_payloadLength;
	uint32_t m_maskKey;
	sl::Array<char> m_payload;

	WebSocketFrame() {
		clear();
	}

	void
	clear();
};

//..............................................................................

struct WebSocketMessage {
	WebSocketFrameOpcode m_opcode;
	size_t m_frameCount;
	sl::Array<char> m_payload;

	WebSocketMessage() {
		clear();
	}

	void
	clear();
};

//..............................................................................

inline
void
mask(
	void* buffer,
	size_t size,
	uint32_t key
) {
	uint32_t* p = (uint32_t*)buffer;
	uint32_t* end = (uint32_t*)((char*)buffer + size);

	for (; p < end; p++)
		*p ^= key;
}

size_t
buildWebSocketFrame(
	sl::Array<char>* buffer,
	WebSocketFrameOpcode opcode,
	bool isFinal,
	bool isMasked,
	const void* p,
	size_t size
);

inline
sl::Array<char>
buildWebSocketFrame(
	WebSocketFrameOpcode opcode,
	bool isFinal,
	bool isMasked,
	const void* p,
	size_t size
) {
	sl::Array<char> buffer;
	buildWebSocketFrame(&buffer, opcode, isFinal, isMasked, p, size);
	return buffer;
}

//..............................................................................

} // namespace io
} // namespace jnc
