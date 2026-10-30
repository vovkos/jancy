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
#include "jnc_rtl_DynamicLayout.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_rtl_Type.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ClassType.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	DynamicLayout,
	"jnc.DynamicLayout",
	sl::g_nullGuid,
	-1,
	DynamicLayout,
	&DynamicLayout::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayout)
	JNC_MAP_FUNCTION("init", &jnc::construct<DynamicLayout>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayout>)
	JNC_MAP_FUNCTION("reset", &DynamicLayout::reset)
	JNC_MAP_FUNCTION("closeAllGroups", &DynamicLayout::closeAllGroups)
	JNC_MAP_FUNCTION("resume", &DynamicLayout::resume)
	JNC_MAP_FUNCTION("asyncScanTo", &DynamicLayout::asyncScanTo)
	JNC_MAP_FUNCTION("addStruct", &DynamicLayout::addStruct)
	JNC_MAP_FUNCTION("addArray", &DynamicLayout::addArray)
	JNC_MAP_FUNCTION("addField", &DynamicLayout::addField)
	JNC_MAP_FUNCTION("addBitField", &DynamicLayout::addBitField)
	JNC_MAP_FUNCTION("openGroup", &DynamicLayout::openGroup)
	JNC_MAP_FUNCTION("closeGroup", &DynamicLayout::closeGroup)
	JNC_MAP_FUNCTION("closeGroups", &DynamicLayout::closeGroups)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

class DynamicDecl: public ct::ModuleItemDecl {
public:
	virtual
	ct::ModuleItem*
	getDeclItem() {
		return NULL;
	}
};

//..............................................................................

void
JNC_CDECL
DynamicLayout::markOpaqueGcRoots(GcHeap* gcHeap) {
	gcHeap->markClassPtr(m_section.m_decl);
	gcHeap->markClassPtr(m_section.m_type);

	size_t count = m_groupStack.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_groupStack[i].m_decl);
}

void
JNC_CDECL
DynamicLayout::reset(
	uint_t mode,
	DynamicLayoutSink* sink,
	DataPtr ptr,
	size_t size
) {
	m_buffer->clear();
	m_promise = NULL;
	m_sink = sink;
	m_mode = mode;
	m_ptr = limitDataPtr(ptr, size); // prevent data access beyond the end-of-buffer
	m_size = 0;
	m_bufferSize = size;
	m_section.m_sectionKind = DynamicLayoutSectionKind_Undefined;
	m_section.m_decl = NULL;
	m_section.m_type = NULL;
	m_groupStack.clear();
	m_lastBitFieldType = NULL;
	m_awaitKind = AwaitKind_None;
}

size_t
JNC_CDECL
DynamicLayout::resume(
	DataPtr ptr,
	size_t size
) {
	if (!m_promise || !m_awaitKind) {
		err::setError("dynamic layout is not in a resumable state");
		dynamicThrow();
	}

	char* p0 = (char*)ptr.m_p;
	char* p = p0;
	char* end = p + size;

	while (p < end && m_awaitKind) {
		size_t bufferLeftover = end - p;
		size_t packetLeftover = 0;
		Variant awaitResult = g_nullVariant;

		switch (m_awaitKind) {
		case AwaitKind_Size:
			ASSERT(m_size > m_bufferSize);
			packetLeftover = m_size - m_bufferSize;
			if (bufferLeftover < packetLeftover) { // not yet
				m_bufferSize = m_buffer->append(p, bufferLeftover);
				return size;
			}

			break;

		case AwaitKind_Char: {
			char* p2 = (char*)memchr(p, m_awaitChar, bufferLeftover);
			if (!p2) { // not yet
				m_bufferSize = m_buffer->append(p, bufferLeftover);
				return size;
			}

			packetLeftover = p2 - p; // not including the char!
			size_t distance = m_bufferSize + packetLeftover - m_awaitCharOffset;

			ct::Module* module = m_promise->m_ifaceHdr.m_box->m_type->getModule();
			awaitResult.create(&distance, module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
			break;
			}

		default:
			ASSERT(false);
		}

		m_bufferSize = m_buffer->append(p, packetLeftover);
		m_awaitKind = AwaitKind_None; // this await is satisified
		p += packetLeftover;

		m_ptr = m_buffer->m_ptr;

		DataPtr errorPtr = g_nullDataPtr;
		bool result = reportSection(); // the pending section is ready now
		if (!result) {
			err::Error error = err::getLastError();
			errorPtr = getCurrentThreadGcHeap()->allocateBuffer(error->m_size);
			memcpy(errorPtr.m_p, error, error->m_size);
		}

		m_promise->complete(awaitResult, errorPtr); // this will resume the layout coroutine
	}

	return p - p0;
}

Promise*
JNC_CDECL
DynamicLayout::asyncScanTo(char c) {
	if (m_awaitKind) {
		err::setError("dynamic layout is not in an awaitable state");
		dynamicThrow();
	}

	char* p0 = (char*)m_ptr.m_p;
	char* p = p0 + m_size;
	char* end = p0 + m_bufferSize;
	char* p2 = p < end ? (char*)memchr(p, c, end - p) : NULL;

	if (!p2) {
		if (m_mode == DynamicLayoutMode_Stream) {
			prepareForAwait();
			m_awaitKind = AwaitKind_Char;
			m_awaitCharOffset = m_size;
			m_awaitChar = c;
			return m_promise;
		}

		p2 = end; // if we are not in the stream mode -- report the end of buffer
	}

	preparePromise();
	size_t distance = p2 - p;
	ct::Module* module = m_box->m_type->getModule();
	Variant result;
	result.create(&distance, module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
	m_promise->complete(result, g_nullDataPtr);
	return m_promise;
}

inline
size_t
getArraySize(
	ct::Type* type,
	size_t elementCount
) {
	if (elementCount && type->getSize() > (size_t)-1 / elementCount)
		return err::fail<size_t>(-1, "dynamic array size overflow");

	return type->getSize() * elementCount;
}

inline
bool
DynamicLayout::addSize(size_t size) {
	if (size > m_sizeLimit || m_size > m_sizeLimit - size)
		return err::fail("dynamic layout size overflow");

	m_size += size;
	return true;
}

size_t
JNC_CDECL
DynamicLayout::addStruct(
	ct::StructType* type,
	bool isAsync
) {
	size_t offset = m_size;
	size_t size = type->getSize();
	if (!addSize(size))
		return -1;

	if (m_sink)
		setSection(DynamicLayoutSectionKind_Struct, NULL, NULL, 0, type, offset, 0);

	if (!finalizeSection(isAsync))
		return -1;

	m_lastBitFieldType = NULL;
	return offset;
}

size_t
JNC_CDECL
DynamicLayout::addArray(
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount,
	ct::Type* type,
	size_t elementCount,
	uint_t ptrTypeFlags,
	bool isAsync
) {
	size_t offset = m_size;
	size_t size = getArraySize(type, elementCount);
	if (size == -1 || !addSize(size))
		return -1;

	if (m_sink) {
		setSection(DynamicLayoutSectionKind_Array, decl, dynamicAttrArray, dynamicAttrCount, type, offset, ptrTypeFlags);
		m_section.m_elementCount = elementCount;
	}

	if (!finalizeSection(isAsync))
		return -1;

	m_lastBitFieldType = NULL;
	return offset;
}

size_t
JNC_CDECL
DynamicLayout::addField(
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount,
	ct::Type* type,
	uint_t ptrTypeFlags,
	bool isAsync
) {
	size_t offset = m_size;
	size_t size = type->getSize();
	if (!addSize(size))
		return -1;

	if (m_sink)
		setSection(DynamicLayoutSectionKind_Field, decl, dynamicAttrArray, dynamicAttrCount, type, offset, ptrTypeFlags);

	if (!finalizeSection(isAsync))
		return -1;

	m_lastBitFieldType = NULL;
	return offset;
}

uint64_t
JNC_CDECL
DynamicLayout::addBitField(
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount,
	ct::Type* type,
	uint_t bitCount,
	uint_t ptrTypeFlags,
	bool isAsync
) {
	size_t size = type->getSize();
	size_t baseBitCount = size * 8;
	if (bitCount > baseBitCount)
		return err::fail<uint64_t>(-1, "type of bit field too small for number of bits");

	bool isMerged = m_lastBitFieldType && m_lastBitFieldType->isEqual(type);

	size_t bitOffset;
	if (ptrTypeFlags & PtrTypeFlag_BigEndian) {
		if (!isMerged) {
			bitOffset = baseBitCount - bitCount;
		} else {
			isMerged = m_lastBitOffset >= bitCount;
			bitOffset = isMerged ? m_lastBitOffset - bitCount : baseBitCount - bitCount;
		}
	} else {
		if (!isMerged) {
			bitOffset = 0;
		} else {
			size_t lastBitOffset = m_lastBitOffset + m_lastBitCount;
			isMerged = lastBitOffset + bitCount <= baseBitCount;
			bitOffset = isMerged ? lastBitOffset : 0;
		}
	}

	size_t offset = m_size;
	if (isMerged)
		offset -= size;
	else if (!addSize(size))
		return -1;

	if (m_sink) {
		setSection(
			DynamicLayoutSectionKind_Field,
			decl,
			dynamicAttrArray,
			dynamicAttrCount,
			type,
			offset,
			ptrTypeFlags | PtrTypeFlag_BitField
		);

		m_section.m_bitOffset = bitOffset;
		m_section.m_bitCount = bitCount;
	}

	if (!finalizeSection(isAsync))
		return -1;

	m_lastBitFieldType = type;
	m_lastBitOffset = bitOffset;
	m_lastBitCount = bitCount;
	return (offset << 8) | bitOffset;
}

size_t
JNC_CDECL
DynamicLayout::openGroup(
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount
) {
	Group group;
	group.m_offset = m_size;
	group.m_decl = m_sink ? getDecl(decl, dynamicAttrArray, dynamicAttrCount) : NULL;
	m_groupStack.append(group); // gc-marked from now on

	if (!m_sink)
		return group.m_offset;

	bool retVal = false;
	bool result = callFunctionImpl_s(
		getCurrentThreadRuntime(),
		m_sink->getVtable()->m_onGroupOpenedFunc,
		&retVal,
		m_sink,
		group.m_decl,
		group.m_offset
	);

	if (!result || !retVal) {
		m_groupStack.pop();
		return -1;
	}

	return group.m_offset;
}

bool
JNC_CDECL
DynamicLayout::closeGroups(size_t count) {
	if (count > m_groupStack.getCount())
		count = m_groupStack.getCount();

	if (!m_sink) {
		m_groupStack.pop(count);
		return true;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	size_t end = m_size < m_bufferSize ? m_size : m_bufferSize; // don't report bytes beyond the end-of-buffer

	for (size_t i = 0; i < count; i++) {
		Group group = m_groupStack.getBack(); // keep it on the stack (gc-marked) during the call
		size_t offset = group.m_offset < end ? group.m_offset : end;

		bool retVal = false;
		bool result = callFunctionImpl_s(
			runtime,
			m_sink->getVtable()->m_onGroupClosedFunc,
			&retVal,
			m_sink,
			group.m_decl,
			group.m_offset,
			getPtr(offset),
			end - offset
		);

		m_groupStack.pop();

		if (!result || !retVal)
			return false;
	}

	return true;
}

void
DynamicLayout::prepareForAwait() {
	ASSERT(m_mode == DynamicLayoutMode_Stream);

	if (m_buffer->m_ptr.m_p != m_ptr.m_p) // initial invokation
		m_buffer->copy(m_ptr.m_p, m_bufferSize);

	preparePromise();
}

void
DynamicLayout::preparePromise() {
	if (!m_auxPromise) {
		m_auxPromise = m_promise;
		m_promise = createClass<PromiseImpl>(getCurrentThreadRuntime());
	} else {
		PromiseImpl* promise = m_auxPromise;
		m_auxPromise = m_promise;
		m_promise = promise;
		promise->reset();
	}
}

ModuleItemDecl*
DynamicLayout::getDecl(
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount
) {
	if (!decl)
		return NULL;

	if (!dynamicAttrCount)
		return rtl::getModuleItemDecl(decl);

	Runtime* runtime = getCurrentThreadRuntime();
	ct::Module* module = runtime->getModule();
	NoCollectRegion noCollectRegion(runtime);

	ct::AttributeBlock* attributeBlock = module->m_attributeMgr.createDynamicAttributeBlock(decl);
	DynamicDecl* dynamicDecl = new DynamicDecl;
	dynamicDecl->copyDecl(decl, attributeBlock);

	const sl::Array<ct::Attribute*>& attributeArray = attributeBlock->getAttributeArray();
	size_t totalCount = attributeArray.getCount();
	const Variant* p = dynamicAttrArray;
	const Variant* end = p + dynamicAttrCount;

	for (size_t i = 0; i < totalCount; i++)
		if (attributeArray[i]->getFlags() & ct::AttributeFlag_DynamicValue) {
			ASSERT(p < end);
			attributeBlock->setDynamicAttributeValue(i, *p++);
		}

	ASSERT(p == end);

	// rtl::AttributeBlock & rtl::ModuleItemDecl take ownership

	AttributeBlock* attributeBlock_rtl = (AttributeBlock*)runtime->getGcHeap()->createIntrospectionClass(
		attributeBlock,
		jnc_StdType_AttributeBlock
	);

	ModuleItemDecl* decl_rtl = (ModuleItemDecl*)runtime->getGcHeap()->createIntrospectionClass(
		dynamicDecl,
		jnc_StdType_ModuleItemDecl
	);

	decl_rtl->initializeDynamicDecl(attributeBlock_rtl);
	return decl_rtl;
}

void
DynamicLayout::setSection(
	DynamicLayoutSectionKind sectionKind,
	ct::ModuleItemDecl* decl,
	const Variant* dynamicAttrArray,
	size_t dynamicAttrCount,
	ct::Type* type,
	size_t offset,
	uint_t ptrTypeFlags
) {
	ASSERT(m_sink && m_section.m_sectionKind == DynamicLayoutSectionKind_Undefined);

	m_section.m_sectionKind = sectionKind;
	m_section.m_offset = offset;
	m_section.m_ptrTypeFlags = ptrTypeFlags;
	m_section.m_decl = getDecl(decl, dynamicAttrArray, dynamicAttrCount);
	m_section.m_type = rtl::getType(type);
}

bool
DynamicLayout::finalizeSection(bool isAsync) {
	if (m_size <= m_bufferSize)
		return reportSection();

	if (isAsync && m_mode == DynamicLayoutMode_Stream) {
		prepareForAwait();
		m_awaitKind = AwaitKind_Size; // the section will be reported in resume()
		return true;
	}

	// beyond the end-of-buffer -- never ready

	m_section.m_sectionKind = DynamicLayoutSectionKind_Undefined;
	m_section.m_decl = NULL;
	m_section.m_type = NULL;
	return true;
}

bool
DynamicLayout::reportSection() {
	if (m_section.m_sectionKind == DynamicLayoutSectionKind_Undefined)
		return true;

	ASSERT(m_sink);

	Runtime* runtime = getCurrentThreadRuntime();
	const DynamicLayoutSinkVtable* vtable = m_sink->getVtable();
	DataPtr ptr = getPtr(m_section.m_offset);
	bool retVal = false;
	bool result;

	switch (m_section.m_sectionKind) {
	case DynamicLayoutSectionKind_Struct:
		result = callFunctionImpl_s(
			runtime,
			vtable->m_onStructFunc,
			&retVal,
			m_sink,
			m_section.m_type,
			m_section.m_offset,
			ptr
		);
		break;

	case DynamicLayoutSectionKind_Array:
		result = callFunctionImpl_s(
			runtime,
			vtable->m_onArrayFunc,
			&retVal,
			m_sink,
			m_section.m_decl,
			m_section.m_type,
			m_section.m_elementCount,
			m_section.m_ptrTypeFlags,
			m_section.m_offset,
			ptr
		);
		break;

	case DynamicLayoutSectionKind_Field:
		if (m_section.m_ptrTypeFlags & PtrTypeFlag_BitField)
			result = callFunctionImpl_s(
				runtime,
				vtable->m_onBitFieldFunc,
				&retVal,
				m_sink,
				m_section.m_decl,
				m_section.m_type,
				m_section.m_bitOffset,
				m_section.m_bitCount,
				m_section.m_ptrTypeFlags,
				m_section.m_offset,
				ptr
			);
		else
			result = callFunctionImpl_s(
				runtime,
				vtable->m_onFieldFunc,
				&retVal,
				m_sink,
				m_section.m_decl,
				m_section.m_type,
				m_section.m_ptrTypeFlags,
				m_section.m_offset,
				ptr
			);

		break;

	default:
		ASSERT(false);
		result = true;
		retVal = true;
	}

	m_section.m_sectionKind = DynamicLayoutSectionKind_Undefined;
	m_section.m_decl = NULL;
	m_section.m_type = NULL;
	return result && retVal;
}

DataPtr
DynamicLayout::getPtr(size_t offset) {
	DataPtr ptr = m_ptr;
	ptr.m_p = (char*)ptr.m_p + offset;
	return ptr;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
