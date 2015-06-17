#include "pch.h"
#include "jnc_HashTable.h"

namespace jnc {

//.............................................................................

DataPtr
StringHashTable::find_s (
	StringHashTable* self,
	DataPtr keyPtr
	)
{
	rtl::StringHashTableMap <Variant>::Iterator it = self->m_hashTable->find ((const char*) keyPtr.m_p);
	if (!it)
		return g_nullPtr;

	DataPtr ptr;
	ptr.m_p = &it->m_value;
	ptr.m_rangeBegin = &it->m_value;
	ptr.m_rangeEnd = &it->m_value + 1;
	ptr.m_object = getStaticObjHdr ();
	return ptr;
}

//.............................................................................

DataPtr
VariantHashTable::find_s (
	VariantHashTable* self,
	Variant key
	)
{
	VariantHashTableMap::Iterator it = self->m_hashTable->find (key);
	if (!it)
		return g_nullPtr;

	DataPtr ptr;
	ptr.m_p = &it->m_value;
	ptr.m_rangeBegin = &it->m_value;
	ptr.m_rangeEnd = &it->m_value + 1;
	ptr.m_object = getStaticObjHdr ();
	return ptr;
}

//.............................................................................

} // namespace jnc
