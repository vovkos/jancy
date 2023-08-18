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
#include "jnc_io_HidDb.h"
#include "jnc_io_HidLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidUsagePage,
	"io.HidUsagePage",
	g_hidLibGuid,
	HidLibCacheSlot_HidUsagePage,
	HidUsagePage,
	&HidUsagePage::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidUsagePage)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HidUsagePage>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HidUsagePage>)
	JNC_MAP_CONST_PROPERTY("m_name", &HidUsagePage::getName)
	JNC_MAP_CONST_PROPERTY("m_usageName", &HidUsagePage::getUsageName)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidDb,
	"io.HidDb",
	g_hidLibGuid,
	HidLibCacheSlot_HidDb,
	HidDb,
	&HidDb::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidDb)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HidDb>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HidDb>)
	JNC_MAP_CONST_PROPERTY("m_usagePage", &HidDb::getUsagePage)
	JNC_MAP_FUNCTION("load", &HidDb::load)
	JNC_MAP_FUNCTION("clear", &HidDb::clear)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
HidUsagePage::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	gcHeap->markDataPtr(m_namePtr);

	sl::ConstMapIterator<uint_t, DataPtr> it = m_usageNameMap.getHead();
	for (; it; it++)
		gcHeap->markDataPtr(it->m_value);
}

DataPtr
JNC_CDECL
HidUsagePage::getName(HidUsagePage* self) {
	if (!self->m_page) // detached
		return g_nullDataPtr;

	if (!self->m_namePtr.m_p)
		self->m_namePtr = strDup(self->m_page->getName());

	return self->m_namePtr;
}

DataPtr
JNC_CDECL
HidUsagePage::getUsageName(
	HidUsagePage* self,
	uint_t usage
) {
	if (!self->m_page) // detached
		return g_nullDataPtr;

	sl::MapIterator<uint_t, DataPtr> it = self->m_usageNameMap.visit(usage);
	if (!it->m_value.m_p)
		it->m_value = strDup(self->m_page->getUsageName(usage));

	return it->m_value;
}

void
HidUsagePage::detach() {
	m_id = 0;
	m_page = NULL;
	m_namePtr = g_nullDataPtr;
	m_usageNameMap.clear();
}

//..............................................................................

void
JNC_CDECL
HidDb::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	sl::ConstMapIterator<uint_t, HidUsagePage*> it = m_usagePageMap.getHead();
	for (; it; it++)
		gcHeap->markClassPtr(it->m_value);
}

HidUsagePage*
JNC_CDECL
HidDb::getUsagePage(uint_t pageId) {
	sl::MapIterator<uint_t, HidUsagePage*> it = m_usagePageMap.visit(pageId);
	if (it->m_value)
		return it->m_value;

	Runtime* runtime = getCurrentThreadRuntime();
	HidUsagePage* page = createClass<HidUsagePage>(runtime);
	page->m_page = m_db.getUsagePage(pageId);
	it->m_value = page;
	return page;
}

bool
JNC_CDECL
HidDb::load(DataPtr fileNamePtr) {
	clear();
	return m_db.load((char*)fileNamePtr.m_p);
}

void
JNC_CDECL
HidDb::clear() {
	sl::MapIterator<uint_t, HidUsagePage*> it = m_usagePageMap.getHead();
	for (; it; it++)
		it->m_value->detach();

	m_usagePageMap.clear();
	m_db.clear();
}

//..............................................................................

} // namespace io
} // namespace jnc
