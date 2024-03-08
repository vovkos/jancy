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
#include "jnc_DynamicLayout.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_rtl_DynamicLayout.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicSectionGroup_getSectionCount(jnc_DynamicSectionGroup* sectionGroup) {
	return ((jnc::rtl::DynamicSectionGroup*)sectionGroup)->m_sectionCount;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicSection*
jnc_DynamicSectionGroup_getSection(
	jnc_DynamicSectionGroup* sectionGroup,
	size_t i
) {
	return (jnc_DynamicSection*)((jnc::rtl::DynamicSectionGroup*)sectionGroup)->getSection(i);
}

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicSectionKind
jnc_DynamicSection_getSectionKind(jnc_DynamicSection* section) {
	return (jnc_DynamicSectionKind)((jnc::rtl::DynamicSection*)section)->m_sectionKind;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicSection_getOffset(jnc_DynamicSection* section) {
	return ((jnc::rtl::DynamicSection*)section)->m_offset;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicSection_getElementCount(jnc_DynamicSection* section) {
	return ((jnc::rtl::DynamicSection*)section)->m_elementCount;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_DynamicSection_getType(jnc_DynamicSection* section) {
	return ((jnc::rtl::DynamicSection*)section)->getType_ct();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItemDecl*
jnc_DynamicSection_getDecl(jnc_DynamicSection* section) {
	return ((jnc::rtl::DynamicSection*)section)->getDecl_ct();
}

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayout*
jnc_createDynamicLayoutEx(
	jnc_Runtime* runtime,
	jnc_DataPtr ptr,
	size_t size
) {
	return (jnc_DynamicLayout*)jnc::createClass<jnc::rtl::DynamicLayout>(runtime, ptr, size);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayout_getSize(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->getSize();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_DynamicLayout_reset(
	jnc_DynamicLayout* layout,
	jnc_DataPtr ptr,
	size_t size
) {
	((jnc::rtl::DynamicLayout*)layout)->reset(ptr, size);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
