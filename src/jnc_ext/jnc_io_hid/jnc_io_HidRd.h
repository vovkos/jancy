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

#include "jnc_io_HidDb.h"

namespace jnc {
namespace io {

class HidReport;
class HidUsagePage;
class HidRd;
class HidDb;

JNC_DECLARE_OPAQUE_CLASS_TYPE(HidReportField)
JNC_DECLARE_OPAQUE_CLASS_TYPE(HidReport)
JNC_DECLARE_OPAQUE_CLASS_TYPE(HidStandaloneReport)
JNC_DECLARE_OPAQUE_CLASS_TYPE(HidRdCollection)
JNC_DECLARE_OPAQUE_CLASS_TYPE(HidRd)

//..............................................................................

inline
axl::io::HidRdUnit
getHidRdUnit(
	axl::io::HidRdUnitNibbleRole role,
	axl::io::HidRdUnitSystem system
) {
	return axl::io::getHidRdUnit(role, system);
}

inline
DataPtr
getHidRdComplexUnitString(uint32_t unit) {
	return strDup(axl::io::getHidRdComplexUnitString(unit));
}

//..............................................................................

class HidReportField: public IfaceHdr {
	friend class HidRd;
	friend class HidStandaloneReport;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HidReportField)

public:
	HidReport* m_report;
	HidUsagePage* m_usagePage;
	size_t m_bitCount;
	uint_t m_valueFlags;
	uint_t m_mask;
	size_t m_auxUsageCount;

protected:
	const axl::io::HidReportField* m_field;

public:
	uint_t
	JNC_CDECL
	getItem(axl::io::HidRdItemId id) const {
		return m_field ? (*m_field)[id] : 0;
	}

	uint_t
	JNC_CDECL
	getUsage(size_t i) const {
		return m_field ? m_field->getUsage(i) : 0;
	}

protected:
	void
	init(
		HidReport* report,
		HidDb* db,
		const axl::io::HidReportField* field
	);

	void
	detach();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
HidReportField::init(
	HidReport* report,
	HidDb* db,
	const axl::io::HidReportField* field
) {
	const axl::io::HidUsagePage* usagePage = field->getUsagePage();

	m_report = report;
	m_usagePage = usagePage ? db->getUsagePage(usagePage->getId()) : NULL;
	m_field = field;
	m_bitCount = field->getBitCount();
	m_valueFlags = field->getValueFlags();
	m_mask = field->getMask();
	m_auxUsageCount = field->getAuxUsageCount();
}

inline
void
HidReportField::detach() {
	m_report = NULL;
	m_bitCount = 0;
	m_valueFlags = 0;
	m_mask = 0;
}

//..............................................................................

class HidReport: public IfaceHdr {
	friend class HidRd;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HidReport)

public:
	enum FieldStorageKind {
		FieldStorageKind_Rd,
		FieldStorageKind_StandaloneReport,
	};

public:
	axl::io::HidReportKind m_reportKind;
	uint_t m_reportId;
	size_t m_bitCount; // size of all fields in bits
	size_t m_size;     // size of all fields in bytes
	size_t m_fieldCount;

protected:
	FieldStorageKind m_fieldStorageKind;
	HidRd* m_rd;
	const axl::io::HidReport* m_report;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	HidReportField*
	JNC_CDECL
	getField(size_t i);

	size_t
	JNC_CDECL
	saveDecodeInfo(std::Buffer* buffer);

protected:
	void
	init(
		HidRd* rd,
		const axl::io::HidReport* field
	);

	void
	detach();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
HidReport::init(
	HidRd* rd,
	const axl::io::HidReport* report
) {
	m_fieldStorageKind = FieldStorageKind_Rd;
	m_rd = rd;
	m_report = report;
	m_reportKind = report->getReportKind();
	m_reportId = report->getReportId();
	m_bitCount = report->getBitCount();
	m_size = report->getSize();
	m_fieldCount = report->getFieldArray().getCount();
}

inline
void
HidReport::detach() {
	m_rd = NULL;
	m_report = NULL;
	m_reportKind = axl::io::HidReportKind_Invalid;
	m_reportId = 0;
	m_bitCount = 0;
	m_size = 0;
	m_fieldCount = 0;
}

//..............................................................................

class HidStandaloneReport: public HidReport {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HidStandaloneReport)

protected:
	HidDb* m_db;
	axl::io::HidStandaloneReport m_standaloneReport;
	sl::Array<HidReportField*> m_fieldArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	size_t
	JNC_CDECL
	loadDecodeInfo(
		HidDb const* db,
		DataPtr ptr,
		size_t size
	);

	HidReportField*
	getFieldImpl(size_t i);
};

//..............................................................................

class HidRdCollection: public IfaceHdr {
	friend class HidRd;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HidRdCollection)

public:
	axl::io::HidRdCollectionKind m_collectionKind;
	HidUsagePage* m_usagePage;
	uint_t m_usage;
	size_t m_collectionCount;
	size_t m_fieldCount;

protected:
	HidRd* m_rd;
	const axl::io::HidRdCollection* m_collection;
	sl::Array<HidRdCollection*> m_collectionArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	HidRdCollection*
	JNC_CDECL
	getCollection(size_t i);

	HidReportField*
	JNC_CDECL
	getField(size_t i);

protected:
	void
	init(
		HidRd* rd,
		HidDb* db,
		const axl::io::HidRdCollection* collection
	);

	void
	detach();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
HidRdCollection::init(
	HidRd* rd,
	HidDb* db,
	const axl::io::HidRdCollection* collection
) {
	const axl::io::HidUsagePage* usagePage = collection->getUsagePage();

	m_rd = rd;
	m_collection = collection;
	m_collectionKind = collection->getCollectionKind();
	m_usagePage = usagePage ? db->getUsagePage(usagePage->getId()) : NULL;
	m_usage = collection->getUsage();
	m_collectionCount = collection->getCollectionList().getCount();
	m_fieldCount = collection->getFieldArray().getCount();
}

inline
void
HidRdCollection::detach() {
	m_rd = NULL;
	m_collection = NULL;
	m_collectionKind = axl::io::HidRdCollectionKind_Invalid;
	m_usagePage = NULL;
	m_usage = 0;
	m_collectionCount = 0;
	m_fieldCount = 0;
	m_collectionArray.clear();
}

//..............................................................................

class HidRd: public IfaceHdr {
public:
	uint_t m_flags;
	ClassBox<HidRdCollection> m_rootCollection;

protected:
	axl::io::HidRd m_rd;
	sl::Array<HidReport*> m_reportTable[axl::io::HidReportKind__Count];
	sl::SimpleHashTable<const axl::io::HidReport*, HidReport*> m_reportMap;
	sl::SimpleHashTable<const axl::io::HidReportField*, HidReportField*> m_fieldMap;
	sl::SimpleHashTable<const axl::io::HidRdCollection*, HidRdCollection*> m_collectionMap;

protected:
	HidDb* m_db;

public:
	HidRd();
	~HidRd();

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	size_t
	JNC_CDECL
	getReportCount(axl::io::HidReportKind reportKind) {
		return (size_t)reportKind < axl::io::HidReportKind__Count ?
			m_rd.getReportMap(reportKind).getCount() :
			0;
	}

	HidReport*
	JNC_CDECL
	getReport(
		axl::io::HidReportKind reportKind,
	    size_t i
	);

	HidReport*
	JNC_CDECL
	findReport(
		axl::io::HidReportKind reportKind,
		uint_t reportId
	) {
		const axl::io::HidReport* report = m_rd.findReport(reportKind, reportId);
		return report ? getReportImpl(report) : NULL;
	}

	void
	JNC_CDECL
	clear();

	void
	JNC_CDECL
	parse(
		HidDb* db,
		DataPtr ptr,
		size_t size
	);

	void
	JNC_CDECL
	printReports() {
		m_rd.printReports();
	}

	void
	JNC_CDECL
	printCollections() {
		m_rd.printCollections();
	}

public:
	HidReportField*
	getField(const axl::io::HidReportField* field);

	HidRdCollection*
	getCollection(const axl::io::HidRdCollection* collection);

protected:
	HidReport*
	getReportImpl(const axl::io::HidReport* report) {
		return getReportImpl(getCurrentThreadRuntime(), report);
	}

	HidReport*
	getReportImpl(
		Runtime* runtime,
		const axl::io::HidReport* report
	);
};

//..............................................................................

} // namespace io
} // namespace jnc
