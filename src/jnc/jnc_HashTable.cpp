#include "pch.h"
#include "jnc_HashTable.h"

namespace jnc {

//.............................................................................

void
AXL_CDECL
StringHashTable::insert (
	DataPtr keyPtr,
	Variant value
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = runtime->getFirstModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);
	DataBox* box = (DataBox*) runtime->m_gcHeap.allocateData (type);

	DataPtr valuePtr;
	valuePtr.m_p = box + 1;
	valuePtr.m_validator = &box->m_validator;

	(*m_hashTable) [(const char*) keyPtr.m_p] = valuePtr;
	m_count = m_hashTable->getCount ();

}

//.............................................................................

void
AXL_CDECL
VariantHashTable::insert (
	Variant key,
	Variant value
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = runtime->getFirstModule ()->m_typeMgr.getPrimitiveType (TypeKind_Variant);
	DataBox* box = (DataBox*) runtime->m_gcHeap.tryAllocateData (type);

	DataPtr valuePtr;
	valuePtr.m_p = box + 1;
	valuePtr.m_validator = &box->m_validator;

	(*m_hashTable) [key] = valuePtr;
	m_count = m_hashTable->getCount ();
}

//.............................................................................

} // namespace jnc
