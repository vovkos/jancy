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

JNC_DEFINE_TYPE (
	MapEntry,
	"std.MapEntry",
	g_stdLibGuid,
	StdLibCacheSlot_MapEntry
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (MapEntry)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
Map::clear ()
{
	m_headPtr = g_nullPtr;
	m_tailPtr = g_nullPtr;
	m_count = 0;
}

DataPtr
Map::add (const sl::MapIterator <Variant, DataPtr>& it)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	Type* entryType = MapEntry::getType (runtime->getModule ());
	DataPtr entryPtr = runtime->getGcHeap ()->allocateData (entryType);
	MapEntry* entry = (MapEntry*) entryPtr.m_p;

	entry->m_key = it->m_key;
	entry->m_prevPtr = it->m_prev ? ((sl::MapEntry <Variant, DataPtr>*) it->m_prev)->m_value : g_nullPtr;
	entry->m_nextPtr = it->m_next ? ((sl::MapEntry <Variant, DataPtr>*) it->m_next)->m_value : g_nullPtr;
	entry->m_map = this;
	entry->m_mapEntry = it.getEntry ();

	if (entry->m_prevPtr.m_p)
		((MapEntry*) entry->m_prevPtr.m_p)->m_nextPtr = entryPtr;
	else
		m_headPtr = entryPtr;

	if (entry->m_nextPtr.m_p)
		((MapEntry*) entry->m_nextPtr.m_p)->m_prevPtr = entryPtr;
	else
		m_tailPtr = entryPtr;

	m_count++;

	return entryPtr;
}

void
Map::remove (MapEntry* entry)
{
	ASSERT (entry->m_map == this);
	ASSERT (entry->m_prevPtr.m_p || m_headPtr.m_p == entry);
	ASSERT (entry->m_nextPtr.m_p || m_tailPtr.m_p == entry);

	if (entry->m_prevPtr.m_p)
		((MapEntry*) entry->m_prevPtr.m_p)->m_nextPtr = entry->m_nextPtr;
	else
		m_headPtr = entry->m_nextPtr;

	if (entry->m_nextPtr.m_p)
		((MapEntry*) entry->m_nextPtr.m_p)->m_prevPtr = entry->m_prevPtr;
	else
		m_tailPtr = entry->m_prevPtr;

	m_count--;
}

//..............................................................................

} // namespace std
} // namespace jnc
