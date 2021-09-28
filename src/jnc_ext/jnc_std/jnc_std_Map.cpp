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
#include "jnc_std_Map.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_TYPE(
	MapEntry,
	"std.MapEntry",
	g_stdLibGuid,
	StdLibCacheSlot_MapEntry
)

JNC_BEGIN_TYPE_FUNCTION_MAP(MapEntry)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
Map::clear() {
	m_headPtr = g_nullDataPtr;
	m_tailPtr = g_nullDataPtr;
	m_count = 0;
}

DataPtr
Map::add(const sl::MapIterator<Variant, DataPtr>& it) {
	Runtime* runtime = getCurrentThreadRuntime();
	Type* entryType = MapEntry::getType(runtime->getModule());
	DataPtr entryPtr = runtime->getGcHeap()->allocateData(entryType);
	MapEntry* entry = (MapEntry*)entryPtr.m_p;

	sl::MapEntry<Variant, DataPtr>* next = (sl::MapEntry<Variant, DataPtr>*) it->getNext();
	sl::MapEntry<Variant, DataPtr>* prev = (sl::MapEntry<Variant, DataPtr>*) it->getPrev();

	entry->m_key = it->getKey();
	entry->m_nextPtr = next ? next->m_value : g_nullDataPtr;
	entry->m_prevPtr = prev ? prev->m_value : g_nullDataPtr;
	entry->m_map = this;
	entry->m_mapEntry = it.getEntry();

	if (entry->m_prevPtr.m_p)
		((MapEntry*)entry->m_prevPtr.m_p)->m_nextPtr = entryPtr;
	else
		m_headPtr = entryPtr;

	if (entry->m_nextPtr.m_p)
		((MapEntry*)entry->m_nextPtr.m_p)->m_prevPtr = entryPtr;
	else
		m_tailPtr = entryPtr;

	m_count++;

	return entryPtr;
}

void
Map::remove(MapEntry* entry) {
	ASSERT(entry->m_map == this);
	ASSERT(entry->m_prevPtr.m_p || m_headPtr.m_p == entry);
	ASSERT(entry->m_nextPtr.m_p || m_tailPtr.m_p == entry);

	if (entry->m_prevPtr.m_p)
		((MapEntry*)entry->m_prevPtr.m_p)->m_nextPtr = entry->m_nextPtr;
	else
		m_headPtr = entry->m_nextPtr;

	if (entry->m_nextPtr.m_p)
		((MapEntry*)entry->m_nextPtr.m_p)->m_prevPtr = entry->m_prevPtr;
	else
		m_tailPtr = entry->m_prevPtr;

	m_count--;
}

//..............................................................................

} // namespace std
} // namespace jnc
