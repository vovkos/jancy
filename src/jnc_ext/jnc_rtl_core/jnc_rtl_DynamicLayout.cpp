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
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_Runtime.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

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
	JNC_MAP_CONSTRUCTOR(&DynamicLayout::construct_0)
	JNC_MAP_OVERLOAD(&DynamicLayout::construct_1)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayout>)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
DynamicSection::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_sectionArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_sectionArray[i]);
}

rtl::ModuleItemDecl*
JNC_CDECL
DynamicSection::getDecl() {
	return m_decl ? rtl::getModuleItemDecl(m_decl) : NULL;
}

//..............................................................................

void
DynamicLayout::construct_1(
	DataPtr ptr,
	size_t size
) {
	m_ptr = m_basePtr = m_endPtr = ptr;
	m_endPtr.m_p = (char*)m_endPtr.m_p + size;
	m_sectionCount = 0;
}

void
JNC_CDECL
DynamicLayout::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_sectionArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_sectionArray[i]);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
