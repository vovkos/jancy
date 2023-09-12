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
#include "jnc_io_HidRd.h"
#include "jnc_io_HidLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidReportField,
	"io.HidReportField",
	g_hidLibGuid,
	HidLibCacheSlot_HidReportField,
	HidReportField,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidReportField)
	JNC_MAP_BINARY_OPERATOR(BinOpKind_Idx, &HidReportField::getItem)
	JNC_MAP_CONST_PROPERTY("m_usageArray", &HidReportField::getUsage)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidReport,
	"io.HidReport",
	g_hidLibGuid,
	HidLibCacheSlot_HidReport,
	HidReport,
	&HidReport::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidReport)
	JNC_MAP_CONST_PROPERTY("m_fieldArray", &HidReport::getField)
	JNC_MAP_FUNCTION("saveDecodeInfo", &HidReport::saveDecodeInfo)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidStandaloneReport,
	"io.HidStandaloneReport",
	g_hidLibGuid,
	HidLibCacheSlot_HidStandaloneReport,
	HidStandaloneReport,
	&HidStandaloneReport::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidStandaloneReport)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HidStandaloneReport>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HidStandaloneReport>)
	JNC_MAP_FUNCTION("loadDecodeInfo", &HidStandaloneReport::loadDecodeInfo)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidRdCollection,
	"io.HidRdCollection",
	g_hidLibGuid,
	HidLibCacheSlot_HidRdCollection,
	HidRdCollection,
	&HidRdCollection::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidRdCollection)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HidRdCollection>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HidRdCollection>)
	JNC_MAP_CONST_PROPERTY("m_collectionArray", &HidRdCollection::getCollection)
	JNC_MAP_CONST_PROPERTY("m_fieldArray", &HidRdCollection::getField)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidRd,
	"io.HidRd",
	g_hidLibGuid,
	HidLibCacheSlot_HidRd,
	HidRd,
	&HidRd::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidRd)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<HidRd>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<HidRd>)
	JNC_MAP_CONST_PROPERTY("m_reportCount", &HidRd::getReportCount)
	JNC_MAP_CONST_PROPERTY("m_reportArray", &HidRd::getReport)
	JNC_MAP_FUNCTION("findReport", &HidRd::findReport)
	JNC_MAP_FUNCTION("clear", &HidRd::clear)
	JNC_MAP_FUNCTION("parse", &HidRd::parse)
	JNC_MAP_FUNCTION("printCollections", &HidRd::printCollections);
	JNC_MAP_FUNCTION("printReports", &HidRd::printReports);
JNC_END_TYPE_FUNCTION_MAP()


//..............................................................................

void
JNC_CDECL
HidReport::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	if (m_rd)
		gcHeap->weakMark(m_rd->m_box);
}

HidReportField*
JNC_CDECL
HidReport::getField(size_t i) {
	if (!m_report || i > m_fieldCount)
		return NULL;

	ASSERT(m_report->getFieldArray().getCount() == m_fieldCount);
	const axl::io::HidReportField* field = m_report->getFieldArray()[i];

	switch (m_fieldStorageKind) {
	case FieldStorageKind_Rd:
		ASSERT(m_rd);
		return m_rd->getField(field);

	case FieldStorageKind_StandaloneReport:
		ASSERT(!m_rd);
		return static_cast<HidStandaloneReport*>(this)->getFieldImpl(i);

	default:
		ASSERT(false);
		return NULL;
	}
}

size_t
JNC_CDECL
HidReport::saveDecodeInfo(std::Buffer* resultBuffer) {
	char stackBuffer[256];
	sl::Array<char> buffer(rc::BufKind_Stack, stackBuffer, sizeof(stackBuffer));
	m_report->saveDecodeInfo(&buffer);
	return resultBuffer->copy_u(buffer.cp(), buffer.getCount());
}

//..............................................................................

void
JNC_CDECL
HidStandaloneReport::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	size_t count = m_fieldArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_fieldArray[i]);

	gcHeap->markClassPtr(m_db);
}

size_t
JNC_CDECL
HidStandaloneReport::loadDecodeInfo(
	HidDb* db,
	DataPtr ptr,
	size_t size
) {
	size_t result = m_standaloneReport.loadDecodeInfo(db->getDb(), ptr.m_p, size);
	if (result == -1)
		return -1;

	ASSERT(result <= size);

	m_fieldStorageKind = FieldStorageKind_StandaloneReport;
	m_rd = NULL;
	m_db = db;
	m_report = &m_standaloneReport;
	m_reportKind = m_standaloneReport.getReportKind();
	m_reportId = m_standaloneReport.getReportId();
	m_bitCount = m_standaloneReport.getBitCount();
	m_size = m_standaloneReport.getSize();
	m_fieldCount = m_standaloneReport.getFieldArray().getCount();
	m_fieldArray.setCountZeroConstruct(m_fieldCount);
	return result;
}

HidReportField*
HidStandaloneReport::getFieldImpl(size_t i) {
	ASSERT(m_fieldArray.getCount() == m_fieldCount);

	if (i >= m_fieldCount)
		return NULL;

	if (m_fieldArray[i])
		return m_fieldArray[i];

	const axl::io::HidReportField* field = m_standaloneReport.getFieldArray()[i];
	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	HidReportField* newField = createClass<HidReportField>(runtime);
	newField->init(this, m_db, field);
	m_fieldArray[i] = newField;
	return newField;
}

//..............................................................................

void
JNC_CDECL
HidRdCollection::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	size_t count = m_collectionArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_collectionArray[i]);

	if (m_rd)
		gcHeap->weakMark(m_rd->m_box);
}

HidRdCollection*
HidRdCollection::getCollection(size_t i) {
	if (!m_collection || i > m_collectionCount)
		return NULL;

	if (i < m_collectionArray.getCount())
		return m_collectionArray[i];

	ASSERT(m_rd && m_collection->getCollectionList().getCount() == m_collectionCount);
	m_collectionArray.setCount(m_collectionCount);
	sl::ConstIterator<axl::io::HidRdCollection> it = m_collection->getCollectionList().getHead();
	for (size_t j = 0; it; it++, j++)
		m_collectionArray[j] = m_rd->getCollection(*it);

	return m_collectionArray[i];
}

HidReportField*
HidRdCollection::getField(size_t i) {
	if (!m_collection || i > m_fieldCount)
		return NULL;

	ASSERT(m_rd && m_collection->getFieldArray().getCount() == m_fieldCount);
	const axl::io::HidReportField* field = m_collection->getFieldArray()[i];
	return m_rd->getField(field);
}

//..............................................................................

HidRd::HidRd() {
	sl::construct(m_rootCollection.p()); // already primed (non-opaque class field)
	m_rootCollection->m_collectionKind = axl::io::HidRdCollectionKind_Invalid;
}

HidRd::~HidRd() {
	clear();
	sl::destruct(m_rootCollection.p());
}

void
JNC_CDECL
HidRd::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	sl::MapIterator<const axl::io::HidReport*, HidReport*> it = m_reportMap.getHead();
	for (; it; it++)
		gcHeap->markClassPtr(it->m_value);

	sl::MapIterator<const axl::io::HidReportField*, HidReportField*> it2 = m_fieldMap.getHead();
	for (; it2; it2++)
		gcHeap->markClassPtr(it2->m_value);

	sl::MapIterator<const axl::io::HidRdCollection*, HidRdCollection*> it3 = m_collectionMap.getHead();
	for (; it3; it3++)
		gcHeap->markClassPtr(it3->m_value);

	gcHeap->markClassPtr(m_db);
}

HidReport*
JNC_CDECL
HidRd::getReport(
	axl::io::HidReportKind reportKind,
    size_t i
) {
	if ((size_t)reportKind >= countof(m_reportTable))
		return NULL;

	sl::Array<HidReport*>& reportArray = m_reportTable[reportKind];
	if (i < reportArray.getCount())
		return reportArray[i];

	const sl::SimpleHashTable<uint_t, axl::io::HidReport>& reportMap = m_rd.getReportMap(reportKind);
	size_t count = reportMap.getCount();
	if (i >= count)
		return NULL;

	Runtime* runtime = getCurrentThreadRuntime();
	reportArray.setCount(count);
	sl::ConstMapIterator<uint_t, axl::io::HidReport> it = reportMap.getHead();
	for (size_t j = 0; it; it++, j++)
		reportArray[j] = getReportImpl(&it->m_value);

	return reportArray[i];
}

void
JNC_CDECL
HidRd::parse(
	HidDb* db,
	DataPtr ptr,
	size_t size
) {
	clear();
	m_rd.parse(db->getDb(), ptr.m_p, size);
	m_rootCollection->init(this, db, &m_rd.getRootCollection());
	m_flags = m_rd.getFlags();
	m_db = db;
}

void
JNC_CDECL
HidRd::clear() {
	// clear is called during shutdown, reports/collections/fields could already be destructed

	for (size_t i = 0; i < countof(m_reportTable); i++) {
		sl::Array<HidReport*>& reportArray = m_reportTable[i];
		size_t count = reportArray.getCount();
		for (size_t j = 0; j < count; j++) {
			HidReport* report = reportArray[j];
			if (!(report->m_box->m_flags & jnc::BoxFlag_Destructed))
				report->detach();
		}

		reportArray.clear();
	}

	sl::MapIterator<const axl::io::HidReport*, HidReport*> it = m_reportMap.getHead();
	for (; it; it++)
		if (!(it->m_value->m_box->m_flags & jnc::BoxFlag_Destructed))
			it->m_value->detach();

	sl::MapIterator<const axl::io::HidReportField*, HidReportField*> it2 = m_fieldMap.getHead();
	for (; it2; it2++)
		if (!(it2->m_value->m_box->m_flags & jnc::BoxFlag_Destructed))
			it2->m_value->detach();

	sl::MapIterator<const axl::io::HidRdCollection*, HidRdCollection*> it3 = m_collectionMap.getHead();
	for (; it3; it3++)
		if (!(it3->m_value->m_box->m_flags & jnc::BoxFlag_Destructed))
			it3->m_value->detach();

	if (!(m_rootCollection->m_box->m_flags & jnc::BoxFlag_Destructed))
		m_rootCollection->detach();

	m_reportMap.clear();
	m_fieldMap.clear();
	m_collectionMap.clear();
	m_rd.clear();
	m_db = NULL;
	m_flags = 0;
}

HidReportField*
HidRd::getField(const axl::io::HidReportField* field) {
	sl::MapIterator<const axl::io::HidReportField*, HidReportField*> it = m_fieldMap.visit(field);
	if (it->m_value)
		return it->m_value;

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	HidReportField* newField = createClass<HidReportField>(runtime);
	newField->init(getReportImpl(field->getReport()), m_db, field);
	it->m_value = newField;
	return newField;
}

HidRdCollection*
HidRd::getCollection(const axl::io::HidRdCollection* collection) {
	sl::MapIterator<const axl::io::HidRdCollection*, HidRdCollection*> it = m_collectionMap.visit(collection);
	if (it->m_value)
		return it->m_value;

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	HidRdCollection* newCollection = createClass<HidRdCollection>(runtime);
	newCollection->init(this, m_db, collection);
	it->m_value = newCollection;
	return newCollection;
}

HidReport*
HidRd::getReportImpl(
	Runtime* runtime,
	const axl::io::HidReport* report
) {
	sl::MapIterator<const axl::io::HidReport*, HidReport*> it = m_reportMap.visit(report);
	if (it->m_value)
		return it->m_value;

	HidReport* newReport = createClass<HidReport>(runtime);
	newReport->init(this, report);
	it->m_value = newReport;
	return newReport;
}

//..............................................................................

} // namespace io
} // namespace jnc
