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
	JNC_MAP_CONST_PROPERTY("m_rawData", &WebSocketHandshake::getRawData)
JNC_END_TYPE_FUNCTION_MAP()

JNC_BEGIN_CLASS_TYPE_VTABLE(WebSocketHandshake)
JNC_END_CLASS_TYPE_VTABLE()

//..............................................................................

WebSocketHandshake::WebSocketHandshake() {
	m_httpVersion = 0;
	m_statusCode = 0;
	sl::construct(m_headers.p()); // already primed (non-opaque class field)
}

WebSocketHandshake::~WebSocketHandshake() {
	sl::destruct(m_headers.p());
}

void
JNC_CDECL
WebSocketHandshake::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	m_resource.markGcRoots(gcHeap);
	m_reasonPhrase.markGcRoots(gcHeap);
	m_rawData.markGcRoots(gcHeap);
	m_headers->markOpaqueGcRoots(gcHeap);
}

void
WebSocketHandshake::clear() {
	m_resource.clear();
	m_reasonPhrase.clear();
	m_rawData.clear();
	m_statusCode = 0;
	m_headers->clear();
}

size_t
WebSocketHandshake::buildRequest(
	sl::String* resultString,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	WebSocketHandshakeHeaders* extraHeaders
) {
	resultString->format(
		"GET %s HTTP/1.1\r\n",
		resource.sz(),
		host.sz(),
		key.sz()
	);

	m_httpVersion = 0x0101;
	m_resource = resource;

	m_headers->addImpl("Host", host);
	m_headers->addImpl("Connection", "Upgrade");
	m_headers->addImpl("Upgrade", "websocket");
	m_headers->addImpl("Sec-WebSocket-Version", "13");
	m_headers->addImpl("Sec-WebSocket-Key", key);

	return finalizeBuild(resultString, extraHeaders);
}

size_t
WebSocketHandshake::buildResponse(
	sl::String* resultString,
	WebSocketHandshake* handshakeRequest,
	WebSocketHandshakeHeaders* extraHeaders
) {
	static const char ReasonPhrase[] = "Switching Protocols";

	uchar_t hash[SHA_DIGEST_LENGTH];
	WebSocketHandshakeHeader* keyHeader = handshakeRequest->m_headers->getStdHeader(WebSocketHandshakeStdHeader_WebSocketKey);
	calcWebSocketHandshakeKeyHash(hash, keyHeader->m_firstValue);
	sl::String acceptKey = enc::Base64Encoding::encode(hash, sizeof(hash));

	resultString->format(
		"HTTP/1.1 101 %s\r\n",
		ReasonPhrase
	);

	m_httpVersion = 0x0101;
	m_statusCode = 101;
	m_reasonPhrase = ReasonPhrase;

	m_headers->addImpl("Connection", "Upgrade");
	m_headers->addImpl("Upgrade", "websocket");
	m_headers->addImpl("Sec-WebSocket-Version", "13");
	m_headers->addImpl("Sec-WebSocket-Accept", acceptKey);

	return finalizeBuild(resultString, extraHeaders);
}

size_t
WebSocketHandshake::finalizeBuild(
	sl::String* resultString,
	WebSocketHandshakeHeaders* extraHeaders
) {
	if (extraHeaders)
		m_headers->addImpl(extraHeaders);

	m_headers->appendFormat(resultString);
	resultString->append("\r\n");
	m_rawData = *resultString;
	return resultString->getLength();
}

//..............................................................................

size_t
generateWebSocketHandshakeKey(sl::String* key) {
	uchar_t keyValue[16];
	::RAND_bytes(keyValue, sizeof(keyValue));
	return enc::Base64Encoding::encode(key, keyValue, sizeof(keyValue));
}

void
calcWebSocketHandshakeKeyHash(
	uchar_t hash[SHA_DIGEST_LENGTH],
	const sl::StringRef& key
) {
	static const char WebSocketUuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha, key.cp(), key.getLength());
	SHA1_Update(&sha, WebSocketUuid, lengthof(WebSocketUuid));
	SHA1_Final(hash, &sha);
}

//..............................................................................

} // namespace io
} // namespace jnc
