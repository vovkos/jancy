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
#include "jnc_StdBuffer.h"
#include "jnc_rtl_Promise.h"

namespace jnc {
namespace rtl {

class ModuleItemDecl;
class DynamicSection;

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicSectionGroup)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicSection)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayout)

//..............................................................................

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

//..............................................................................

enum DynamicSectionKind {
	DynamicSectionKind_Undefined,
	DynamicSectionKind_Struct,
	DynamicSectionKind_Array,
	DynamicSectionKind_Group,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicSection: public DynamicSectionGroup {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicSection)

public:
	uint_t m_sectionKind;
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
	getType_rtl(); // disambiguate vs JNC_DECLARE_CLASS_TYPE_STATIC_METHODS()

	ct::Type*
	getType_ct() {
		return m_type;
	}

	rtl::ModuleItemDecl*
	JNC_CDECL
	getDecl_rtl();

	ct::ModuleItemDecl*
	JNC_CDECL
	getDecl_ct() {
		return m_decl;
	}
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

enum DynamicLayoutMode {
	DynamicLayoutMode_Save   = 0x01,
	DynamicLayoutMode_Stream = 0x02,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicLayout: public DynamicSectionGroup {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayout)

public:
	ClassBox<StdBuffer> m_buffer;
	PromiseImpl* m_promise;
	PromiseImpl* m_auxPromise;

	DataPtr m_ptr;
	size_t m_size;
	size_t m_bufferSize;

	uint_t m_mode;

protected:
	sl::Array<DynamicSectionGroup*> m_groupStack; // groups are already added -- no need to extra mark

public:
	void
	JNC_CDECL
	clear() {
		reset(0, g_nullDataPtr, 0);
	}

	void
	JNC_CDECL
	reset(
		uint_t mode,
		DataPtr ptr,
		size_t size
	);

	size_t
	JNC_CDECL
	resume(
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

	size_t
	JNC_CDECL
	openGroup(ct::ModuleItemDecl* decl);

	void
	JNC_CDECL
	closeGroup() {
		if (!m_groupStack.isEmpty())
			m_groupStack.pop();
	}

protected:
	void
	prepareForAwaitIf() {
		if ((m_mode & DynamicLayoutMode_Stream) && m_size > m_bufferSize)
			prepareForAwait();
	}

	void
	prepareForAwait();

	void
	addSection(DynamicSection* section);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
