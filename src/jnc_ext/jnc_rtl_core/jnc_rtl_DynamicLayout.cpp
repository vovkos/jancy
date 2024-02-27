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
#include "jnc_rtl_DynamicLayout.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_rtl_Type.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_Runtime.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicSectionGroup,
	"jnc.DynamicSectionGroup",
	sl::g_nullGuid,
	-1,
	DynamicSectionGroup,
	&DynamicSection::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicSectionGroup)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<DynamicSectionGroup>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicSectionGroup>)
	JNC_MAP_CONST_PROPERTY("m_sectionArray", &DynamicSectionGroup::getSection)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicSection,
	"jnc.DynamicSection",
	sl::g_nullGuid,
	-1,
	DynamicSection,
	&DynamicSection::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicSection)
	JNC_MAP_CONST_PROPERTY("m_decl", &DynamicSection::getDecl)
	JNC_MAP_CONST_PROPERTY("m_type", &DynamicSection::getType_0)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicLayout,
	"jnc.DynamicLayout",
	sl::g_nullGuid,
	-1,
	DynamicLayout,
	&DynamicLayout::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayout)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<DynamicLayout>)
	JNC_MAP_OVERLOAD(&(jnc::construct<DynamicLayout, DataPtr, size_t>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayout>)
	JNC_MAP_FUNCTION("addStruct", &DynamicLayout::addStruct)
	JNC_MAP_FUNCTION("addArray", &DynamicLayout::addArray)
	JNC_MAP_FUNCTION("openGroup", &DynamicLayout::openGroup)
	JNC_MAP_FUNCTION("closeGroup", &DynamicLayout::closeGroup)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
DynamicSectionGroup::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_sectionArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_sectionArray[i]);
}

//..............................................................................

rtl::ModuleItemDecl*
JNC_CDECL
DynamicSection::getDecl() {
	return m_decl ? rtl::getModuleItemDecl(m_decl) : NULL;
}

IfaceHdr*
JNC_CDECL
DynamicSection::getType_0() {
	return m_type ? rtl::getType(m_type) : NULL;
}

//..............................................................................

void
JNC_CDECL
DynamicLayout::reset(
	DataPtr ptr,
	size_t size
) {
	m_ptr = m_basePtr = ptr;
	m_endPtr.m_p = (char*)ptr.m_p + size;
	m_endPtr.m_validator = ptr.m_validator;
	m_sectionCount = 0;
	m_groupStack.clear();
	m_sectionArray.clear();
}

size_t
JNC_CDECL
DynamicLayout::addStruct(ct::StructType* type) {
	size_t offset = getSize();
	DynamicSection* section = createClass<DynamicSection>(
		jnc::getCurrentThreadRuntime(),
		DynamicSectionKind_Struct,
		offset,
		(ct::ModuleItemDecl*)NULL,
		type
	);

	addSection(section);
	m_ptr.m_p = (char*)m_ptr.m_p + type->getSize();
	return offset;
}

size_t
JNC_CDECL
DynamicLayout::addArray(
	ct::ModuleItemDecl* decl,
	ct::Type* type,
	size_t elementCount
) {
	size_t offset = getSize();
	DynamicSection* section = createClass<DynamicSection>(
		jnc::getCurrentThreadRuntime(),
		DynamicSectionKind_Array,
		getSize(),
		decl,
		type
	);

	section->m_elementCount = elementCount;
	addSection(section);
	m_ptr.m_p = (char*)m_ptr.m_p + type->getSize() * elementCount;
	return offset;
}

void
DynamicLayout::addSection(DynamicSection* section) {
	DynamicSectionGroup* group = !m_groupStack.isEmpty() ? m_groupStack.getBack() : this;
	group->m_sectionArray.append(section);
	group->m_sectionCount++;
}

void
JNC_CDECL
DynamicLayout::openGroup(ct::ModuleItemDecl* decl) {
	DynamicSection* section = createClass<DynamicSection>(
		jnc::getCurrentThreadRuntime(),
		DynamicSectionKind_Group,
		getSize(),
		decl
	);

	addSection(section);
	m_groupStack.append(section);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
