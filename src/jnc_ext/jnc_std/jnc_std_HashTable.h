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

#include "jnc_std_Map.h"
#include "jnc_StdHashTable.h"

namespace jnc {
namespace std {

JNC_DECLARE_OPAQUE_CLASS_TYPE(HashTable)

typedef StdHashFunc    HashFunc;
typedef StdIsEqualFunc IsEqualFunc;

//..............................................................................

inline
size_t
hashVariant(Variant key) {
	return key.hash();
}

inline
bool_t
isEqualVariant(
	Variant key1,
	Variant key2
) {
	return key1.isEqual(key2);
}

//..............................................................................

class HashIndirect {
protected:
	HashFunc* m_func;

public:
	HashIndirect(HashFunc* func = NULL) {
		m_func = func ? func : hashVariant;
	}

	size_t
	operator () (const Variant& key) const {
		ASSERT(m_func);
		return m_func(key);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class IsEqualIndirect {
protected:
	IsEqualFunc* m_func;

public:
	IsEqualIndirect(IsEqualFunc* func = NULL) {
		m_func = func ? func : isEqualVariant;
	}

	bool
	operator () (
		const Variant& key1,
		const Variant& key2
	) const {
		ASSERT(m_func);
		return m_func(key1, key2) != 0;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HashTable: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HashTable)

public:
	Map m_map;

protected:
	sl::HashTable<Variant, DataPtr, HashIndirect, IsEqualIndirect> m_hashTable;

public:
	HashTable(
		HashFunc* hashFunc,
		IsEqualFunc* isEqualFunc
	):
		m_hashTable(HashIndirect(hashFunc), IsEqualIndirect(isEqualFunc)) {}

	void
	JNC_CDECL
	clear() {
		m_map.clear();
		m_hashTable.clear();
	}

	static
	DataPtr
	visit(
		HashTable* self,
		Variant key
	) {
		return self->visitImpl(key);
	}

	static
	DataPtr
	find(
		HashTable* self,
		Variant key
	) {
		return self->m_hashTable.findValue(key, g_nullDataPtr);
	}

	void
	JNC_CDECL
	remove(DataPtr entryPtr) {
		removeImpl((MapEntry*)entryPtr.m_p);
	}

public:
	void
	removeImpl(MapEntry* entry);

protected:
	DataPtr
	visitImpl(Variant key);
};

//..............................................................................

} // namespace std
} // namespace jnc
