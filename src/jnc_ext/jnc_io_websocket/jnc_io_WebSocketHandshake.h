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

#include "jnc_io_WebSocketHandshakeHeaders.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(WebSocketHandshake)

//..............................................................................

class WebSocketHandshake: IfaceHdr {
	friend class WebSocketHandshakeParser;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(WebSocketHandshake)

public:
	uint_t m_httpVersion;
	uint_t m_statusCode;
	ClassBox<WebSocketHandshakeHeaders> m_headers;

protected:
	DualString m_resource;
	DualString m_reasonPhrase;
	DualString m_rawData;

public:
	WebSocketHandshake();
	~WebSocketHandshake();

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	String
	JNC_CDECL
	getResource(WebSocketHandshake* self) {
		return self->m_resource;
	}

	static
	String
	JNC_CDECL
	getReasonPhrase(WebSocketHandshake* self) {
		return self->m_reasonPhrase;
	}

	static
	String
	JNC_CDECL
	getRawData(WebSocketHandshake* self) {
		return self->m_rawData;
	}

	void
	clear();

	size_t
	buildRequest(
		sl::String* resultString,
		const sl::StringRef& resource,
		const sl::StringRef& host,
		const sl::StringRef& key,
		WebSocketHandshakeHeaders* extraHeaders = NULL
	);

	size_t
	buildResponse(
		sl::String* resultString,
		uint_t statusCode,
		const sl::StringRef& reasonPhrase,
		WebSocketHandshake* handshake,
		WebSocketHandshakeHeaders* extraHeaders = NULL
	);

	size_t
	buildResponse(
		sl::String* resultString,
		WebSocketHandshake* handshake,
		WebSocketHandshakeHeaders* extraHeaders = NULL
	) {
		return buildResponse(resultString, 101, "Switching Protocols", handshake, extraHeaders);
	}

	sl::String
	buildRequest(
		const sl::StringRef& resource,
		const sl::StringRef& host,
		const sl::StringRef& key,
		WebSocketHandshakeHeaders* extraHeaders = NULL
	) {
		sl::String string;
		buildRequest(&string, resource, host, key, extraHeaders);
		return string;
	}

	sl::String
	buildResponse(
		WebSocketHandshake* handshake,
		WebSocketHandshakeHeaders* extraHeaders = NULL
	) {
		sl::String string;
		buildResponse(&string, handshake, extraHeaders);
		return string;
	}

protected:
	size_t
	finalizeBuild(
		sl::String* resultString,
		WebSocketHandshakeHeaders* extraHeaders
	);
};

//..............................................................................

size_t
generateWebSocketHandshakeKey(sl::String* key);

inline
sl::String
generateWebSocketHandshakeKey() {
	sl::String key;
	generateWebSocketHandshakeKey(&key);
	return key;
}

void
calcWebSocketHandshakeKeyHash(
	uchar_t hash[SHA_DIGEST_LENGTH],
	const sl::StringRef& key
);

//..............................................................................

} // namespace io
} // namespace jnc
