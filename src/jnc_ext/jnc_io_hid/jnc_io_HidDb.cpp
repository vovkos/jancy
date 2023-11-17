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
	JNC_MAP_FUNCTION("getUsageName", &HidUsagePage::getUsageName)
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
	JNC_MAP_FUNCTION("getUsagePage", &HidDb::getUsagePage)
	JNC_MAP_FUNCTION("load", &HidDb::load)
	JNC_MAP_FUNCTION("clear", &HidDb::clear)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
HidUsagePage::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	gcHeap->markString(m_name);

	sl::ConstMapIterator<uint_t, String> it = m_usageNameMap.getHead();
	for (; it; it++)
		gcHeap->markString(it->m_value);
}

String
JNC_CDECL
HidUsagePage::getName(HidUsagePage* self) {
	if (!self->m_page) // detached
		return g_nullString;

	if (!self->m_name.m_ptr.m_p)
		self->m_name = allocateString(self->m_page->getName());

	return self->m_name;
}

String
JNC_CDECL
HidUsagePage::getUsageName(
	HidUsagePage* self,
	uint_t usage
) {
	if (!self->m_page) // detached
		return g_nullString;

	sl::MapIterator<uint_t, String> it = self->m_usageNameMap.visit(usage);
	if (!it->m_value.m_ptr.m_p)
		it->m_value = allocateString(self->m_page->getUsageName(usage));

	return it->m_value;
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
	page->m_id = pageId;
	page->m_page = m_db.getUsagePage(pageId);
	it->m_value = page;
	return page;
}

bool
JNC_CDECL
HidDb::load(String fileName0) {
	clear();

	sl::StringRef fileName = fileName0 >> toAxl;
	if (!fileName.isEmpty())
		return m_db.load(fileName);

	Module* module = getCurrentThreadRuntime()->getModule();
	const char* filePath = module->getExtensionLibFilePath(HidLib_getLib());
	rc::Ptr<HidDbZipLoader> loader = AXL_RC_NEW(HidDbZipLoader);
	return
		loader->open(filePath) &&
		m_db.load(loader);
}

void
JNC_CDECL
HidDb::clear() {
	// clear is called during shutdown; pages could already be destructed

	sl::MapIterator<uint_t, HidUsagePage*> it = m_usagePageMap.getHead();
	for (; it; it++)
		if (!(it->m_value->m_box->m_flags & jnc::BoxFlag_Destructed))
			it->m_value->detach();

	m_usagePageMap.clear();
	m_db.clear();
}

//..............................................................................

bool
HidDbZipLoader::open(const sl::StringRef& fileName) {
	bool result = m_zipReader.openFile(fileName);
	if (!result)
		return false;

	size_t count = m_zipReader.getFileCount();
	for (size_t i = 0; i < count; i++) {
		sl::String fileName = m_zipReader.getFileName(i);
		if (!fileName.isEmpty()) // wat?
			m_fileNameMap[fileName] = i;
	}

	m_fileName = "hid-00-usage-page-dir.ini";
	m_path = fileName;
	m_path.append(':');
	m_path.append(m_fileName);
	return true;
}

bool
HidDbZipLoader::load(
	sl::Array<char>* buffer,
	const sl::StringRef& fileName
) const {
	size_t i = m_fileNameMap.findValue(fileName, -1);
	if (i == -1)
		return err::fail(err::SystemErrorCode_ObjectNameNotFound);

	return m_zipReader.extractFileToMem(i, buffer);
}

//..............................................................................

} // namespace io
} // namespace jnc
