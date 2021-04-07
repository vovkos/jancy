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
	JNC_MAP_CONST_PROPERTY("m_value", &WebSocketHandshakeHeaders::getValue)

	JNC_MAP_FUNCTION("findName", &WebSocketHandshakeHeaders::findName)
	JNC_MAP_FUNCTION("findValue", &WebSocketHandshakeHeaders::findValue)
	JNC_MAP_FUNCTION("clear", &WebSocketHandshakeHeaders::clear)
	JNC_MAP_FUNCTION("add", &WebSocketHandshakeHeaders::add)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
WebSocketHandshakeHeader::add(
	const sl::StringRef& value,
	DataPtr valuePtr
	)
{
	if (m_firstValue.isEmpty())
	{
		m_firstValue.m_string = value;
		m_firstValue.m_ptr = valuePtr;
	}
	else
	{
		DualString extraValue;
		extraValue.m_string = value;
		extraValue.m_ptr = valuePtr;
		sl::BoxIterator<DualString> it = m_extraValueList.insertTail(extraValue);
		m_extraValueArray.append(it.p());
	}
}

void
WebSocketHandshakeHeader::markGcRoots(jnc::GcHeap* gcHeap)
{
	m_name.markGcRoots(gcHeap);
	m_firstValue.markGcRoots(gcHeap);

	sl::BoxIterator<DualString> it = m_extraValueList.getHead();
	for (; it; it++)
		it->markGcRoots(gcHeap);
}

//..............................................................................

void
JNC_CDECL
WebSocketHandshakeHeaders::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	size_t count = m_headerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_headerArray[i]->markGcRoots(gcHeap);
}

void
WebSocketHandshakeHeaders::clear()
{
	m_nameCount = 0;
	m_headerMap.clear();
	m_headerArray.clear();
	memset(m_stdHeaderTable, 0, sizeof(m_stdHeaderTable));
}

DataPtr
JNC_CDECL
WebSocketHandshakeHeaders::getName(
	WebSocketHandshakeHeaders* self,
	size_t nameIdx
	)
{
	return nameIdx < self->m_headerArray.getCount() ?
		self->m_headerArray[nameIdx]->m_name.getPtr() :
		g_nullDataPtr;
}

size_t
JNC_CDECL
WebSocketHandshakeHeaders::getValueCount(size_t nameIdx)
{
	return nameIdx < m_headerArray.getCount() ?
		m_headerArray[nameIdx]->m_extraValueList.getCount() + 1 :
		0;
}

DataPtr
JNC_CDECL
WebSocketHandshakeHeaders::getValue(
	WebSocketHandshakeHeaders* self,
	size_t nameIdx,
	size_t valueIdx
	)
{
	if (nameIdx >= self->m_headerArray.getCount())
		return g_nullDataPtr;

	WebSocketHandshakeHeader* header = self->m_headerArray[nameIdx];
	if (valueIdx > header->m_extraValueArray.getCount())
		return g_nullDataPtr;

	DualString* value = valueIdx == 0 ?
		&header->m_firstValue :
		header->m_extraValueArray[valueIdx];

	return value->getPtr();
}

size_t
JNC_CDECL
WebSocketHandshakeHeaders::findName(DataPtr namePtr)
{
	sl::StringRef name((char*)namePtr.m_p, strLen(namePtr));
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = m_headerMap.find(name);
	return it ? it->m_value.m_nameIdx : -1;
}

DataPtr
JNC_CDECL
WebSocketHandshakeHeaders::findValue(
	WebSocketHandshakeHeaders* self,
	DataPtr namePtr
	)
{
	sl::StringRef name((char*)namePtr.m_p, strLen(namePtr));
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = self->m_headerMap.find(name);
	return it ? it->m_value.m_firstValue.getPtr() : g_nullDataPtr;
}

size_t
WebSocketHandshakeHeaders::addImpl(
	const sl::StringRef& name,
	DataPtr namePtr,
	const sl::StringRef& value,
	DataPtr valuePtr
	)
{
	sl::StringHashTableIterator<WebSocketHandshakeHeader> it = m_headerMap.visit(name);

	WebSocketHandshakeHeader* header = &it->m_value;
	ASSERT(header->m_nameIdx != -1 || !header->m_name.isEmpty() && !header->m_firstValue.isEmpty());

	if (header->m_nameIdx == -1)
	{
		header->m_nameIdx = m_headerArray.getCount();
		header->m_name.m_string = name;
		header->m_name.m_ptr = namePtr;
		m_headerArray.append(header);
	}

	WebSocketHandshakeStdHeader stdHeader = WebSocketHandshakeStdHeaderMap::findValue(name, WebSocketHandshakeStdHeader_Undefined);
	if (stdHeader != WebSocketHandshakeStdHeader_Undefined)
		m_stdHeaderTable[stdHeader] = header;

	header->add(value, valuePtr);
	return header->m_extraValueArray.getCount() + 1;
}

//..............................................................................

} // namespace io
} // namespace jnc
