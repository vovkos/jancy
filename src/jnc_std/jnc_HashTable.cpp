#include "pch.h"
#include "jnc_HashTable.h"

namespace jnc {

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
	DataPtr keyPtr,
	Variant value
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = runtime->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);
	DataPtr valuePtr = runtime->m_gcHeap.allocateData (type);
	*(Variant*) valuePtr.m_p = value;
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
	Variant key,
	Variant value
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = runtime->getModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);
	DataPtr valuePtr = runtime->m_gcHeap.tryAllocateData (type);
	*(Variant*) valuePtr.m_p = value;
	(*m_hashTable) [key] = valuePtr;
	m_count = m_hashTable->getCount ();
}

//.............................................................................

} // namespace jnc
