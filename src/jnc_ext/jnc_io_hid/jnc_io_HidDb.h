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

namespace jnc {
namespace io {

class HidDb;

JNC_DECLARE_OPAQUE_CLASS_TYPE(HidUsagePage)
JNC_DECLARE_OPAQUE_CLASS_TYPE(HidDb)

//..............................................................................

class HidUsagePage: public IfaceHdr {
	friend class HidDb;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(HidUsagePage)

public:
    uint_t m_id;

protected:
	const axl::io::HidUsagePage* m_page;
	DataPtr m_namePtr;
	sl::SimpleHashTable<uint_t, DataPtr> m_usageNameMap;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
    DataPtr
	JNC_CDECL
	getName(HidUsagePage* self);

	static
    DataPtr
	JNC_CDECL
	getUsageName(
		HidUsagePage* self,
		uint_t usage
	);

	void
	detach();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
HidUsagePage::detach() {
	m_id = 0;
	m_page = NULL;
	m_namePtr = g_nullDataPtr;
	m_usageNameMap.clear();
}

//..............................................................................

class HidDb: public IfaceHdr {
protected:
	axl::io::HidDb m_db;
	sl::SimpleHashTable<uint_t, HidUsagePage*> m_usagePageMap;

public:
	~HidDb() {
		printf("HidDb::~HidDb\n");
		clear();
	}

	const axl::io::HidDb*
	getDb() const {
		return &m_db;
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

    HidUsagePage*
	JNC_CDECL
	getUsagePage(uint_t page);

    bool
	JNC_CDECL
	load(DataPtr fileNamePtr);

	void
	JNC_CDECL
	clear();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HidDbZipLoader: public axl::io::HidDbLoader {
protected:
	axl::zip::ZipReader m_zipReader;
	sl::StringHashTable<size_t> m_fileNameMap;

public:
	bool
	open(const sl::StringRef& fileName);

	virtual
	bool
	load(
		sl::Array<char>* buffer,
		const sl::StringRef& fileName
	) const;
};

//..............................................................................

} // namespace io
} // namespace jnc