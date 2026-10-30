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
#include "jnc_DynamicLayoutSection.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_rtl_DynamicLayoutSection.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayoutSectionGroup_getSectionCount(jnc_DynamicLayoutSectionGroup* sectionGroup) {
	return ((jnc::rtl::DynamicLayoutSectionGroup*)sectionGroup)->m_sectionCount;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayoutSection*
jnc_DynamicLayoutSectionGroup_getSection(
	jnc_DynamicLayoutSectionGroup* sectionGroup,
	size_t i
) {
	return (jnc_DynamicLayoutSection*)((jnc::rtl::DynamicLayoutSectionGroup*)sectionGroup)->getSection(i);
}

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayoutSectionKind
jnc_DynamicLayoutSection_getSectionKind(jnc_DynamicLayoutSection* section) {
	return (jnc_DynamicLayoutSectionKind)((jnc::rtl::DynamicLayoutSection*)section)->m_sectionKind;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayoutSection_getOffset(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_offset;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayoutSection_getSize(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_size;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayoutSection_getElementCount(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_elementCount;
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DynamicLayoutSection_getBitOffset(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_bitOffset;
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DynamicLayoutSection_getBitCount(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_bitCount;
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DynamicLayoutSection_getPtrTypeFlags(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->m_ptrTypeFlags;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_DynamicLayoutSection_getType(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->getType_ct();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItemDecl*
jnc_DynamicLayoutSection_getDecl(jnc_DynamicLayoutSection* section) {
	return ((jnc::rtl::DynamicLayoutSection*)section)->getDecl_ct();
}

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayoutSectionBuilder*
jnc_createDynamicLayoutSectionBuilder(jnc_Runtime* runtime) {
	return (jnc_DynamicLayoutSectionBuilder*)jnc::createClass<jnc::rtl::DynamicLayoutSectionBuilder>(runtime);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayoutSectionGroup*
jnc_DynamicLayoutSectionBuilder_getSectionGroup(jnc_DynamicLayoutSectionBuilder* builder) {
	return (jnc_DynamicLayoutSectionGroup*)((jnc::rtl::DynamicLayoutSectionBuilder*)builder)->m_sectionGroup;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_DynamicLayoutSectionBuilder_reset(jnc_DynamicLayoutSectionBuilder* builder) {
	((jnc::rtl::DynamicLayoutSectionBuilder*)builder)->reset();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
