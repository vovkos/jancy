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

namespace jnc {
namespace io {

static const char WebSocketUuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//..............................................................................

void
WebSocketHeader::clear()
{
	m_firstValue.clear();
	m_extraValueList.clear();
}

void
WebSocketHeader::add(const sl::String& value)
{
	if (m_firstValue.isEmpty())
		m_firstValue = value;
	else
		m_extraValueList.insertTail(value);
}

//..............................................................................

void
WebSocketHandshake::clear()
{
	m_resource.clear();
	m_httpVersion.clear();
	m_headerMap.clear();
	memset(m_stdHeaderTable, 0, sizeof(m_stdHeaderTable));
}

//..............................................................................

WebSocketHandshakeParser::WebSocketHandshakeParser(WebSocketHandshake* handshake)
{
	handshake->clear();
	m_handshake = handshake;
	m_state = State_RequestLine;
}

size_t
WebSocketHandshakeParser::bufferLine(
	const char* p,
	size_t size
	)
{
	char* lf = (char*)memchr(p, '\n', size);
	if (lf)
		size = lf - p + 1;

	if (m_line.getLength() + size > MaxLineLength)
		return err::fail<size_t>(-1, "HTTP request line length limit exceeded");

	m_line.append(p, size);
	return size;
}

size_t
WebSocketHandshakeParser::parse(
	const void* p0,
	size_t size
	)
{
	const char* p = (char*)p0;
	const char* end = p + size;

	while (p < end && m_state != State_Completed)
	{
		size_t result = bufferLine(p, end - p);
		if (result == -1)
			return -1;

		p += result;

		if (!m_line.isSuffix('\n')) // line completed
			break;

		m_line.trim();

		switch (m_state)
		{
		case State_RequestLine:
			result = parseRequestLine();
			break;

		case State_HeaderLine:
			result = parseHeaderLine();
			break;

		default:
			ASSERT(false);
		}

		if (!result)
			return -1;

		m_line.clear();
	}

	return p - (char*)p0;
}

bool
WebSocketHandshakeParser::parseRequestLine()
{
	size_t delim = m_line.find(' ');
	if (delim == -1)
		return err::fail("invalid HTTP request line: missing resource");

	sl::StringRef verb = m_line.getLeftSubString(delim);
	if (verb != "GET")
		return err::fail("invalid HTTP request: unexpected verb");

	m_line.remove(0, delim + 1);
	m_line.trimLeft();

	delim = m_line.find(' ');
	if (delim == -1)
		return err::fail("invalid HTTP request line: missing version");

	m_handshake->m_resource = m_line.getLeftSubString(delim);

	m_line.remove(0, delim + 1);
	m_line.trimLeft();

	if (!m_line.isPrefix("HTTP/"))
		return err::fail("invalid HTTP request line: unexpected version prefix");

	m_line.remove(0, lengthof("HTTP/"));
	m_handshake->m_httpVersion = m_line;

	uint_t version = strtoul(m_line, NULL, 10) << 8;

	delim = m_line.find('.');
	if (delim != -1)
	{
		m_line.remove(0, delim + 1);
		version |= strtoul(m_line, NULL, 10);
	}

	if (version < 0x0101) // HTTP/1.1
		return err::fail("invalid HTTP request line: unsupported HTTP version");

	m_state = State_HeaderLine;
	return true;
}

bool
WebSocketHandshakeParser::parseHeaderLine()
{
	if (m_line.isEmpty())
		return finalize();

	size_t delim = m_line.find(':');
	if (delim == -1)
		return err::fail("invalid HTTP header line");

	sl::StringRef key = m_line.getLeftSubString(delim).getRightTimmedString();
	sl::StringRef value = m_line.getSubString(delim + 1).getLeftTrimmedString();

	sl::StringHashTableIterator<WebSocketHeader> it = m_handshake->m_headerMap.visit(key);
	it->m_value.add(value);

	WebSocketHandshakeStdHeader stdHeader = WebSocketHandshakeStdHeaderMap::findValue(key, WebSocketHandshakeStdHeader_Undefined);
	if (stdHeader != WebSocketHandshakeStdHeader_Undefined)
		m_handshake->m_stdHeaderTable[stdHeader] = &it->m_value;

	return true;
}

bool
findInCsvStringIgnoreCase(
	const sl::StringRef& string0,
	const sl::StringRef& value,
	char c = ','
	)
{
	sl::StringRef string = string0;

	for (size_t i = 0;; i++)
	{
		size_t delim = string.find(c);
		if (delim == -1)
			return string.cmpIgnoreCase(value) == 0 ? i : -1;

		if (string.getLeftSubString(delim).getRightTimmedString().cmpIgnoreCase(value) == 0)
			return i;

		string = string.getSubString(i + 1).getLeftTrimmedString();
	}
}

bool
WebSocketHandshakeParser::finalize()
{
	if (!m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_Host] ||
		!m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_Connection] ||
		!m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_Upgrade] ||
		!m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketKey] ||
		!m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketVersion])
		return err::fail("invalid handshake: missing one or more required HTTP headers");

	if (!findInCsvStringIgnoreCase(m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_Connection]->m_firstValue, "upgrade"))
		return err::fail("invalid handshake: missing 'Upgrade' in HTTP 'Connection' header");

	if (m_handshake->m_stdHeaderTable[WebSocketHandshakeStdHeader_Upgrade]->m_firstValue.cmpIgnoreCase("websocket") != 0)
		return err::fail("invalid handshake: unexpected value of HTTP 'Upgrade' header");

	m_state = State_Completed;
	return true;
}

//..............................................................................

size_t
buildWebSocketHandshake(
	sl::String* handshake,
	const sl::StringRef& resource,
	const sl::StringRef& host,
	const sl::StringHashTable<WebSocketHeader>* extraHeaderMap
	)
{
	uchar_t keyValue[16];
	::RAND_bytes(keyValue, sizeof(keyValue));

	char buffer[256];
	sl::String keyString(ref::BufKind_Stack, buffer, sizeof(buffer));
	enc::Base64Encoding::encode(&keyString, keyValue, sizeof(keyValue));

	handshake->format(
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: Upgrade\r\n"
		"Upgrade: websocket\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Sec-WebSocket-Key: %s\r\n"
		"\r\n",
		resource.sz(),
		host.sz(),
		keyString.sz()
	);

	sl::ConstStringHashTableIterator<WebSocketHeader> it = extraHeaderMap ? extraHeaderMap->getHead() : NULL;
	for (; it; it++)
	{
		handshake->appendFormat("%s: %s\n", it->getKey().sz(), it->m_value.m_firstValue.sz());

		sl::ConstBoxIterator<sl::String> it2 = it->m_value.m_extraValueList.getHead();
		for (; it2; it2++)
			handshake->appendFormat("%s: %s\n", it->getKey().sz(), it2->sz());
	}

	return handshake->append("\r\n");
}

size_t
buildWebSocketHandshakeResponse(
	sl::String* response,
	const WebSocketHandshake& handshake
	)
{
	uchar_t hash[SHA_DIGEST_LENGTH];
	const sl::String& key = handshake.m_stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketKey]->m_firstValue;

	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha, key.cp(), key.getLength());
	SHA1_Update(&sha, WebSocketUuid, lengthof(WebSocketUuid));
	SHA1_Final(hash, &sha);

	char buffer[256];
	sl::String acceptKey(ref::BufKind_Stack, buffer, sizeof(buffer));
	enc::Base64Encoding::encode(&acceptKey, hash, sizeof(hash));

	return response->format(
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
