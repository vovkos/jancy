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
#include "jnc_std_HashTable.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE_REQ(
	HashTable,
	"std.HashTable",
	g_stdLibGuid,
	StdLibCacheSlot_HashTable,
	HashTable,
	NULL
)

JNC_BEGIN_OPAQUE_CLASS_REQUIRE_TABLE(HashTable)
	JNC_OPAQUE_CLASS_REQUIRE_TYPE(TypeKind_Struct, "std.MapEntry")
JNC_END_OPAQUE_CLASS_REQUIRE_TABLE()

JNC_BEGIN_TYPE_FUNCTION_MAP(HashTable)
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<HashTable, HashFunc*, IsEqualFunc*>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HashTable>)
	JNC_MAP_FUNCTION("clear",  &HashTable::clear)
	JNC_MAP_FUNCTION("find", &HashTable::find)
	JNC_MAP_FUNCTION("visit", &HashTable::visit)
	JNC_MAP_FUNCTION("remove", &HashTable::remove)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
HashTable::visitImpl(Variant key) {
	sl::MapIterator<Variant, DataPtr> it = m_hashTable.visit(key);
	if (!it->m_value.m_p) {
		it->m_value = m_map.add(it);
		ASSERT(m_map.m_count == m_hashTable.getCount());
	}

	return it->m_value;
}

void
HashTable::removeImpl(MapEntry* entry) {
	if (!entry || entry->m_map != &m_map) {
		err::setError("attempt to remove an invalid map entry from the hash table");
		dynamicThrow();
	}

	m_hashTable.erase((sl::HashTableEntry<Variant, DataPtr>*) entry->m_mapEntry);
	m_map.remove(entry);
	ASSERT(m_map.m_count == m_hashTable.getCount());
}

//..............................................................................

} // namespace std
} // namespace jnc
