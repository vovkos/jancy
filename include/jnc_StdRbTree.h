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

#define _JNC_STDRBTREE_H

#include "jnc_StdMap.h"

typedef struct jnc_StdRbTree jnc_StdRbTree;

//..............................................................................

typedef
int
jnc_StdCmpFunc(
	jnc_Variant key1,
	jnc_Variant key2
);

//..............................................................................

JNC_EXTERN_C
jnc_StdRbTree*
jnc_createStdRbTree(
	jnc_Runtime* runtime,
	jnc_StdCmpFunc* cmpFunc
);

JNC_EXTERN_C
void
jnc_StdRbTree_clear(jnc_StdRbTree* RbTree);

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdRbTree_find(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
);

JNC_EXTERN_C
jnc_StdMapEntry*
jnc_StdRbTree_add(
	jnc_StdRbTree* RbTree,
	jnc_Variant key,
	jnc_Variant value
);

JNC_EXTERN_C
void
jnc_StdRbTree_remove(
	jnc_StdRbTree* RbTree,
	jnc_StdMapEntry* entry
);

JNC_EXTERN_C
bool_t
jnc_StdRbTree_removeKey(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdRbTree {
	jnc_IfaceHdr m_ifaceHdr;
	jnc_StdMap m_map;

#ifdef __cplusplus
	void
	clear() {
		jnc_StdRbTree_clear(this);
	}

	jnc_StdMapEntry*
	find(jnc_Variant key) {
		return jnc_StdRbTree_find(this, key);
	}

	jnc_StdMapEntry*
	add(
		jnc_Variant key,
		jnc_Variant value
	) {
		return jnc_StdRbTree_add(this, key, value);
	}

	void
	remove(jnc_StdMapEntry* entry) {
		jnc_StdRbTree_remove(this, entry);
	}

	bool
	removeKey(jnc_Variant key) {
		return jnc_StdRbTree_removeKey(this, key) != 0;
	}
#endif
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_StdRbTree  StdRbTree;
typedef jnc_StdCmpFunc StdCmpFunc;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
StdRbTree*
createStdRbTree(
	Runtime* runtime,
	StdCmpFunc* cmpFunc = NULL
) {
	return jnc_createStdRbTree(runtime, cmpFunc);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
