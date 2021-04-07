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
#include "jnc_io_WebSocketHandshake.h"
#include "jnc_io_WebSocketLib.h"

namespace jnc {
namespace io {

static const char WebSocketUuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	WebSocketHandshake,
	"io.WebSocketHandshake",
	g_webSocketLibGuid,
	WebSocketLibCacheSlot_WebSocketHandshake,
	WebSocketHandshake,
	&WebSocketHandshake::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(WebSocketHandshake)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<WebSocketHandshake>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<WebSocketHandshake>)

	JNC_MAP_CONST_PROPERTY("m_resource", &WebSocketHandshake::getResource)
	JNC_MAP_CONST_PROPERTY("m_reasonPhrase", &WebSocketHandshake::getReasonPhrase)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

WebSocketHandshake::WebSocketHandshake()
{
	m_httpVersion = 0;
	m_statusCode = 0;
	jnc::primeClass(m_box->m_type->getModule(), &m_headers);
	sl::construct(m_headers.p());
	m_publicHeaders = m_headers;
}

void
JNC_CDECL
WebSocketHandshake::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	m_resource.markGcRoots(gcHeap);
	m_reasonPhrase.markGcRoots(gcHeap);
	m_headers->markOpaqueGcRoots(gcHeap);
}

void
WebSocketHandshake::clear()
{
	m_resource.clear();
	m_reasonPhrase.clear();
	m_statusCode = 0;
	m_headers->clear();
}

//..............................................................................

size_t
generateWebSocketHandshakeKey(sl::String* key)
{
	uchar_t keyValue[16];
	::RAND_bytes(keyValue, sizeof(keyValue));
	return enc::Base64Encoding::encode(key, keyValue, sizeof(keyValue));
}

void
calcWebSocketHandshakeKeyHash(
	uchar_t hash[SHA_DIGEST_LENGTH],
	const sl::StringRef& key
	)
{
	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha, key.cp(), key.getLength());
	SHA1_Update(&sha, WebSocketUuid, lengthof(WebSocketUuid));
	SHA1_Final(hash, &sha);
}

size_t
buildWebSocketHandshake(
	sl::String* resultString,
	WebSocketHandshake* resultHandshake,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	const WebSocketHandshakeHeaders* extraHeaders
	)
{
	resultString->format(
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Sec-WebSocket-Key: %s\r\n",
		resource.sz(),
		host.sz(),
		key.sz()
	);

	sl::ConstStringHashTableIterator<WebSocketHandshakeHeader> it = extraHeaders ? extraHeaders->getHeaderMap().getHead() : NULL;
	for (; it; it++)
	{
		resultString->appendFormat("%s: %s\n", it->getKey().sz(), it->m_value.m_firstValue.sz());

		sl::ConstBoxIterator<DualString> it2 = it->m_value.m_extraValueList.getHead();
		for (; it2; it2++)
			resultString->appendFormat("%s: %s\n", it->getKey().sz(), it2->sz());
	}

	return resultString->append("\r\n");
}

size_t
buildWebSocketHandshakeResponse(
	sl::String* resultString,
	WebSocketHandshake* resultHandshakeResponse,
	const WebSocketHandshake* handshakeRequest
	)
{
	uchar_t hash[SHA_DIGEST_LENGTH];
	WebSocketHandshakeHeader* keyHeader = handshakeRequest->m_publicHeaders->getStdHeader(WebSocketHandshakeStdHeader_WebSocketKey);
	calcWebSocketHandshakeKeyHash(hash, keyHeader->m_firstValue);

	char buffer[256];
	sl::String acceptKey(ref::BufKind_Stack, buffer, sizeof(buffer));
	enc::Base64Encoding::encode(&acceptKey, hash, sizeof(hash));

	return resultString->format(
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %s\r\n"
		"\r\n",
		acceptKey.sz()
		);
}

//..............................................................................

} // namespace io
} // namespace jnc
