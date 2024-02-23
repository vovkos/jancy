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

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicSection)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayout)

//..............................................................................

enum DynamicSectionKind {
	DynamicSectionKind_Group,
	DynamicSectionKind_Array,
	DynamicSectionKind_Struct,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicSection: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicSection)

public:
	size_t m_offset;
	size_t m_size;
	DynamicSectionKind m_sectoinKind;
	Type* m_type;

	union {
		size_t m_elementCount;
		size_t m_sectionCount;
	};

protected:
	ct::ModuleItemDecl* m_decl;
	sl::Array<DynamicSection*> m_sectionArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	rtl::ModuleItemDecl*
	JNC_CDECL
	getDecl();

	DynamicSection*
	JNC_CDECL
	getSection(size_t i) {
		return m_sectionArray[i];
	}
};

//..............................................................................

class DynamicLayout: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayout)

public:
	DataPtr m_ptr;
	DataPtr m_basePtr;
	DataPtr m_endPtr;
	size_t m_sectionCount;

protected:
	sl::Array<DynamicSection*> m_sectionArray;

public:
	void
	JNC_CDECL
	construct_0() {}

	void
	JNC_CDECL
	construct_1(
		DataPtr ptr,
		size_t size
	);

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
