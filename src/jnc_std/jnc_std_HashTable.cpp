#include "pch.h"
#include "jnc_std_HashTable.h"

namespace jnc {
namespace std {

//.............................................................................

void
AXL_CDECL
StringHashTable::markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap)
{
	ct::Type* ptrType = gcHeap->getRuntime ()->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType ();
	ct::Type* variantType = gcHeap->getRuntime ()->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);

	sl::Iterator <Entry> it = m_list.getHead ();
	for (; it; it++)
	{
		ptrType->markGcRoots (&it->m_keyPtr, gcHeap);
		variantType->markGcRoots (&it->m_value, gcHeap);
	}
}

bool
AXL_CDECL
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
AXL_CDECL
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
AXL_CDECL
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
AXL_CDECL
VariantHashTable::markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap)
{
	ct::Type* variantType = gcHeap->getRuntime ()->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);

	sl::Iterator <Entry> it = m_list.getHead ();
	for (; it; it++)
	{
		variantType->markGcRoots (&it->m_key, gcHeap);
		variantType->markGcRoots (&it->m_value, gcHeap);
	}
}

bool
AXL_CDECL
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
AXL_CDECL
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
AXL_CDECL
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
