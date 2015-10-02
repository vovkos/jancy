#pragma once

#include "jnc_StdLibSlots.h"
#include "jnc_Variant.h"

namespace jnc {

//.............................................................................

class StringHashTable: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("jnc.StringHashTable", g_stdLibSlot, StdLibTypeSlot_StringHashTable)
		JNC_MAP_CONSTRUCTOR (&rtl::construct <StringHashTable>)
		JNC_MAP_DESTRUCTOR (&rtl::destruct <StringHashTable>)
		JNC_MAP_FUNCTION ("clear",  &StringHashTable::clear)
		JNC_MAP_FUNCTION ("find", &StringHashTable::find)
		JNC_MAP_FUNCTION ("insert", &StringHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &StringHashTable::remove)
	JNC_END_TYPE_MAP ()

public:
	typedef rtl::StringHashTableMap <DataPtr> StringHashTableMap;

public:
	StringHashTableMap* m_hashTable;
	size_t m_count;

public:
	StringHashTable ()
	{
		m_hashTable = AXL_MEM_NEW (StringHashTableMap);
		m_count = 0;
	}

	~StringHashTable ();

	void
	AXL_CDECL
	clear ()
	{
		m_hashTable->clear ();
		m_count = 0;
	}

	static
	DataPtr
	find (
		StringHashTable* self,
		DataPtr keyPtr
		)
	{
		StringHashTableMap::Iterator it = self->m_hashTable->find ((const char*) keyPtr.m_p);
		return it ? it->m_value : g_nullPtr;
	}

	void
	AXL_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		);

	bool
	AXL_CDECL
	remove (DataPtr keyPtr)
	{
		return m_hashTable->eraseByKey ((const char*) keyPtr.m_p);
	}
};

//.............................................................................

class VariantHashTable: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("jnc.VariantHashTable", g_stdLibSlot, StdLibTypeSlot_VariantHashTable)
		JNC_MAP_CONSTRUCTOR (&rtl::construct <VariantHashTable>)
		JNC_MAP_DESTRUCTOR (&rtl::destruct <VariantHashTable>)
		JNC_MAP_FUNCTION ("clear",  &VariantHashTable::clear)
		JNC_MAP_FUNCTION ("find", &VariantHashTable::find)
		JNC_MAP_FUNCTION ("insert", &VariantHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &VariantHashTable::remove)
	JNC_END_TYPE_MAP ()

public:
	typedef rtl::HashTableMap <Variant, DataPtr, HashVariant, CmpVariant> VariantHashTableMap;

public:
	VariantHashTableMap* m_hashTable;
	size_t m_count;

public:
	VariantHashTable ()
	{
		m_hashTable = AXL_MEM_NEW (VariantHashTableMap);
		m_count = 0;
	}

	~VariantHashTable ();

	void
	AXL_CDECL
	clear ()
	{
		m_hashTable->clear ();
		m_count = 0;
	}

	static
	DataPtr
	find (
		VariantHashTable* self,
		Variant key
		)
	{
		VariantHashTableMap::Iterator it = self->m_hashTable->find (key);
		return it ? it->m_value : g_nullPtr;
	}

	void
	AXL_CDECL
	insert (
		Variant key,
		Variant value
		);

	bool
	AXL_CDECL
	remove (Variant key)
	{
		return m_hashTable->eraseByKey (key);
	}
};

//.............................................................................

} // namespace jnc
