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

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	HashTable,
	"std.HashTable",
	g_stdLibGuid,
	StdLibCacheSlot_HashTable,
	HashTable,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (HashTable)
	JNC_MAP_CONSTRUCTOR (&(jnc::construct <HashTable, HashFunc*, IsEqualFunc*>))
	JNC_MAP_DESTRUCTOR (&jnc::destruct <HashTable>)
	JNC_MAP_FUNCTION ("clear",  &HashTable::clear)
	JNC_MAP_FUNCTION ("find", &HashTable::find)
	JNC_MAP_FUNCTION ("visit", &HashTable::visit)
	JNC_MAP_FUNCTION ("remove", &HashTable::remove)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	StringHashTable,
	"std.StringHashTable",
	g_stdLibGuid,
	StdLibCacheSlot_StringHashTable,
	StringHashTable,
	&StringHashTable::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (StringHashTable)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <StringHashTable>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <StringHashTable>)
	JNC_MAP_CONST_PROPERTY ("m_isEmpty",  &StringHashTable::isEmpty)
	JNC_MAP_FUNCTION ("clear",  &StringHashTable::clear)
	JNC_MAP_FUNCTION ("find", &StringHashTable::find)
	JNC_MAP_FUNCTION ("insert", &StringHashTable::insert)
	JNC_MAP_FUNCTION ("remove", &StringHashTable::remove)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	VariantHashTable,
	"std.VariantHashTable",
	g_stdLibGuid,
	StdLibCacheSlot_VariantHashTable,
	VariantHashTable,
	&VariantHashTable::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (VariantHashTable)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <VariantHashTable>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <VariantHashTable>)
	JNC_MAP_CONST_PROPERTY ("m_isEmpty",  &VariantHashTable::isEmpty)
	JNC_MAP_FUNCTION ("clear",  &VariantHashTable::clear)
	JNC_MAP_FUNCTION ("find", &VariantHashTable::find)
	JNC_MAP_FUNCTION ("insert", &VariantHashTable::insert)
	JNC_MAP_FUNCTION ("remove", &VariantHashTable::remove)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

HashTable::HashTable (
	HashFunc* hashFunc,
	IsEqualFunc* isEqualFunc
	)
{
}

HashTable::~HashTable ()
{
}

void
HashTable::clear ()
{
}

DataPtr
HashTable::visit (Variant key) const
{
	return g_nullPtr;
}

DataPtr
HashTable::find (Variant key) const
{
	return g_nullPtr;
}

void
HashTable::remove (DataPtr entryPtr)
{
}

//..............................................................................

void
JNC_CDECL
StringHashTable::markOpaqueGcRoots (GcHeap* gcHeap)
{
	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* ptrType = module->getPrimitiveType (TypeKind_Void)->getDataPtrType ();
	Type* variantType = module->getPrimitiveType (TypeKind_Variant);

	sl::Iterator <Entry> it = m_list.getHead ();
	for (; it; it++)
	{
		ptrType->markGcRoots (&it->m_keyPtr, gcHeap);
		variantType->markGcRoots (&it->m_value, gcHeap);
	}
}

bool
JNC_CDECL
StringHashTable::find (
	DataPtr keyPtr,
	DataPtr valuePtr
	)
{
	Map::Iterator it = m_map.find ((const char*) keyPtr.m_p);
	if (!it)
		return false;

	*(Variant*) valuePtr.m_p = it->m_value->m_value;
	return true;
}

void
JNC_CDECL
StringHashTable::insert (
	DataPtr keyPtr,
	Variant value
	)
{
	Map::Iterator it = m_map.visit ((const char*) keyPtr.m_p);
	if (it->m_value)
	{
		it->m_value->m_value = value;
		return;
	}

	Entry* entry = AXL_MEM_NEW (Entry);
	entry->m_keyPtr = keyPtr;
	entry->m_value = value;
	m_list.insertTail (entry);
	m_map [(const char*) keyPtr.m_p] = entry;
}

bool
JNC_CDECL
StringHashTable::remove (DataPtr keyPtr)
{
	Map::Iterator it = m_map.find ((const char*) keyPtr.m_p);
	if (!it)
		return false;

	m_list.erase (it->m_value);
	m_map.erase (it);
	return true;
}

//..............................................................................

void
JNC_CDECL
VariantHashTable::markOpaqueGcRoots (GcHeap* gcHeap)
{
	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* variantType = module->getPrimitiveType (TypeKind_Variant);

	sl::Iterator <Entry> it = m_list.getHead ();
	for (; it; it++)
	{
		variantType->markGcRoots (&it->m_key, gcHeap);
		variantType->markGcRoots (&it->m_value, gcHeap);
	}
}

bool
JNC_CDECL
VariantHashTable::find (
	Variant key,
	DataPtr valuePtr
	)
{
	Map::Iterator it = m_map.find (key);
	if (!it)
		return false;

	*(Variant*) valuePtr.m_p = it->m_value->m_value;
	return true;
}

void
JNC_CDECL
VariantHashTable::insert (
	Variant key,
	Variant value
	)
{
	Map::Iterator it = m_map.visit (key);
	if (it->m_value)
	{
		it->m_value->m_value = value;
		return;
	}

	Entry* entry = AXL_MEM_NEW (Entry);
	entry->m_key = key;
	entry->m_value = value;
	m_list.insertTail (entry);
	m_map [key] = entry;
}

bool
JNC_CDECL
VariantHashTable::remove (Variant key)
{
	Map::Iterator it = m_map.find (key);
	if (!it)
		return false;

	m_list.erase (it->m_value);
	m_map.erase (it);
	return true;
}

//..............................................................................

} // namespace std
} // namespace jnc
