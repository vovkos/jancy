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
#include "jnc_io_WebSocketHandshakeParser.h"

namespace jnc {
namespace io {

//..............................................................................

size_t
findInCsvStringIgnoreCase(
	const sl::StringRef& string0,
	const sl::StringRef& value,
	char c = ','
) {
	sl::StringRef string = string0;

	for (size_t i = 0;; i++) {
		size_t delim = string.find(c);
		if (delim == -1)
			return string.cmpIgnoreCase(value) == 0 ? i : -1;

		if (string.getLeftSubString(delim).getRightTimmedString().cmpIgnoreCase(value) == 0)
			return i;

		string = string.getSubString(i + 1).getLeftTrimmedString();
	}
}

//..............................................................................

WebSocketHandshakeParser::WebSocketHandshakeParser(
	WebSocketHandshake* handshake,
	const sl::StringRef& key
) {
	handshake->clear();
	m_handshake = handshake;
	m_key = key;
	m_state = key.isEmpty() ? State_RequestLine : State_ResponseLine;
}

size_t
WebSocketHandshakeParser::bufferLine(
	const char* p,
	size_t size
) {
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
) {
	const char* p = (char*)p0;
	const char* end = p + size;

	while (p < end && m_state != State_Completed) {
		size_t result = bufferLine(p, end - p);
		if (result == -1)
			return -1;

		p += result;

		if (!m_line.isSuffix('\n')) // incomplete line
			break;

		m_line.trim();

		switch (m_state) {
		case State_RequestLine:
			result = parseRequestLine();
			break;

		case State_ResponseLine:
			result = parseResponseLine();
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

	size_t length = p - (char*)p0;
	m_handshake->m_rawData = m_handshake->m_rawData.m_string + sl::StringRef((char*)p0, length);
	return length;
}

bool
WebSocketHandshakeParser::parseRequestLine() {
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
	m_handshake->m_httpVersion = strtoul(m_line, NULL, 10) << 8;

	delim = m_line.find('.');
	if (delim != -1) {
		m_line.remove(0, delim + 1);
		m_handshake->m_httpVersion |= strtoul(m_line, NULL, 10);
	}

	m_state = State_HeaderLine;
	return true;
}

bool
WebSocketHandshakeParser::parseResponseLine() {
	if (!m_line.isPrefix("HTTP/"))
		return err::fail("invalid HTTP response line: unexpected version prefix");

	m_line.remove(0, lengthof("HTTP/"));
	m_handshake->m_httpVersion = strtoul(m_line, NULL, 10) << 8;

	size_t delim = m_line.find('.');
	if (delim != -1) {
		m_line.remove(0, delim + 1);
		m_handshake->m_httpVersion |= strtoul(m_line, NULL, 10);
	}

	delim = m_line.find(' ');
	if (delim == -1)
		return err::fail("invalid HTTP response line: missing status code");

	m_line.remove(0, delim + 1);
	m_line.trimLeft();

	m_handshake->m_statusCode = strtoul(m_line, NULL, 10);

	delim = m_line.find(' ');
	if (delim == -1)
		return err::fail("invalid HTTP response line: missing reason phrase");

	m_line.remove(0, delim + 1);
	m_line.trimLeft();
	m_handshake->m_reasonPhrase = m_line;
	m_state = State_HeaderLine;
	return true;
}

bool
WebSocketHandshakeParser::parseHeaderLine() {
	if (m_line.isEmpty())
		return finalize();

	size_t delim = m_line.find(':');
	if (delim == -1)
		return err::fail("invalid HTTP header line");

	sl::StringRef key = m_line.getLeftSubString(delim).getRightTimmedString();
	sl::StringRef value = m_line.getSubString(delim + 1).getLeftTrimmedString();
	m_handshake->m_headers->addImpl(key, value);
	return true;
}

bool
WebSocketHandshakeParser::finalize() {
	if (m_handshake->m_httpVersion < 0x0101) // HTTP/1.1
		return err::fail("unsupported HTTP version");

	const WebSocketHandshakeHeader* const* stdHeaderTable = m_handshake->m_headers->getStdHeaderTable();

	bool hasMissingFields = !m_key.isEmpty() ?
		!stdHeaderTable[WebSocketHandshakeStdHeader_Connection] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_Upgrade] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketAccept]
		:
		!stdHeaderTable[WebSocketHandshakeStdHeader_Host] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_Connection] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_Upgrade] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketVersion] ||
		!stdHeaderTable[WebSocketHandshakeStdHeader_WebSocketKey];

	if (hasMissingFields)
		return err::fail("invalid handshake: missing one or more required HTTP headers");

	size_t i = findInCsvStringIgnoreCase(stdHeaderTable[WebSocketHandshakeStdHeader_Connection]->m_firstValue, "upgrade");
	if (i == -1)
		return err::fail("invalid handshake: missing 'Upgrade' in HTTP 'Connection' header");

	if (stdHeaderTable[WebSocketHandshakeStdHeader_Upgrade]->m_firstValue.m_string.cmpIgnoreCase("websocket") != 0)
		return err::fail("invalid handshake: unexpected value of HTTP 'Upgrade' header");

	if (!m_key.isEmpty() && !verifyAccept())
		return false;

	m_state = State_Completed;
	return true;
}

bool
WebSocketHandshakeParser::verifyAccept() {
	const WebSocketHandshakeHeader* accept = m_handshake->m_headers->getStdHeader(WebSocketHandshakeStdHeader_WebSocketAccept);
	ASSERT(!m_key.isEmpty() && accept);

	if (m_handshake->m_statusCode != 101) {
		err::setFormatStringError(
			"failure to switch HTTP protocol: %d %s",
			m_handshake->m_statusCode,
			m_handshake->m_reasonPhrase.sz()
		);

		return false;
	}

	char buffer[256];
	sl::Array<char> acceptHash(rc::BufKind_Stack, buffer, sizeof(buffer));
	enc::Base64Encoding::decode(&acceptHash, accept->m_firstValue);
	if (acceptHash.getCount() != SHA_DIGEST_LENGTH)
		return err::fail("invalid handshake: bad 'Sec-WebSocket-Accept' format");

	uchar_t keyHash[SHA_DIGEST_LENGTH];
	calcWebSocketHandshakeKeyHash(keyHash, m_key);
	if (memcmp(acceptHash, keyHash, SHA_DIGEST_LENGTH) != 0)
		return err::fail("invalid handshake: 'Sec-WebSocket-Accept' doesn't match 'Sec-WebSocket-Key'");

	return true;
}

//..............................................................................

} // namespace io
} // namespace jnc
