#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"
#include "jnc_rt_VariantUtils.h"

namespace jnc {
namespace std {

//.............................................................................

class StringHashTable: public rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("std.StringHashTable", g_stdLibCacheSlot, StdLibTypeCacheSlot_StringHashTable)
		JNC_MAP_CONSTRUCTOR (&sl::construct <StringHashTable>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <StringHashTable>)
		JNC_MAP_FUNCTION ("clear",  &StringHashTable::clear)
		JNC_MAP_FUNCTION ("find", &StringHashTable::find)
		JNC_MAP_FUNCTION ("insert", &StringHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &StringHashTable::remove)
	JNC_END_CLASS_TYPE_MAP ()

public:
	typedef sl::StringHashTableMap <rt::DataPtr> StringHashTableMap;

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
	rt::DataPtr
	find (
		StringHashTable* self,
		rt::DataPtr keyPtr
		)
	{
		StringHashTableMap::Iterator it = self->m_hashTable->find ((const char*) keyPtr.m_p);
		return it ? it->m_value : rt::g_nullPtr;
	}

	void
	AXL_CDECL
	insert (
		rt::DataPtr keyPtr,
		rt::Variant value
		);

	bool
	AXL_CDECL
	remove (rt::DataPtr keyPtr)
	{
		return m_hashTable->eraseByKey ((const char*) keyPtr.m_p);
	}
};

//.............................................................................

class VariantHashTable: public rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("std.VariantHashTable", g_stdLibCacheSlot, StdLibTypeCacheSlot_VariantHashTable)
		JNC_MAP_CONSTRUCTOR (&sl::construct <VariantHashTable>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <VariantHashTable>)
		JNC_MAP_FUNCTION ("clear",  &VariantHashTable::clear)
		JNC_MAP_FUNCTION ("find", &VariantHashTable::find)
		JNC_MAP_FUNCTION ("insert", &VariantHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &VariantHashTable::remove)
	JNC_END_CLASS_TYPE_MAP ()

public:
	typedef sl::HashTableMap <rt::Variant, rt::DataPtr, rt::HashVariant, rt::CmpVariant> VariantHashTableMap;

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
	rt::DataPtr
	find (
		VariantHashTable* self,
		rt::Variant key
		)
	{
		VariantHashTableMap::Iterator it = self->m_hashTable->find (key);
		return it ? it->m_value : rt::g_nullPtr;
	}

	void
	AXL_CDECL
	insert (
		rt::Variant key,
		rt::Variant value
		);

	bool
	AXL_CDECL
	remove (rt::Variant key)
	{
		return m_hashTable->eraseByKey (key);
	}
};

//.............................................................................

} // namespace std
} // namespace jnc
