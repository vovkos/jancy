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
#include "jnc_DynamicLayoutSection.h"
#include "jnc_StdBuffer.h"
#include "jnc_rtl_Promise.h"

namespace jnc {
namespace rtl {

class ModuleItemDecl;
class Type;
class DynamicLayoutSink;

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayout)

//..............................................................................

typedef
bool
DynamicLayoutSink_OnStructFunc(
	DynamicLayoutSink* sink,
	Type* type,
	size_t offset,
	DataPtr ptr
);

typedef
bool
DynamicLayoutSink_OnArrayFunc(
	DynamicLayoutSink* sink,
	ModuleItemDecl* decl,
	Type* type,
	size_t elementCount,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
);

typedef
bool
DynamicLayoutSink_OnFieldFunc(
	DynamicLayoutSink* sink,
	ModuleItemDecl* decl,
	Type* type,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
);

typedef
bool
DynamicLayoutSink_OnBitFieldFunc(
	DynamicLayoutSink* sink,
	ModuleItemDecl* decl,
	Type* type,
	uint_t bitOffset,
	uint_t bitCount,
	uint_t ptrTypeFlags,
	size_t offset,
	DataPtr ptr
);

typedef
bool
DynamicLayoutSink_OnGroupOpenedFunc(
	DynamicLayoutSink* sink,
	ModuleItemDecl* decl,
	size_t offset
);

typedef
bool
DynamicLayoutSink_OnGroupClosedFunc(
	DynamicLayoutSink* sink,
	ModuleItemDecl* decl,
	size_t offset,
	DataPtr ptr,
	size_t size
);

// mirrors the vtable of jnc.DynamicLayoutSink

struct DynamicLayoutSinkVtable {
	DynamicLayoutSink_OnStructFunc* m_onStructFunc;
	DynamicLayoutSink_OnArrayFunc* m_onArrayFunc;
	DynamicLayoutSink_OnFieldFunc* m_onFieldFunc;
	DynamicLayoutSink_OnBitFieldFunc* m_onBitFieldFunc;
	DynamicLayoutSink_OnGroupOpenedFunc* m_onGroupOpenedFunc;
	DynamicLayoutSink_OnGroupClosedFunc* m_onGroupClosedFunc;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicLayoutSink: public IfaceHdr {
public:
	const DynamicLayoutSinkVtable*
	getVtable() {
		return (const DynamicLayoutSinkVtable*)m_vtable;
	}
};

//..............................................................................

enum DynamicLayoutMode {
	DynamicLayoutMode_Block,
	DynamicLayoutMode_Stream,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DynamicLayout: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(DynamicLayout)

protected:
	enum AwaitKind {
		AwaitKind_None,
		AwaitKind_Size,
		AwaitKind_Char,
	};

	struct Section { // a data section waiting to be reported to the sink
		DynamicLayoutSectionKind m_sectionKind; // DynamicLayoutSectionKind_Undefined if none
		ModuleItemDecl* m_decl;
		Type* m_type;
		size_t m_offset;

		union {
			size_t m_elementCount;
			struct {
				uint_t m_bitOffset;
				uint_t m_bitCount;
			};
		};

		uint_t m_ptrTypeFlags;
	};

	struct Group {
		ModuleItemDecl* m_decl;
		size_t m_offset;
	};

public:
	ClassBox<StdBuffer> m_buffer;
	PromiseImpl* m_auxPromise;
	DynamicLayoutSink* m_sink;
	PromiseImpl* m_promise;
	DataPtr m_ptr;
	size_t m_size;
	size_t m_bufferSize;
	size_t m_sizeLimit;
	uint_t m_mode;

protected:
	Section m_section;
	sl::Array<Group> m_groupStack;
	ct::Type* m_lastBitFieldType;
	size_t m_lastBitOffset;
	size_t m_lastBitCount;

	AwaitKind m_awaitKind;
	size_t m_awaitCharOffset;
	char m_awaitChar;

public:
	DynamicLayout() {
		m_sizeLimit = -1; // the rest is zero-initialized
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	void
	JNC_CDECL
	clear() {
		reset(DynamicLayoutMode_Block, NULL, g_nullDataPtr, 0);
	}

	void
	JNC_CDECL
	reset(
		uint_t mode,
		DynamicLayoutSink* sink,
		DataPtr ptr,
		size_t size
	);

	bool
	JNC_CDECL
	closeAllGroups() {
		return closeGroups(m_groupStack.getCount());
	}

	size_t
	JNC_CDECL
	resume(
		DataPtr ptr,
		size_t size
	);

	Promise*
	JNC_CDECL
	asyncScanTo(char c);

	size_t
	JNC_CDECL
	addStruct(
		ct::StructType* type,
		bool isAsync
	);

	size_t
	JNC_CDECL
	addArray(
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount,
		ct::Type* type,
		size_t elementCount,
		uint_t ptrTypeFlags,
		bool isAsync
	);

	size_t
	JNC_CDECL
	addField(
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount,
		ct::Type* type,
		uint_t ptrTypeFlags,
		bool isAsync
	);

	uint64_t
	JNC_CDECL
	addBitField(
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount,
		ct::Type* type,
		uint_t bitCount,
		uint_t ptrTypeFlags,
		bool isAsync
	);

	size_t
	JNC_CDECL
	openGroup(
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount
	);

	bool
	JNC_CDECL
	closeGroup() {
		return closeGroups(1);
	}

	bool
	JNC_CDECL
	closeGroups(size_t count);

protected:
	bool
	addSize(size_t size);

	void
	prepareForAwait();

	void
	preparePromise();

	ModuleItemDecl*
	getDecl(
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount
	);

	void
	setSection(
		DynamicLayoutSectionKind sectionKind,
		ct::ModuleItemDecl* decl,
		const Variant* dynamicAttrArray,
		size_t dynamicAttrCount,
		ct::Type* type,
		size_t offset,
		uint_t ptrTypeFlags
	);

	bool
	finalizeSection(bool isAsync);

	bool
	reportSection();

	DataPtr
	getPtr(size_t offset);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
