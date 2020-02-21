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

#define _JNC_STDHASHTABLE_H

#include "jnc_StdMap.h"

typedef struct jnc_StdHashTable jnc_StdHashTable;

//..............................................................................

typedef
size_t
jnc_StdHashFunc(jnc_Variant key);

typedef
bool_t
jnc_StdIsEqualFunc(
	jnc_Variant key1,
	jnc_Variant key2
	);

//..............................................................................

JNC_EXTERN_C
jnc_StdHashTable*
jnc_createStdHashTable(
	jnc_Runtime* runtime,
	jnc_StdHashFunc* hashFunc,
	jnc_StdIsEqualFunc* isEqualFunc
	);

JNC_EXTERN_C
void
jnc_StdHashTable_clear(jnc_StdHashTable* hashTable);

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdHashTable_find(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	);

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdHashTable_add(
	jnc_StdHashTable* hashTable,
	jnc_Variant key,
	jnc_Variant value
	);

JNC_EXTERN_C
void
jnc_StdHashTable_remove(
	jnc_StdHashTable* hashTable,
	jnc_StdMapEntry* entry
	);

JNC_EXTERN_C
bool_t
jnc_StdHashTable_removeKey(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdHashTable
{
	jnc_IfaceHdr m_ifaceHdr;
	jnc_StdMap m_map;

#ifdef __cplusplus
	void
	clear()
	{
		jnc_StdHashTable_clear(this);
	}

	jnc_StdMapEntry*
	find(jnc_Variant key)
	{
		return jnc_StdHashTable_find(this, key);
	}

	jnc_StdMapEntry*
	add(
		jnc_Variant key,
		jnc_Variant value
		)
	{
		return jnc_StdHashTable_add(this, key, value);
	}

	void
	remove(jnc_StdMapEntry* entry)
	{
		jnc_StdHashTable_remove(this, entry);
	}

	bool
	removeKey(jnc_Variant key)
	{
		return jnc_StdHashTable_removeKey(this, key) != 0;
	}
#endif
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_StdHashTable   StdHashTable;
typedef jnc_StdHashFunc    StdHashFunc;
typedef jnc_StdIsEqualFunc StdIsEqualFunc;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
StdHashTable*
createStdHashTable(
	Runtime* runtime,
	StdHashFunc* hashFunc = NULL,
	StdIsEqualFunc* isEqualFunc = NULL
	)
{
	return jnc_createStdHashTable(runtime, hashFunc, isEqualFunc);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
