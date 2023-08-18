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
	HidUsagePage() {}
	~HidUsagePage() {}

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

class HidDb: public IfaceHdr {
protected:
	axl::io::HidDb m_db;
	sl::SimpleHashTable<uint_t, HidUsagePage*> m_usagePageMap;

public:
	HidDb() {}

	~HidDb() {
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

//..............................................................................

} // namespace io
} // namespace jnc
