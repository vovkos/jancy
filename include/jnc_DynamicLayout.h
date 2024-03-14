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

#define _JNC_DYNAMICLAYOUT_H

#include "jnc_RuntimeStructs.h"
#include "jnc_ModuleItem.h"

typedef struct jnc_DynamicSection      jnc_DynamicSection;
typedef struct jnc_DynamicSectionGroup jnc_DynamicSectionGroup;
typedef struct jnc_DynamicLayout       jnc_DynamicLayout;

//..............................................................................

enum jnc_DynamicSectionKind {
	jnc_DynamicSectionKind_Undefined = 0,
	jnc_DynamicSectionKind_Struct,
	jnc_DynamicSectionKind_Array,
	jnc_DynamicSectionKind_Group,
};

typedef enum jnc_DynamicSectionKind jnc_DynamicSectionKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
size_t
jnc_DynamicSectionGroup_getSectionCount(jnc_DynamicSectionGroup* sectionGroup);

JNC_EXTERN_C
jnc_DynamicSection*
jnc_DynamicSectionGroup_getSection(
	jnc_DynamicSectionGroup* sectionGroup,
	size_t i
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicSectionGroup: jnc_IfaceHdr {
	size_t
	getSectionCount() {
		return jnc_DynamicSectionGroup_getSectionCount(this);
	}

	jnc_DynamicSection*
	getSection(size_t i) {
		return jnc_DynamicSectionGroup_getSection(this, i);
	}
};
#endif

//..............................................................................

JNC_EXTERN_C
jnc_DynamicSectionKind
jnc_DynamicSection_getSectionKind(jnc_DynamicSection* section);

JNC_EXTERN_C
size_t
jnc_DynamicSection_getOffset(jnc_DynamicSection* section);

JNC_EXTERN_C
size_t
jnc_DynamicSection_getElementCount(jnc_DynamicSection* section);

JNC_EXTERN_C
jnc_Type*
jnc_DynamicSection_getType(jnc_DynamicSection* section);

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_DynamicSection_getDecl(jnc_DynamicSection* section);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicSection: jnc_DynamicSectionGroup {
	jnc_DynamicSectionKind
	getSectionKind() {
		return jnc_DynamicSection_getSectionKind(this);
	}

	size_t
	getOffset() {
		return jnc_DynamicSection_getOffset(this);
	}

	size_t
	getElementCount() {
		return jnc_DynamicSection_getElementCount(this);
	}

	jnc_Type*
	getType() {
		return jnc_DynamicSection_getType(this);
	}

	jnc_ModuleItemDecl*
	getDecl() {
		return jnc_DynamicSection_getDecl(this);
	}
};
#endif

//..............................................................................

JNC_EXTERN_C
jnc_DynamicLayout*
jnc_createDynamicLayoutEx(
	jnc_Runtime* runtime,
	jnc_DataPtr ptr,
	size_t size
);

JNC_INLINE
jnc_DynamicLayout*
jnc_createDynamicLayout(jnc_Runtime* runtime) {
	return jnc_createDynamicLayoutEx(runtime, jnc_g_nullDataPtr, 0);
}

JNC_EXTERN_C
size_t
jnc_DynamicLayout_getSize(jnc_DynamicLayout* layout);

JNC_EXTERN_C
void
jnc_DynamicLayout_reset(
	jnc_DynamicLayout* layout,
	jnc_DataPtr ptr,
	size_t size
);

JNC_INLINE
void
jnc_DynamicLayout_clear(jnc_DynamicLayout* layout) {
	jnc_DynamicLayout_reset(layout, jnc_g_nullDataPtr, 0);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicLayout: jnc_DynamicSectionGroup {
	size_t
	getSize() {
		return jnc_DynamicLayout_getSize(this);
	}

	void
	clear() {
		jnc_DynamicLayout_clear(this);
	}

	void
	reset(
		jnc_DataPtr ptr,
		size_t size
	) {
		jnc_DynamicLayout_reset(this, ptr, size);
	}
};
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

typedef jnc_DynamicSectionKind  DynamicSectionKind;
typedef jnc_DynamicSection      DynamicSection;
typedef jnc_DynamicSectionGroup DynamicSectionGroup;
typedef jnc_DynamicLayout       DynamicLayout;

const DynamicSectionKind
	DynamicSectionKind_Undefined = jnc_DynamicSectionKind_Undefined,
	DynamicSectionKind_Struct    = jnc_DynamicSectionKind_Struct,
	DynamicSectionKind_Array     = jnc_DynamicSectionKind_Array,
	DynamicSectionKind_Group     = jnc_DynamicSectionKind_Group;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DynamicLayout*
createDynamicLayout(Runtime* runtime) {
	return jnc_createDynamicLayout(runtime);
}

inline
DynamicLayout*
createDynamicLayout(
	Runtime* runtime,
	DataPtr ptr,
	size_t size
) {
	return jnc_createDynamicLayoutEx(runtime, ptr, size);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
