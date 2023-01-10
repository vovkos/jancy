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
#include "jnc_StdRbTree.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_std_RbTree.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_StdRbTree*
jnc_createStdRbTree(
	jnc_Runtime* runtime,
	jnc_StdCmpFunc* cmpFunc
) {
	return jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_createStdRbTreeFunc(runtime, cmpFunc);
}

JNC_EXTERN_C
void
jnc_StdRbTree_clear(jnc_StdRbTree* RbTree) {
	jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_clearFunc(RbTree);
}

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdRbTree_find(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
) {
	return jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_findFunc(RbTree, key);
}

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdRbTree_add(
	jnc_StdRbTree* RbTree,
	jnc_Variant key,
	jnc_Variant value
) {
	return jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_addFunc(RbTree, key, value);
}

JNC_EXTERN_C
void
jnc_StdRbTree_remove(
	jnc_StdRbTree* RbTree,
	jnc_StdMapEntry* entry
) {
	jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_removeFunc(RbTree, entry);
}

JNC_EXTERN_C
bool_t
jnc_StdRbTree_removeKey(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
) {
	return jnc_g_dynamicExtensionLibHost->m_stdRbTreeFuncTable->m_removeKeyFunc(RbTree, key);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdRbTree*
jnc_createStdRbTree(
	jnc_Runtime* runtime,
	jnc_StdCmpFunc* cmpFunc
) {
	return (jnc_StdRbTree*)jnc::createClass<jnc::std::RbTree>(runtime, cmpFunc);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_StdRbTree_clear(jnc_StdRbTree* RbTree) {
	 ((jnc::std::RbTree*)RbTree)->clear();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdMapEntry*
jnc_StdRbTree_find(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
) {
	 return (jnc_StdMapEntry*)jnc::std::RbTree::find((jnc::std::RbTree*)RbTree, key).m_p;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_StdMapEntry*
jnc_StdRbTree_add(
	jnc_StdRbTree* RbTree,
	jnc_Variant key,
	jnc_Variant value
) {
	 jnc_StdMapEntry* entry = (jnc_StdMapEntry*)jnc::std::RbTree::visit((jnc::std::RbTree*)RbTree, key).m_p;
	 entry->m_value = value;
	 return entry;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_StdRbTree_remove(
	jnc_StdRbTree* RbTree,
	jnc_StdMapEntry* entry
) {
	ASSERT(entry->m_map == &RbTree->m_map);
	((jnc::std::RbTree*)RbTree)->removeImpl((jnc::std::MapEntry*)entry);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_StdRbTree_removeKey(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
) {
	jnc_DataPtr itPtr = jnc::std::RbTree::find((jnc::std::RbTree*)RbTree, key);
	if (!itPtr.m_p)
		return false;

	jnc_StdRbTree_remove(RbTree, (jnc_StdMapEntry*)itPtr.m_p);
	return true;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
