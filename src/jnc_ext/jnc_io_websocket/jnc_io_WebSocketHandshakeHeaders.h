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

JNC_DECLARE_OPAQUE_CLASS_TYPE(WebSocketHandshakeHeaders)

//..............................................................................

enum WebSocketHandshakeStdHeader {
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

struct WebSocketHandshakeHeader {
	size_t m_nameIdx;
	DualString m_name;
	DualString m_firstValue;
	sl::BoxList<DualString> m_extraValueList;
	sl::Array<DualString*> m_extraValueArray;

	WebSocketHandshakeHeader() {
		m_nameIdx = -1;
	}

	void
	add(
		const sl::StringRef& value,
		DataPtr valuePtr
	);

	void
	markGcRoots(jnc::GcHeap* gcHeap);
};

//..............................................................................

class WebSocketHandshakeHeaders: public IfaceHdr {
	friend class WebSocketHandshake;
	friend class WebSocketHandshakeParser;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(WebSocketHandshakeHeaders)

public:
	size_t m_nameCount;

protected:
	sl::StringHashTable<WebSocketHandshakeHeader> m_headerMap;
	sl::Array<WebSocketHandshakeHeader*> m_headerArray;
	WebSocketHandshakeHeader* m_stdHeaderTable[WebSocketHandshakeStdHeader__Count];

public:
	WebSocketHandshakeHeaders() {
		m_nameCount = 0;
	}

	const sl::StringHashTable<WebSocketHandshakeHeader>&
	getHeaderMap() const {
		return m_headerMap;
	}

	const WebSocketHandshakeHeader* const*
	getStdHeaderTable() const {
		return m_stdHeaderTable;
	}

	WebSocketHandshakeHeader*
	getStdHeader(WebSocketHandshakeStdHeader stdHeader) const {
		ASSERT((size_t)stdHeader < WebSocketHandshakeStdHeader__Count);
		return m_stdHeaderTable[stdHeader];
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	void
	JNC_CDECL
	clear();

	static
	DataPtr
	JNC_CDECL
	getName(
		WebSocketHandshakeHeaders* self,
		size_t nameIdx
	);

	size_t
	JNC_CDECL
	getValueCount(size_t nameIdx);

	static
	DataPtr
	JNC_CDECL
	getFirstValue(
		WebSocketHandshakeHeaders* self,
		size_t nameIdx
	);

	static
	DataPtr
	JNC_CDECL
	getValue(
		WebSocketHandshakeHeaders* self,
		size_t nameIdx,
		size_t valueIdx
	);

	size_t
	JNC_CDECL
	findName(DataPtr namePtr);

	static
	DataPtr
	JNC_CDECL
	findValue(
		WebSocketHandshakeHeaders* self,
		DataPtr namePtr
	);

	size_t
	JNC_CDECL
	add(
		DataPtr namePtr,
		DataPtr valuePtr
	) {
		return addImpl(
			sl::StringRef((char*)namePtr.m_p, strLen(namePtr)),
			namePtr,
			sl::StringRef((char*)valuePtr.m_p, strLen(valuePtr)),
			valuePtr
		)->m_nameIdx;
	}

	static
	DataPtr
	JNC_CDECL
	format(
		WebSocketHandshakeHeaders* self,
		DataPtr delimiterPtr,
		DataPtr eolPtr
	);

protected:
	WebSocketHandshakeHeader*
	addImpl(
		const sl::StringRef& name,
		DataPtr namePtr,
		const sl::StringRef& value,
		DataPtr valuePtr
	);

	WebSocketHandshakeHeader*
	addImpl(
		const sl::StringRef& name,
		const sl::StringRef& value
	) {
		return addImpl(name, g_nullDataPtr, value, g_nullDataPtr);
	}

	void
	addImpl(WebSocketHandshakeHeaders* headers);

	size_t
	appendFormat(
		sl::String* string,
		const sl::StringRef& delimiter = ": ",
		const sl::StringRef& eol = "\r\n"
	);
};

//..............................................................................

} // namespace io
} // namespace jnc
