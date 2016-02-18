#include "pch.h"
#include "jnc_std_HashTable.h"

namespace jnc {
namespace std {

//.............................................................................

StringHashTable::~StringHashTable ()
{
	ASSERT (m_hashTable);

	AXL_MEM_DELETE (m_hashTable);
	m_hashTable = NULL;
	m_count = 0;
}

void
AXL_CDECL
StringHashTable::insert (
	rt::DataPtr keyPtr,
	rt::Variant value
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ct::Type* type = runtime->getModule ()->m_typeMgr.getPrimitiveType (ct::TypeKind_Variant);
	rt::DataPtr valuePtr = runtime->m_gcHeap.allocateData (type);
	*(rt::Variant*) valuePtr.m_p = value;
	(*m_hashTable) [(const char*) keyPtr.m_p] = valuePtr;
	m_count = m_hashTable->getCount ();
}

//.............................................................................

VariantHashTable::~VariantHashTable ()
{
	ASSERT (m_hashTable);

	AXL_MEM_DELETE (m_hashTable);
	m_hashTable = NULL;
	m_count = 0;
}

void
AXL_CDECL
VariantHashTable::insert (
	rt::Variant key,
	rt::Variant value
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ct::Type* type = runtime->getModule ()->m_typeMgr.getPrimitiveType (ct::TypeKind_Variant);
	rt::DataPtr valuePtr = runtime->m_gcHeap.tryAllocateData (type);
	*(rt::Variant*) valuePtr.m_p = value;
	(*m_hashTable) [key] = valuePtr;
	m_count = m_hashTable->getCount ();
}

//.............................................................................

} // namespace std
} // namespace jnc
