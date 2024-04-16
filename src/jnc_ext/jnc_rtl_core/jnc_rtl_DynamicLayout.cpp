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
	JNC_MAP_CONST_PROPERTY("m_decl", &DynamicSection::getDecl_rtl)
	JNC_MAP_CONST_PROPERTY("m_type", &DynamicSection::getType_rtl)
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
	JNC_MAP_FUNCTION("init", &jnc::construct<DynamicLayout>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayout>)
	JNC_MAP_FUNCTION("reset", &DynamicLayout::reset)
	JNC_MAP_FUNCTION("resume", &DynamicLayout::resume)
	JNC_MAP_FUNCTION("addStruct", &DynamicLayout::addStruct)
	JNC_MAP_FUNCTION("addArray", &DynamicLayout::addArray)
	JNC_MAP_FUNCTION("openGroup", &DynamicLayout::openGroup)
	JNC_MAP_FUNCTION("closeGroup", &DynamicLayout::closeGroup)
	JNC_MAP_FUNCTION("closeGroups", &DynamicLayout::closeGroups)
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
DynamicSection::getDecl_rtl() {
	return m_decl ? rtl::getModuleItemDecl(m_decl) : NULL;
}

IfaceHdr*
JNC_CDECL
DynamicSection::getType_rtl() {
	return m_type ? rtl::getType(m_type) : NULL;
}

//..............................................................................

void
JNC_CDECL
DynamicLayout::reset(
	uint_t mode,
	DataPtr ptr,
	size_t size
) {
	m_buffer->clear();
	m_promise = NULL;
	m_mode = mode;
	m_ptr = limitDataPtr(ptr, size); // prevent data access beyond the end-of-buffer
	m_size = 0;
	m_bufferSize = size;
	m_sectionCount = 0;
	m_groupStack.clear();
	m_sectionArray.clear();

}

void
JNC_CDECL
DynamicLayout::updateGroupSizes() {
	size_t count = m_groupStack.getCount();
	for (size_t i = 0; i < count; i++) {
		DynamicSection* group = m_groupStack[i];
		group->m_size = m_size - group->m_offset;
	}
}

size_t
JNC_CDECL
DynamicLayout::resume(
	DataPtr ptr,
	size_t size
) {
	if (!m_promise || m_size <= m_bufferSize) {
		err::setError("dynamic layout is not in a resumable state");
		dynamicThrow();
	}

	char* p = (char*)ptr.m_p;
	char* end = p + size;
	char* p0 = p;

	while (p < end) {
		size_t bufferLeftover = end - p;
		size_t packetLeftover = m_size - m_bufferSize;
		if (bufferLeftover < packetLeftover) { // not yet
			m_bufferSize = m_buffer->append(p, bufferLeftover);
			return size;
		}

		m_bufferSize = m_buffer->append(p, packetLeftover);
		p += packetLeftover;

		m_ptr = m_buffer->m_ptr;
		m_promise->complete(g_nullVariant, g_nullDataPtr); // this will resume the layout coroutine
		if (m_size <= m_bufferSize) // complete
			break;
	}

	return p - p0;
}

size_t
JNC_CDECL
DynamicLayout::addStruct(
	ct::StructType* type,
	bool isAsync
) {
	size_t offset = m_size;
	size_t size = type->getSize();

	if (m_mode & DynamicLayoutMode_Save)
		addSection(DynamicSectionKind_Struct, m_size, size, NULL, type);

	m_size += size;

	if (isAsync && (m_mode & DynamicLayoutMode_Stream) && m_size > m_bufferSize)
		prepareForAwait();

	return offset;
}

size_t
JNC_CDECL
DynamicLayout::addArray(
	ct::ModuleItemDecl* decl,
	ct::Type* type,
	size_t elementCount,
	uint_t ptrTypeFlags,
	bool isAsync
) {
	size_t offset = m_size;
	size_t size = type->getSize() * elementCount;
	if (m_mode & DynamicLayoutMode_Save) {
		DynamicSection* section = addSection(DynamicSectionKind_Array, m_size, size, decl, type);
		section->m_elementCount = elementCount;
		section->m_ptrTypeFlags = ptrTypeFlags;
	}

	m_size += size;

	if (isAsync && (m_mode & DynamicLayoutMode_Stream) && m_size > m_bufferSize)
		prepareForAwait();

	return offset;
}

DynamicSection*
DynamicLayout::addSection(
	DynamicSectionKind sectionKind,
	size_t offset,
	size_t size,
	ct::ModuleItemDecl* decl,
	ct::Type* type
) {
	DynamicSection* section = createClass<DynamicSection>(jnc::getCurrentThreadRuntime());
	section->m_sectionKind = sectionKind;
	section->m_offset = offset;
	section->m_size = size;
	section->m_decl = decl;
	section->m_type = type;

	DynamicSectionGroup* group = !m_groupStack.isEmpty() ? (DynamicSectionGroup*)m_groupStack.getBack() : this;
	group->m_sectionArray.append(section);
	group->m_sectionCount++;
	return section;
}

size_t
JNC_CDECL
DynamicLayout::openGroup(ct::ModuleItemDecl* decl) {
	size_t offset = m_size;
	if (m_mode & DynamicLayoutMode_Save) {
		DynamicSection* group = addSection(DynamicSectionKind_Group, m_size, 0, decl, NULL);
		m_groupStack.append(group);
	}

	return offset;
}

void
JNC_CDECL
DynamicLayout::closeGroup() {
	if (!m_groupStack.isEmpty()) {
		DynamicSection* group = m_groupStack.getBackAndPop();
		group->m_size = m_size - group->m_offset;
	}
}

void
JNC_CDECL
DynamicLayout::closeGroups(size_t count) {
	for (size_t i = 0; i < count && !m_groupStack.isEmpty(); i++) {
		DynamicSection* group = m_groupStack.getBackAndPop();
		group->m_size = m_size - group->m_offset;
	}
}

void
DynamicLayout::prepareForAwait() {
	ASSERT(m_mode & DynamicLayoutMode_Stream);

	if (m_buffer->m_ptr.m_p != m_ptr.m_p) // initial invokation
		m_buffer->copy(m_ptr.m_p, m_bufferSize);

	if (!m_auxPromise) {
		m_auxPromise = m_promise;
		m_promise = createClass<PromiseImpl>(getCurrentThreadRuntime());
	} else {
		PromiseImpl* promise = m_auxPromise;
		m_auxPromise = m_promise;
		m_promise = promise;
		promise->reset();
	}
}

//..............................................................................

} // namespace rtl
} // namespace jnc
