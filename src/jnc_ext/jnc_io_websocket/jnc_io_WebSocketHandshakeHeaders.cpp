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
#include "jnc_io_WebSocketHandshakeHeaders.h"
#include "jnc_io_WebSocketLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	WebSocketHandshakeHeaders,
	"io.WebSocketHandshakeHeaders",
	g_webSocketLibGuid,
	WebSocketLibCacheSlot_WebSocketHandshakeHeaders,
	WebSocketHandshakeHeaders,
	&WebSocketHandshakeHeaders::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(WebSocketHandshakeHeaders)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<WebSocketHandshakeHeaders>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<WebSocketHandshakeHeaders>)

	JNC_MAP_CONST_PROPERTY("m_name", &WebSocketHandshakeHeaders::getName)
	JNC_MAP_CONST_PROPERTY("m_valueCount", &WebSocketHandshakeHeaders::getValueCount)
	JNC_MAP_CONST_PROPERTY("m_value", &WebSocketHandshakeHeaders::getFirstValue)

	JNC_MAP_FUNCTION("findName", &WebSocketHandshakeHeaders::findName)
	JNC_MAP_FUNCTION("findValue", &WebSocketHandshakeHeaders::findValue)
	JNC_MAP_FUNCTION("getValue", &WebSocketHandshakeHeaders::getValue)
	JNC_MAP_FUNCTION("clear", &WebSocketHandshakeHeaders::clear)
	JNC_MAP_FUNCTION("add", &WebSocketHandshakeHeaders::add)
	JNC_MAP_FUNCTION("format", &WebSocketHandshakeHeaders::format)
JNC_END_TYPE_FUNCTION_MAP()

JNC_BEGIN_CLASS_TYPE_VTABLE(WebSocketHandshakeHeaders)
JNC_END_CLASS_TYPE_VTABLE()

//..............................................................................

void
WebSocketHandshakeHeader::add(
	const sl::StringRef& value_axl,
	String value_jnc
) {
	if (m_firstValue.isEmpty())
		m_firstValue.setup(value_axl, value_jnc);
	else {
		sl::BoxIterator<DualString> it = m_extraValueList.insertTail(DualString(value_axl, value_jnc));
		m_extraValueArray.append(it.p());
	}
}

void
WebSocketHandshakeHeader::markGcRoots(jnc::GcHeap* gcHeap) {
	m_name.markGcRoots(gcHeap);
	m_firstValue.markGcRoots(gcHeap);

	sl::BoxIterator<DualString> it = m_extraValueList.getHead();
	for (; it; it++)
		it->markGcRoots(gcHeap);
}

//..............................................................................

void
JNC_CDECL
WebSocketHandshakeHeaders::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	size_t count = m_headerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_headerArray[i]->markGcRoots(gcHeap);
}

void
WebSocketHandshakeHeaders::clear() {
	m_nameCount = 0;
	m_headerMap.clear();
	m_headerArray.clear();
	memset(m_stdHeaderTable, 0, sizeof(m_stdHeaderTable));
}

String
JNC_CDECL
WebSocketHandshakeHeaders::getName(
	WebSocketHandshakeHeaders* self,
	size_t nameIdx
) {
	return nameIdx < self->m_headerArray.getCount() ?
		self->m_headerArray[nameIdx]->m_name :
		g_nullString;
}

size_t
JNC_CDECL
WebSocketHandshakeHeaders::getValueCount(size_t nameIdx) {
	return nameIdx < m_headerArray.getCount() ?
		m_headerArray[nameIdx]->m_extraValueList.getCount() + 1 :
		0;
}

String
JNC_CDECL
WebSocketHandshakeHeaders::getFirstValue(
	WebSocketHandshakeHeaders* self,
	size_t nameIdx
) {
	return nameIdx < self->m_headerArray.getCount() ?
		self->m_headerArray[nameIdx]->m_firstValue :
		g_nullString;
}

String
JNC_CDECL
WebSocketHandshakeHeaders::getValue(
	WebSocketHandshakeHeaders* self,
	size_t nameIdx,
	size_t valueIdx
) {
	if (nameIdx >= self->m_headerArray.getCount())
		return g_nullString;

	WebSocketHandshakeHeader* header = self->m_headerArray[nameIdx];
	if (valueIdx > header->m_extraValueArray.getCount())
		return g_nullString;

	DualString* value = valueIdx == 0 ?
		&header->m_firstValue :
		header->m_extraValueArray[valueIdx];

	return *value;
}

size_t
JNC_CDECL
WebSocketHandshakeHeaders::findName(String name) {
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = m_headerMap.find(name >> toAxl);
	return it ? it->m_value.m_nameIdx : -1;
}

String
JNC_CDECL
WebSocketHandshakeHeaders::findValue(
	WebSocketHandshakeHeaders* self,
	String name
) {
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = self->m_headerMap.find(name >> toAxl);
	return it ? it->m_value.m_firstValue : g_nullString;
}

String
JNC_CDECL
WebSocketHandshakeHeaders::format(
	WebSocketHandshakeHeaders* self,
	String delimiter,
	String eol
) {
	sl::String string;
	self->appendFormat(&string, delimiter >> toAxl, eol >> toAxl);
	return allocateString(string);
}

WebSocketHandshakeHeader*
WebSocketHandshakeHeaders::addImpl(
	const sl::StringRef& name_axl,
	String name_jnc,
	const sl::StringRef& value_axl,
	String value_jnc
) {
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = m_headerMap.visit(name_axl);
	WebSocketHandshakeHeader* header = &it->m_value;
	ASSERT(header->m_nameIdx != -1 || header->m_name.isEmpty() && header->m_firstValue.isEmpty());

	if (header->m_nameIdx != -1) {
		header->add(value_axl, value_jnc);
		return header;
	}

	header->m_nameIdx = m_headerArray.getCount();
	header->m_name.setup(name_axl, name_jnc);
	m_nameCount = m_headerArray.append(header);

	WebSocketHandshakeStdHeader stdHeader = WebSocketHandshakeStdHeaderMap::findValue(name_axl, WebSocketHandshakeStdHeader_Undefined);
	if (stdHeader != WebSocketHandshakeStdHeader_Undefined)
		m_stdHeaderTable[stdHeader] = header;

	header->add(value_axl, value_jnc);
	return header;
}

void
WebSocketHandshakeHeaders::addImpl(WebSocketHandshakeHeaders* headers) {
	size_t count = headers->m_headerArray.getCount();
	for (size_t i = 0; i < count; i++) {
		WebSocketHandshakeHeader* srcHeader = headers->m_headerArray[i];

		WebSocketHandshakeHeader* dstHeader = addImpl(
			srcHeader->m_name.m_string_axl,
			srcHeader->m_name.m_string_jnc,
			srcHeader->m_firstValue.m_string_axl,
			srcHeader->m_firstValue.m_string_jnc
		);

		sl::ConstBoxIterator<DualString> it = srcHeader->m_extraValueList.getHead();
		for (; it; it++)
			dstHeader->add(it->m_string_axl, it->m_string_jnc);
	}
}

size_t
WebSocketHandshakeHeaders::appendFormat(
	sl::String* string,
	const sl::StringRef& delimiter,
	const sl::StringRef& eol
) {
	size_t count = m_headerArray.getCount();
	for (size_t i = 0; i < count; i++) {
		WebSocketHandshakeHeader* header = m_headerArray[i];

		string->appendFormat(
			"%s: %s\r\n",
			header->m_name.sz(),
			header->m_firstValue.sz()
		);

		sl::ConstBoxIterator<DualString> it = header->m_extraValueList.getHead();
		for (; it; it++) {
			string->append(header->m_name.m_string_axl);
			string->append(delimiter);
			string->append(*it);
			string->append(eol);
		}
	}

	return string->getLength();
}

//..............................................................................

} // namespace io
} // namespace jnc
