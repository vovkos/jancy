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

#define _JNC_DYNAMICLAYOUTSECTION_H

#include "jnc_DynamicLayout.h"

typedef struct jnc_DynamicLayoutSectionGroup   jnc_DynamicLayoutSectionGroup;
typedef struct jnc_DynamicLayoutSection        jnc_DynamicLayoutSection;
typedef struct jnc_DynamicLayoutSectionBuilder jnc_DynamicLayoutSectionBuilder;

//..............................................................................

enum jnc_DynamicLayoutSectionKind {
	jnc_DynamicLayoutSectionKind_Undefined = 0,
	jnc_DynamicLayoutSectionKind_Struct,
	jnc_DynamicLayoutSectionKind_Array,
	jnc_DynamicLayoutSectionKind_Field,
	jnc_DynamicLayoutSectionKind_Group,
};

typedef enum jnc_DynamicLayoutSectionKind jnc_DynamicLayoutSectionKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
size_t
jnc_DynamicLayoutSectionGroup_getSectionCount(jnc_DynamicLayoutSectionGroup* sectionGroup);

JNC_EXTERN_C
jnc_DynamicLayoutSection*
jnc_DynamicLayoutSectionGroup_getSection(
	jnc_DynamicLayoutSectionGroup* sectionGroup,
	size_t i
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicLayoutSectionGroup: jnc_IfaceHdr {
	size_t
	getSectionCount() {
		return jnc_DynamicLayoutSectionGroup_getSectionCount(this);
	}

	jnc_DynamicLayoutSection*
	getSection(size_t i) {
		return jnc_DynamicLayoutSectionGroup_getSection(this, i);
	}
};
#endif

//..............................................................................

JNC_EXTERN_C
jnc_DynamicLayoutSectionKind
jnc_DynamicLayoutSection_getSectionKind(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
size_t
jnc_DynamicLayoutSection_getOffset(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
size_t
jnc_DynamicLayoutSection_getSize(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
size_t
jnc_DynamicLayoutSection_getElementCount(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
uint_t
jnc_DynamicLayoutSection_getBitOffset(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
uint_t
jnc_DynamicLayoutSection_getBitCount(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
uint_t
jnc_DynamicLayoutSection_getPtrTypeFlags(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
jnc_Type*
jnc_DynamicLayoutSection_getType(jnc_DynamicLayoutSection* section);

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_DynamicLayoutSection_getDecl(jnc_DynamicLayoutSection* section);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicLayoutSection: jnc_DynamicLayoutSectionGroup {
	jnc_DynamicLayoutSectionKind
	getSectionKind() {
		return jnc_DynamicLayoutSection_getSectionKind(this);
	}

	size_t
	getOffset() {
		return jnc_DynamicLayoutSection_getOffset(this);
	}

	size_t
	getSize() {
		return jnc_DynamicLayoutSection_getSize(this);
	}

	size_t
	getElementCount() {
		return jnc_DynamicLayoutSection_getElementCount(this);
	}

	uint_t
	getBitOffset() {
		return jnc_DynamicLayoutSection_getBitOffset(this);
	}

	uint_t
	getBitCount() {
		return jnc_DynamicLayoutSection_getBitCount(this);
	}

	uint_t
	getPtrTypeFlags() {
		return jnc_DynamicLayoutSection_getPtrTypeFlags(this);
	}

	jnc_Type*
	getType() {
		return jnc_DynamicLayoutSection_getType(this);
	}

	jnc_ModuleItemDecl*
	getDecl() {
		return jnc_DynamicLayoutSection_getDecl(this);
	}
};
#endif

//..............................................................................

JNC_EXTERN_C
jnc_DynamicLayoutSectionBuilder*
jnc_createDynamicLayoutSectionBuilder(jnc_Runtime* runtime);

JNC_EXTERN_C
jnc_DynamicLayoutSectionGroup*
jnc_DynamicLayoutSectionBuilder_getSectionGroup(jnc_DynamicLayoutSectionBuilder* builder);

JNC_EXTERN_C
void
jnc_DynamicLayoutSectionBuilder_reset(jnc_DynamicLayoutSectionBuilder* builder);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef __cplusplus
struct jnc_DynamicLayoutSectionBuilder: jnc_DynamicLayoutSink {
	jnc_DynamicLayoutSectionGroup*
	getSectionGroup() {
		return jnc_DynamicLayoutSectionBuilder_getSectionGroup(this);
	}

	void
	reset() {
		jnc_DynamicLayoutSectionBuilder_reset(this);
	}
};
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

typedef jnc_DynamicLayoutSectionGroup   DynamicLayoutSectionGroup;
typedef jnc_DynamicLayoutSection        DynamicLayoutSection;
typedef jnc_DynamicLayoutSectionBuilder DynamicLayoutSectionBuilder;
typedef jnc_DynamicLayoutSectionKind    DynamicLayoutSectionKind;

const DynamicLayoutSectionKind
	DynamicLayoutSectionKind_Undefined = jnc_DynamicLayoutSectionKind_Undefined,
	DynamicLayoutSectionKind_Struct    = jnc_DynamicLayoutSectionKind_Struct,
	DynamicLayoutSectionKind_Array     = jnc_DynamicLayoutSectionKind_Array,
	DynamicLayoutSectionKind_Field     = jnc_DynamicLayoutSectionKind_Field,
	DynamicLayoutSectionKind_Group     = jnc_DynamicLayoutSectionKind_Group;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DynamicLayoutSectionBuilder*
createDynamicLayoutSectionBuilder(Runtime* runtime) {
	return jnc_createDynamicLayoutSectionBuilder(runtime);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus
