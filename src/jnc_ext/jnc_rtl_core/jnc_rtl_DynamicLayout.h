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

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

class ModuleItemDecl;
class DynamicSection;

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicSectionGroup)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicSection)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayout)

//..............................................................................

enum DynamicSectionKind {
	DynamicSectionKind_Undefined,
	DynamicSectionKind_Struct,
	DynamicSectionKind_Array,
	DynamicSectionKind_Group,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicSectionGroup: public IfaceHdr {
	friend class DynamicLayout;

public:
	size_t m_sectionCount;

protected:
	sl::Array<DynamicSection*> m_sectionArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	DynamicSection*
	JNC_CDECL
	getSection(size_t i) {
		return m_sectionArray[i];
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicSection: public DynamicSectionGroup {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicSection)

public:
	DynamicSectionKind m_sectionKind;
	size_t m_offset;
	size_t m_elementCount;

protected:
	ct::ModuleItemDecl* m_decl;
	ct::Type* m_type;

public:
	DynamicSection(
		DynamicSectionKind sectionKind,
		size_t offset,
		ct::ModuleItemDecl* decl = NULL,
		ct::Type* type = NULL
	);

	IfaceHdr*
	JNC_CDECL
	getType_0(); // disambiguate vs JNC_DECLARE_CLASS_TYPE_STATIC_METHODS()

	rtl::ModuleItemDecl*
	JNC_CDECL
	getDecl();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DynamicSection::DynamicSection(
	DynamicSectionKind sectionKind,
	size_t offset,
	ct::ModuleItemDecl* decl,
	ct::Type* type
) {
	m_sectionKind  = sectionKind;
	m_offset  = offset;
	m_elementCount = 0;
	m_decl = decl;
	m_type = type;
}

//..............................................................................

class DynamicLayout: public DynamicSectionGroup {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayout)

public:
	DataPtr m_basePtr;
	DataPtr m_endPtr;
	DataPtr m_ptr;

protected:
	sl::Array<DynamicSectionGroup*> m_groupStack;

public:
	DynamicLayout() {
	}

	DynamicLayout(
		DataPtr ptr,
		size_t size
	) {
		reset(ptr, size);
	}

	void
	JNC_CDECL
	reset(
		DataPtr ptr,
		size_t size
	);

	size_t
	JNC_CDECL
	addStruct(ct::StructType* type);

	size_t
	JNC_CDECL
	addArray(
		ct::ModuleItemDecl* decl,
		ct::Type* type,
		size_t elementCount
	);

	void
	JNC_CDECL
	openGroup(ct::ModuleItemDecl* decl);

	void
	JNC_CDECL
	closeGroup() {
		if (!m_groupStack.isEmpty())
			m_groupStack.pop();
	}

protected:
	size_t
	getSize() {
		return (char*)m_ptr.m_p - (char*)m_basePtr.m_p;
	}

	void
	addSection(DynamicSection* section);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
