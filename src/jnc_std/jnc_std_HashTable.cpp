#include "pch.h"
#include "jnc_std_HashTable.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//.............................................................................

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//.............................................................................

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
	StringHashTableMap::Iterator it = m_map.find ((const char*) keyPtr.m_p);
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
	StringHashTableMap::Iterator it = m_map.visit ((const char*) keyPtr.m_p);
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
	StringHashTableMap::Iterator it = m_map.find ((const char*) keyPtr.m_p);
	if (!it)
		return false;

	m_list.erase (it->m_value);
	m_map.erase (it);
	return true;
}

//.............................................................................

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
	VariantHashTableMap::Iterator it = m_map.find (key);
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
	VariantHashTableMap::Iterator it = m_map.visit (key);
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
	VariantHashTableMap::Iterator it = m_map.find (key);
	if (!it)
		return false;

	m_list.erase (it->m_value);
	m_map.erase (it);
	return true;
}

//.............................................................................

} // namespace std
} // namespace jnc
