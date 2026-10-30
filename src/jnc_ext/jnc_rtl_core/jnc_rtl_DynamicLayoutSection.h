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

#include "jnc_rtl_DynamicLayout.h"

namespace jnc {
namespace rtl {

class DynamicLayoutSection;

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayoutSectionGroup)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayoutSection)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayoutSectionBuilder)

//..............................................................................

class DynamicLayoutSectionGroup: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayoutSectionGroup)

public:
	size_t m_sectionCount;

protected:
	sl::Array<DynamicLayoutSection*> m_sectionArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	DynamicLayoutSection*
	JNC_CDECL
	getSection(size_t i) {
		return m_sectionArray[i];
	}

	void
	addSection(DynamicLayoutSection* section) {
		m_sectionArray.append(section);
		m_sectionCount++;
	}
};

//..............................................................................

class DynamicLayoutSection: public DynamicLayoutSectionGroup {
	friend class DynamicLayoutSectionBuilder;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayoutSection)

public:
	uint_t m_sectionKind;
	size_t m_offset;
	size_t m_size;

	union {
		size_t m_elementCount;
		struct {
			uint_t m_bitOffset;
			uint_t m_bitCount;
		};
	};

	uint_t m_ptrTypeFlags;

protected:
	ModuleItemDecl* m_decl;
	Type* m_type;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	ModuleItemDecl*
	JNC_CDECL
	getDecl_rtl() {
		return m_decl;
	}

	Type*
	JNC_CDECL
	getType_rtl() { // disambiguate vs JNC_DECLARE_CLASS_TYPE_STATIC_METHODS()
		return m_type;
	}

	ct::ModuleItemDecl*
	getDecl_ct();

	ct::Type*
	getType_ct();
};

//..............................................................................

class DynamicLayoutSectionBuilder: public DynamicLayoutSink {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayoutSectionBuilder)

public:
	DynamicLayoutSectionGroup* m_sectionGroup;

protected:
	sl::Array<DynamicLayoutSection*> m_groupStack; // groups are in the tree already -- no need to extra mark

public:
	DynamicLayoutSectionBuilder();

	void
	JNC_CDECL
	reset();

	bool
	JNC_CDECL
	onStruct(
		Type* type,
		size_t offset,
		DataPtr ptr
	);

	bool
	JNC_CDECL
	onArray(
		ModuleItemDecl* decl,
		Type* type,
		size_t elementCount,
		uint_t ptrTypeFlags,
		size_t offset,
		DataPtr ptr
	);

	bool
	JNC_CDECL
	onField(
		ModuleItemDecl* decl,
		Type* type,
		uint_t ptrTypeFlags,
		size_t offset,
		DataPtr ptr
	);

	bool
	JNC_CDECL
	onBitField(
		ModuleItemDecl* decl,
		Type* type,
		uint_t bitOffset,
		uint_t bitCount,
		uint_t ptrTypeFlags,
		size_t offset,
		DataPtr ptr
	);

	bool
	JNC_CDECL
	onGroupOpened(
		ModuleItemDecl* decl,
		size_t offset
	);

	bool
	JNC_CDECL
	onGroupClosed(
		ModuleItemDecl* decl,
		size_t offset,
		DataPtr ptr,
		size_t size
	);

protected:
	DynamicLayoutSection*
	addSection(
		DynamicLayoutSectionKind sectionKind,
		ModuleItemDecl* decl,
		Type* type,
		size_t offset,
		size_t size
	);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
