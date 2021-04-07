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
#include "jnc_io_WebSocketFrame.h"
#include "axl_enc_HexEncoding.h"

namespace jnc {
namespace io {

//..............................................................................

void
WebSocketFrame::clear()
{
	m_header = 0;
	m_payloadLength = 0;
	m_maskKey = 0;
	m_payload.clear();
}

//..............................................................................

void
WebSocketMessage::clear()
{
	m_opcode = WebSocketFrameOpcode_Undefined;
	m_frameCount = 0;
	m_payload.clear();
}

//..............................................................................

size_t
buildWebSocketFrame(
	sl::Array<char>* buffer,
	WebSocketFrameOpcode opcode,
	bool isFinal,
	bool isMasked,
	const void* payload,
	size_t payloadSize
	)
{
	size_t reserveSize =
		sizeof(WebSocketFrameHdr) +
		sizeof(uint64_t) +
		sizeof(uint32_t) +
		sl::align<4>(payloadSize);

	bool result = buffer->setCountZeroConstruct(reserveSize);
	if (!result)
		return -1;

	WebSocketFrameHdr* hdr = (WebSocketFrameHdr*)buffer->p();
	hdr->m_opcode = opcode;
	hdr->m_fin = isFinal;
	hdr->m_mask = isMasked;

	char* p = (char*)(hdr + 1);

	if (payloadSize < 126)
	{
		hdr->m_lengthCode = (uint8_t) payloadSize;
	}
	else if (payloadSize < 65536)
	{
		hdr->m_lengthCode = 126;
		*(uint16_t*)p = sl::swapByteOrder16((uint16_t)payloadSize);
		p += sizeof(uint16_t);
	}
	else
	{
		hdr->m_lengthCode = 127;
		*(uint64_t*)p = sl::swapByteOrder64(payloadSize);
		p += sizeof(uint64_t);
	}

	if (!isMasked)
	{
		memcpy(p, payload, payloadSize);
	}
	else
	{
		::RAND_bytes((uchar_t*)p, sizeof(uint32_t));
		uint32_t key = *(uint32_t*)p;
		p += sizeof(uint32_t);
		memcpy(p, payload, payloadSize);
		mask(p, payloadSize, key);
	}

	size_t frameSize = p + payloadSize - buffer->p();
	buffer->setCount(frameSize);
	return frameSize;
}

//..............................................................................

} // namespace io
} // namespace jnc
