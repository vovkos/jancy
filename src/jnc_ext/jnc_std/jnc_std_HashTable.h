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

#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_Variant.h"

namespace jnc {
namespace std {

class HashTable;

JNC_DECLARE_OPAQUE_CLASS_TYPE (HashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (StringHashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (VariantHashTable)

//..............................................................................

struct MapEntry
{
	DataPtr m_nextPtr;
	DataPtr m_prevPtr;

	HashTable* m_hashTable;

	Variant m_key;
	Variant m_value;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef 
size_t 
HashFunc (Variant key);

typedef 
bool 
IsEqualFunc (
	Variant key1,
	Variant key2
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HashTable: public IfaceHdr
{
public:
	DataPtr m_head;
	DataPtr m_tail;
	size_t m_count;

protected:
	HashFunc* m_hashFunc;
	IsEqualFunc* m_eqFunc;
		
public:
	HashTable (
		HashFunc* hashFunc,
		IsEqualFunc* isEqualFunc
		);

	~HashTable ();

	void
	clear ();

	DataPtr
	visit (Variant key) const;

	DataPtr
	find (Variant key) const;

	void
	remove (DataPtr entryPtr);
};

//..............................................................................

class StringHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		DataPtr m_keyPtr;
		Variant m_value;
	};

	typedef sl::StringHashTable <Entry*> Map;

protected:
	sl::StdList <Entry> m_list;
	Map m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		DataPtr keyPtr,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		);

	bool
	JNC_CDECL
	remove (DataPtr keyPtr);
};

//..............................................................................

class VariantHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		Variant m_key;
		Variant m_value;
	};

	typedef sl::DuckTypeHashTable <Variant, Entry*> Map;

protected:
	sl::StdList <Entry> m_list;
	Map m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		Variant key,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		Variant key,
		Variant value
		);

	bool
	JNC_CDECL
	remove (Variant key);
};

//..............................................................................

} // namespace std
} // namespace jnc
