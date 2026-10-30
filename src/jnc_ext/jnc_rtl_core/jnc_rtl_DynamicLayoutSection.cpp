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
#include "jnc_rtl_DynamicLayoutSection.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_rtl_Type.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicLayoutSectionGroup,
	"jnc.DynamicLayoutSectionGroup",
	sl::g_nullGuid,
	-1,
	DynamicLayoutSectionGroup,
	&DynamicLayoutSectionGroup::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayoutSectionGroup)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<DynamicLayoutSectionGroup>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayoutSectionGroup>)
	JNC_MAP_CONST_PROPERTY("m_sectionArray", &DynamicLayoutSectionGroup::getSection)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicLayoutSection,
	"jnc.DynamicLayoutSection",
	sl::g_nullGuid,
	-1,
	DynamicLayoutSection,
	&DynamicLayoutSection::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayoutSection)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayoutSection>)
	JNC_MAP_CONST_PROPERTY("m_decl", &DynamicLayoutSection::getDecl_rtl)
	JNC_MAP_CONST_PROPERTY("m_type", &DynamicLayoutSection::getType_rtl)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE_REQ(
	DynamicLayoutSectionBuilder,
	"jnc.DynamicLayoutSectionBuilder",
	sl::g_nullGuid,
	-1,
	DynamicLayoutSectionBuilder,
	NULL
)

JNC_BEGIN_OPAQUE_CLASS_REQUIRE_TABLE(DynamicLayoutSectionBuilder)
	JNC_OPAQUE_CLASS_REQUIRE_TYPE(TypeKind_Class, "jnc.DynamicLayoutSectionGroup")
	JNC_OPAQUE_CLASS_REQUIRE_TYPE(TypeKind_Class, "jnc.DynamicLayoutSection")
JNC_END_OPAQUE_CLASS_REQUIRE_TABLE()

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayoutSectionBuilder)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<DynamicLayoutSectionBuilder>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayoutSectionBuilder>)
	JNC_MAP_FUNCTION("reset", &DynamicLayoutSectionBuilder::reset)
	JNC_MAP_FUNCTION("onStruct", &DynamicLayoutSectionBuilder::onStruct)
	JNC_MAP_FUNCTION("onArray", &DynamicLayoutSectionBuilder::onArray)
	JNC_MAP_FUNCTION("onField", &DynamicLayoutSectionBuilder::onField)
	JNC_MAP_FUNCTION("onBitField", &DynamicLayoutSectionBuilder::onBitField)
	JNC_MAP_FUNCTION("onGroupOpened", &DynamicLayoutSectionBuilder::onGroupOpened)
	JNC_MAP_FUNCTION("onGroupClosed", &DynamicLayoutSectionBuilder::onGroupClosed)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
DynamicLayoutSectionGroup::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_sectionArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_sectionArray[i]);
}

//..............................................................................

void
JNC_CDECL
DynamicLayoutSection::markOpaqueGcRoots(GcHeap* gcHeap) {
	DynamicLayoutSectionGroup::markOpaqueGcRoots(gcHeap);
	gcHeap->markClassPtr(m_decl);
	gcHeap->markClassPtr(m_type);
}

ct::ModuleItemDecl*
DynamicLayoutSection::getDecl_ct() {
	return m_decl ? m_decl->getDecl() : NULL;
}

ct::Type*
DynamicLayoutSection::getType_ct() {
	return m_type ? m_type->getItem() : NULL;
}

//..............................................................................

DynamicLayoutSectionBuilder::DynamicLayoutSectionBuilder() {
	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime); // the sink itself is not rooted yet
	m_sectionGroup = createClass<DynamicLayoutSectionGroup>(runtime);
}

void
JNC_CDECL
DynamicLayoutSectionBuilder::reset() {
	m_groupStack.clear();
	m_sectionGroup = createClass<DynamicLayoutSectionGroup>(getCurrentThreadRuntime());
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onStruct(
	Type* type,
	size_t offset,
	DataPtr ptr
) {
	addSection(DynamicLayoutSectionKind_Struct, NULL, type, offset, type->getItem()->getSize());
	return true;
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onArray(
	ModuleItemDecl* decl,
	Type* type,
	size_t elementCount,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
) {
	DynamicLayoutSection* section = addSection(
		DynamicLayoutSectionKind_Array,
		decl,
		type,
		offset,
		type->getItem()->getSize() * elementCount
	);

	section->m_elementCount = elementCount;
	section->m_ptrTypeFlags = ptrTypeFlags;
	return true;
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onField(
	ModuleItemDecl* decl,
	Type* type,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
) {
	DynamicLayoutSection* section = addSection(DynamicLayoutSectionKind_Field, decl, type, offset, type->getItem()->getSize());
	section->m_ptrTypeFlags = ptrTypeFlags;
	return true;
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onBitField(
	ModuleItemDecl* decl,
	Type* type,
	uint_t bitOffset,
	uint_t bitCount,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
) {
	DynamicLayoutSection* section = addSection(DynamicLayoutSectionKind_Field, decl, type, offset, type->getItem()->getSize());
	section->m_bitOffset = bitOffset;
	section->m_bitCount = bitCount;
	section->m_ptrTypeFlags = ptrTypeFlags;
	return true;
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onGroupOpened(
	ModuleItemDecl* decl,
	size_t offset
) {
	DynamicLayoutSection* group = addSection(DynamicLayoutSectionKind_Group, decl, NULL, offset, 0);
	m_groupStack.append(group);
	return true;
}

bool
JNC_CDECL
DynamicLayoutSectionBuilder::onGroupClosed(
	ModuleItemDecl* decl,
	size_t offset,
	DataPtr ptr,
	size_t size
) {
	ASSERT(!m_groupStack.isEmpty() && m_groupStack.getBack()->m_decl == decl);
	DynamicLayoutSection* group = m_groupStack.getBackAndPop();
	group->m_size = size;
	return true;
}

DynamicLayoutSection*
DynamicLayoutSectionBuilder::addSection(
	DynamicLayoutSectionKind sectionKind,
	ModuleItemDecl* decl,
	Type* type,
	size_t offset,
	size_t size
) {
	DynamicLayoutSection* section = createClass<DynamicLayoutSection>(getCurrentThreadRuntime());
	section->m_sectionKind = sectionKind;
	section->m_offset = offset;
	section->m_size = size;
	section->m_decl = decl;
	section->m_type = type;

	DynamicLayoutSectionGroup* group = !m_groupStack.isEmpty() ? (DynamicLayoutSectionGroup*)m_groupStack.getBack() : m_sectionGroup;
	group->addSection(section);
	return section;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
