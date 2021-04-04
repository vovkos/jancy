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

enum WebSocketOpcode
{
	WebSocketOpcode_Continuation = 0x0,
	WebSocketOpcode_Text         = 0x1,
	WebSocketOpcode_Binary       = 0x2,
	WebSocketOpcode_Close        = 0x8,
	WebSocketOpcode_Ping         = 0x9,
	WebSocketOpcode_Pong         = 0xa,
	WebSocketOpcode_FirstControl = WebSocketOpcode_Close,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketFrameHdr
{
	union
	{
		struct
		{
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

struct WebSocketFrame: WebSocketFrameHdr
{
	size_t m_payloadLength;
	uint32_t m_maskKey;
	sl::Array<char> m_payload;

	WebSocketFrame()
	{
		clear();
	}

	void
	clear();
};

//..............................................................................

enum WebSocketMessageType
{
	WebSocketMessageType_Undefined = 0,
	WebSocketMessageType_Text,
	WebSocketMessageType_Binary
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketMessage
{
	WebSocketMessageType m_type;
	size_t m_frameCount;
	sl::Array<char> m_payload;

	WebSocketMessage()
	{
		clear();
	}

	void
	clear();
};

//..............................................................................

class WebSocketFrameParser
{
public:
	enum State
	{
		State_Header = 0,
		State_PayloadLength16,
		State_PayloadLength64,
		State_MaskKey,
		State_Payload,
		State_Completed,
	};

protected:
	enum Limits: uint64_t
	{
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
	WebSocketFrameParser(WebSocketFrame* frame)
	{
		m_frame = frame;
		m_state = State_Header;
	}

	State
	getState()
	{
		return m_state;
	}

	bool
	isCompleted()
	{
		return m_state >= State_Completed;
	}

	void
	reset();

	size_t
	parse(
		const void* p,
		size_t size
		);
};

//..............................................................................

size_t
buildWebSocketFrame(
	sl::Array<char>* buffer,
	WebSocketOpcode opcode,
	bool isFinal,
	bool isMasked,
	const void* p,
	size_t size
	);

inline
sl::Array<char>
buildWebSocketFrame(
	WebSocketOpcode opcode,
	bool isFinal,
	bool isMasked,
	const void* p,
	size_t size
	)
{
	sl::Array<char> buffer;
	buildWebSocketFrame(&buffer, opcode, isFinal, isMasked, p, size);
	return buffer;
}

//..............................................................................

} // namespace io
} // namespace jnc
