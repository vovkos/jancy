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

enum WebSocketHandshakeStdHeader
{
	WebSocketHandshakeStdHeader_UserAgent,
	WebSocketHandshakeStdHeader_Host,
	WebSocketHandshakeStdHeader_Origin,
	WebSocketHandshakeStdHeader_Connection,
	WebSocketHandshakeStdHeader_Upgrade,
	WebSocketHandshakeStdHeader_WebSocketVersion,
	WebSocketHandshakeStdHeader_WebSocketExtension,
	WebSocketHandshakeStdHeader_WebSocketProtocol,
	WebSocketHandshakeStdHeader_WebSocketKey,
	WebSocketHandshakeStdHeader_WebSocketAccept,
	WebSocketHandshakeStdHeader__Count,

	WebSocketHandshakeStdHeader_Undefined = WebSocketHandshakeStdHeader__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE_IGNORE_CASE_PCU(WebSocketHandshakeStdHeaderMap, WebSocketHandshakeStdHeader)
	AXL_SL_HASH_TABLE_ENTRY("User-Agent",               WebSocketHandshakeStdHeader_UserAgent)
	AXL_SL_HASH_TABLE_ENTRY("Host",                     WebSocketHandshakeStdHeader_Host)
	AXL_SL_HASH_TABLE_ENTRY("Origin",                   WebSocketHandshakeStdHeader_Origin)
	AXL_SL_HASH_TABLE_ENTRY("Connection",               WebSocketHandshakeStdHeader_Connection)
	AXL_SL_HASH_TABLE_ENTRY("Upgrade",                  WebSocketHandshakeStdHeader_Upgrade)
	AXL_SL_HASH_TABLE_ENTRY("Sec-WebSocket-Version",    WebSocketHandshakeStdHeader_WebSocketVersion)
	AXL_SL_HASH_TABLE_ENTRY("Sec-WebSocket-Extensions", WebSocketHandshakeStdHeader_WebSocketExtension)
	AXL_SL_HASH_TABLE_ENTRY("Sec-WebSocket-Protocol",   WebSocketHandshakeStdHeader_WebSocketProtocol)
	AXL_SL_HASH_TABLE_ENTRY("Sec-WebSocket-Key",        WebSocketHandshakeStdHeader_WebSocketKey)
	AXL_SL_HASH_TABLE_ENTRY("Sec-WebSocket-Accept",     WebSocketHandshakeStdHeader_WebSocketAccept)
AXL_SL_END_STRING_HASH_TABLE_IGNORE_CASE_PCU()

//..............................................................................

struct WebSocketHeader
{
	sl::String m_firstValue;
	sl::BoxList<sl::String> m_extraValueList;

	bool
	isEmpty()
	{
		return m_firstValue.isEmpty();
	}

	void
	clear();

	void
	add(const sl::String& value);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketHandshake
{
	sl::String m_resource;
	sl::String m_httpVersion;
	uint_t m_statusCode;
	sl::String m_statusString;
	WebSocketHeader* m_stdHeaderTable[WebSocketHandshakeStdHeader__Count];
	sl::StringHashTable<WebSocketHeader> m_headerMap;

	void
	clear();
};

//..............................................................................

class WebSocketHandshakeParser
{
public:
	enum State
	{
		State_Idle = 0,
		State_RequestLine,
		State_ResponseLine,
		State_HeaderLine,
		State_Completed,
	};

protected:
	enum // limits from Apache, QWebSocket
	{
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
	getState()
	{
		return m_state;
	}

	bool
	isCompleted()
	{
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
	sl::String* handshake,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	const sl::StringHashTable<WebSocketHeader>* extraHeaderMap = NULL
	);

inline
sl::String
buildWebSocketHandshake(
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringRef& key,
	const sl::StringHashTable<WebSocketHeader>* extraHeaderMap = NULL
	)
{
	sl::String handshake;
	buildWebSocketHandshake(&handshake, resource, host, key, extraHeaderMap);
	return handshake;
}

size_t
buildWebSocketHandshakeResponse(
	sl::String* response,
	const WebSocketHandshake& handshake
	);

inline
sl::String
buildWebSocketHandshakeResponse(const WebSocketHandshake& handshake)
{
	sl::String response;
	buildWebSocketHandshakeResponse(&response, handshake);
	return response;
}

//..............................................................................

} // namespace io
} // namespace jnc
