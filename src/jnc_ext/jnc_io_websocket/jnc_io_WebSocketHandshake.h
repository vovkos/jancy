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

class WebSocketHandshake: IfaceHdr
{
	friend class WebSocketHandshakeParser;
	friend class WebSocketHandshakeBuilder;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(WebSocketHandshake)

public:
	uint_t m_httpVersion;
	uint_t m_statusCode;
	WebSocketHandshakeHeaders* m_publicHeaders;

protected:
	DualString m_resource;
	DualString m_reasonPhrase;
	ClassBox<WebSocketHandshakeHeaders> m_headers;

public:
	WebSocketHandshake();

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getResource(WebSocketHandshake* self)
	{
		return self->m_resource.getPtr();
	}

	static
	DataPtr
	JNC_CDECL
	getReasonPhrase(WebSocketHandshake* self)
	{
		return self->m_reasonPhrase.getPtr();
	}

	void
	clear();
};

//..............................................................................

size_t
generateWebSocketHandshakeKey(sl::String* key);

inline
sl::String
generateWebSocketHandshakeKey()
{
	sl::String key;
	generateWebSocketHandshakeKey(&key);
	return key;
}

void
calcWebSocketHandshakeKeyHash(
	uchar_t hash[SHA_DIGEST_LENGTH],
	const sl::StringRef& key
	);

size_t
buildWebSocketHandshake(
	sl::String* resultString,
	WebSocketHandshake* resultHandshake,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	const WebSocketHandshakeHeaders* extraHeaders = NULL
	);

sl::String
buildWebSocketHandshake(
	WebSocketHandshake* resultHandshake,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	const WebSocketHandshakeHeaders* extraHeaders = NULL
	)
{
	sl::String string;
	buildWebSocketHandshake(&string, resultHandshake, resource, host, key, extraHeaders);
	return string;
}

size_t
buildWebSocketHandshakeResponse(
	sl::String* resultString,
	WebSocketHandshake* resultHandshakeResponse,
	const WebSocketHandshake* handshakeRequest
	);

sl::String
buildWebSocketHandshakeResponse(
	WebSocketHandshake* resultHandshakeResponse,
	const WebSocketHandshake* handshakeRequest
	)
{
	sl::String string;
	buildWebSocketHandshakeResponse(&string, resultHandshakeResponse, handshakeRequest);
	return string;
}

//..............................................................................

} // namespace io
} // namespace jnc
