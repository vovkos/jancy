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
#include "jnc_io_WebSocketFrameParser.h"
#include "axl_enc_HexEncoding.h"

namespace jnc {
namespace io {

//..............................................................................

static
inline
size_t
bufferUntil(
	sl::Array<char>* buffer,
	size_t targetSize,
	const void* p,
	size_t size
	)
{
	size_t bufferSize = buffer->getCount();
	ASSERT(bufferSize < targetSize);

	size_t copySize = targetSize - bufferSize;
	if (copySize > size)
		copySize = size;

	buffer->append((char*)p, copySize);
	return copySize;
}

static
inline
void
mask(
	sl::Array<char>* buffer,
	uint32_t key
	)
{
	size_t size = buffer->getCount();
	buffer->reserve(sl::align<4>(size));
	mask(buffer->p(), size, key);
}

//..............................................................................

void
WebSocketFrameParser::reset()
{
	m_state = State_Header;
	m_buffer.clear();
}

size_t
WebSocketFrameParser::parse(
	const void* p0,
	size_t size
	)
{
	const char* p = (char*)p0;
	const char* end = p + size;

	while (p < end && m_state != State_Completed)
	{
		switch (m_state)
		{
		case State_Header:
			p += bufferUntil(&m_buffer, sizeof(WebSocketFrameHdr), p, end - p);
			if (m_buffer.getCount() < sizeof(WebSocketFrameHdr))
				return size;

			m_frame->clear();
			*(WebSocketFrameHdr*)m_frame = *(WebSocketFrameHdr*)m_buffer.cp();

			switch (m_frame->m_opcode)
			{
			case WebSocketFrameOpcode_Continuation:
			case WebSocketFrameOpcode_Text:
			case WebSocketFrameOpcode_Binary:
				break;

			case WebSocketFrameOpcode_Close:
			case WebSocketFrameOpcode_Ping:
			case WebSocketFrameOpcode_Pong:
				if (!m_frame->m_fin)
					return err::fail<size_t>(-1, "fragmented control frame");

				break;

			default:
				return err::fail<size_t>(-1, "unknown frame opcode");
			}

			switch (m_frame->m_lengthCode)
			{
			case 126:
				m_state = State_PayloadLength16;
				break;

			case 127:
				m_state = State_PayloadLength64;
				break;

			default:
				m_frame->m_payloadLength = m_frame->m_lengthCode;
				m_state = getMaskState();
			}

			m_buffer.clear();
			break;

		case State_PayloadLength16:
			p += bufferUntil(&m_buffer, sizeof(uint16_t), p, end - p);
			if (m_buffer.getCount() < sizeof(uint16_t))
				return size;

			m_frame->m_payloadLength = sl::swapByteOrder16(*(uint16_t*)m_buffer.cp());
			m_state = getMaskState();
			m_buffer.clear();
			break;

		case State_PayloadLength64:
			p += bufferUntil(&m_buffer, sizeof(uint64_t), p, end - p);
			if (m_buffer.getCount() < sizeof(uint64_t))
				return size;

			uint64_t payloadLength;
			payloadLength = sl::swapByteOrder64(*(uint64_t*)m_buffer.cp());
			if (payloadLength > MaxPayloadLength)
				return err::fail<size_t>(-1, "payload length too large");

			m_frame->m_payloadLength = (size_t)payloadLength;
			m_state = getMaskState();
			m_buffer.clear();
			break;

		case State_MaskKey:
			p += bufferUntil(&m_buffer, sizeof(uint32_t), p, end - p);
			if (m_buffer.getCount() < sizeof(uint32_t))
				return size;

			m_frame->m_maskKey = *(uint32_t*)m_buffer.cp();
			m_state = getPayloadState();
			m_buffer.clear();
			break;

		case State_Payload:
			p += bufferUntil(&m_frame->m_payload, m_frame->m_payloadLength, p, end - p);
			if (m_frame->m_payload.getCount() < m_frame->m_payloadLength)
				return size;

			if (m_frame->m_mask)
				mask(&m_frame->m_payload, m_frame->m_maskKey);

			m_state = State_Completed;
			break;
		}
	}

	return p - (char*)p0;
}

//..............................................................................

} // namespace io
} // namespace jnc
