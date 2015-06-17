#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {

//.............................................................................

class StringHashTable: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE ("jnc.StringHashTable", StdApiSlot_StringHashTable)
		JNC_CONSTRUCTOR (&StringHashTable::construct)
		JNC_FUNCTION ("clear",  &StringHashTable::clear)
		JNC_FUNCTION ("find", &StringHashTable::find_s)
		JNC_FUNCTION ("insert", &StringHashTable::insert)
	JNC_END_TYPE ()

public:
	rtl::StringHashTableMap <Variant>* m_hashTable;
	size_t m_count;

public:
	void
	AXL_CDECL
	construct ()
	{
		m_hashTable = AXL_MEM_NEW (rtl::StringHashTableMap <Variant>);
		m_count = 0;
	}

	void
	AXL_CDECL
	clear ()
	{
		m_hashTable->clear ();
		m_count = 0;
	}

	static
	DataPtr
	find_s (
		StringHashTable* self,
		DataPtr keyPtr
		);

	void
	AXL_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		)
	{
		(*m_hashTable) [(const char*) keyPtr.m_p] = value;
		m_count = m_hashTable->getCount ();
	}
};

//.............................................................................

class VariantHashTable: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE ("jnc.VariantHashTable", StdApiSlot_VariantHashTable)
		JNC_CONSTRUCTOR (&VariantHashTable::construct)
		JNC_FUNCTION ("clear",  &VariantHashTable::clear)
		JNC_FUNCTION ("find", &VariantHashTable::find_s)
		JNC_FUNCTION ("insert", &VariantHashTable::insert)
	JNC_END_TYPE ()

public:
	typedef rtl::HashTableMap <Variant, Variant, HashVariant, CmpVariant> VariantHashTableMap;

public:
	VariantHashTableMap* m_hashTable;
	size_t m_count;

public:
	void
	AXL_CDECL
	construct ()
	{
		m_hashTable = AXL_MEM_NEW (VariantHashTableMap);
		m_count = 0;
	}

	void
	AXL_CDECL
	clear ()
	{
		m_hashTable->clear ();
		m_count = 0;
	}

	static
	DataPtr
	find_s (
		VariantHashTable* self,
		Variant key
		);

	void
	AXL_CDECL
	insert (
		Variant key,
		Variant value
		)
	{
		(*m_hashTable) [key] = value;
		m_count = m_hashTable->getCount ();
	}
};

//.............................................................................

} // namespace jnc
