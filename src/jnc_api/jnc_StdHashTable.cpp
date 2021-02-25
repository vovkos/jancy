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

#include "pch.h"
#include "jnc_StdHashTable.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_rtl_Promise.h"
#	include "jnc_std_HashTable.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_StdHashTable*
jnc_createStdHashTable(
	jnc_Runtime* runtime,
	jnc_StdHashFunc* hashFunc,
	jnc_StdIsEqualFunc* isEqualFunc
	)
{
	return jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_createStdHashTableFunc(runtime, hashFunc, isEqualFunc);
}

JNC_EXTERN_C
void
jnc_StdHashTable_clear(jnc_StdHashTable* hashTable)
{
	jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_clearFunc(hashTable);
}

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdHashTable_find(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	)
{
	return jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_findFunc(hashTable, key);
}

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdHashTable_add(
	jnc_StdHashTable* hashTable,
	jnc_Variant key,
	jnc_Variant value
	)
{
	return jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_addFunc(hashTable, key, value);
}

JNC_EXTERN_C
void
jnc_StdHashTable_remove(
	jnc_StdHashTable* hashTable,
	jnc_StdMapEntry* entry
	)
{
	jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_removeFunc(hashTable, entry);
}

JNC_EXTERN_C
bool_t
jnc_StdHashTable_removeKey(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	)
{
	return jnc_g_dynamicExtensionLibHost->m_stdHashTableFuncTable->m_removeKeyFunc(hashTable, key);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdHashTable*
jnc_createStdHashTable(
	jnc_Runtime* runtime,
	jnc_StdHashFunc* hashFunc,
	jnc_StdIsEqualFunc* isEqualFunc
	)
{
	return (jnc_StdHashTable*)jnc::createClass<jnc::std::HashTable>(runtime, hashFunc, isEqualFunc);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_StdHashTable_clear(jnc_StdHashTable* hashTable)
{
	 ((jnc::std::HashTable*)hashTable)->clear();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdMapEntry*
jnc_StdHashTable_find(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	)
{
	 return (jnc_StdMapEntry*)jnc::std::HashTable::find((jnc::std::HashTable*)hashTable, key).m_p;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdMapEntry*
jnc_StdHashTable_add(
	jnc_StdHashTable* hashTable,
	jnc_Variant key,
	jnc_Variant value
	)
{
	 jnc_StdMapEntry* entry = (jnc_StdMapEntry*)jnc::std::HashTable::visit((jnc::std::HashTable*)hashTable, key).m_p;
	 entry->m_value = value;
	 return entry;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_StdHashTable_remove(
	jnc_StdHashTable* hashTable,
	jnc_StdMapEntry* entry
	)
{
	ASSERT(entry->m_map == &hashTable->m_map);
	((jnc::std::HashTable*)hashTable)->removeImpl((jnc::std::MapEntry*)entry);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_StdHashTable_removeKey(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	)
{
	jnc_DataPtr itPtr = jnc::std::HashTable::find((jnc::std::HashTable*)hashTable, key);
	if (!itPtr.m_p)
		return false;

	jnc_StdHashTable_remove(hashTable, (jnc_StdMapEntry*)itPtr.m_p);
	return true;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
