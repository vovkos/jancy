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
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_Runtime.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

size_t
dynamicTypeSizeOf(
	DataPtr ptr,
	DerivableType* type
);

//..............................................................................

// can't use JNC_DEFINE_OPAQUE_CLASS_TYPE cause it relies on namespace lookups

JNC_EXTERN_C
jnc_ClassType*
DynamicLayout_getType(jnc_Module* module) {
	return (jnc_ClassType*)module->m_typeMgr.getStdType(StdType_DynamicLayout);
}

JNC_EXTERN_C
const char*
DynamicLayout_getQualifiedName() {
	return "jnc.DynamicLayout";
}

JNC_EXTERN_C
const jnc_OpaqueClassTypeInfo*
DynamicLayout_getOpaqueClassTypeInfo() {
	static jnc_OpaqueClassTypeInfo typeInfo = {
		sizeof(DynamicLayout),  // m_size
		NULL,                   // m_markOpaqueGcRootsFunc
		NULL,                   // m_requireOpaqueItemsFunc
		false,                  // m_isNonCreatable
	};
	return &typeInfo;
}

JNC_BEGIN_TYPE_FUNCTION_MAP(DynamicLayout)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<DynamicLayout>)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

ClassType*
DynamicLayout::getType(Module* module) {
	return (ClassType*)module->m_typeMgr.getStdType(StdType_DynamicLayout);
}

size_t
DynamicLayout::getDynamicFieldSize(
	DataPtr ptr,
	size_t offset,
	Field* field
) {
	size_t size = 0;

	Type* type = field->getType();
	ASSERT(type->getFlags() & TypeFlag_Dynamic);

	if (type->getTypeKindFlags() & TypeKindFlag_Derivable) {
		ptr.m_p = (char*)ptr.m_p + offset;
		size = dynamicTypeSizeOf(ptr, (DerivableType*)type);
	} else if (type->getTypeKind() == TypeKind_Array) {
		Function* getDynamicSizeFunc = ((ArrayType*)type)->getGetDynamicSizeFunction();
		ASSERT(getDynamicSizeFunc);

		typedef
		size_t
		GetDynamicSize(DataPtr ptr);

		GetDynamicSize* getDynamicSize = (GetDynamicSize*)getDynamicSizeFunc->getMachineCode();
		size = getDynamicSize(ptr);
	} else {
		err::setFormatStringError("invalid dynamic type: %s", type->getTypeString().sz());
		dynamicThrow();
	}

	return size;
}

size_t
DynamicLayout::getDynamicFieldEndOffset(
	DataPtr ptr,
	DerivableType* type,
	size_t fieldIndex // dynamic
) {
	Key key = { ptr.m_p, type };

	Entry* entry;

	m_lock.lock();
	sl::MapIterator<Key, Entry*> it = m_map.visit(key);
	if (it->m_value) {
		entry = it->m_value;
	} else {
		entry = AXL_MEM_NEW(Entry);
		m_list.insertTail(entry);
		it->m_value = entry;
	}

	TypeKind typeKind = type->getTypeKind();
	ASSERT(typeKind == TypeKind_Struct);

	sl::Array<Field*> dynamicFieldArray = ((StructType*)type)->getDynamicFieldArray();

	size_t offset;

	size_t count = entry->m_endOffsetArray.getCount();
	if (fieldIndex < count) {
		offset = entry->m_endOffsetArray[fieldIndex];
		m_lock.unlock();

		return offset;
	}

	entry->m_endOffsetArray.reserve(dynamicFieldArray.getCount());
	offset = count ? entry->m_endOffsetArray[count - 1] : 0;

	for (size_t i = count; i <= fieldIndex; i++) {
		m_lock.unlock();

		Field* field = dynamicFieldArray[i];
		offset += field->getOffset();

		size_t size = getDynamicFieldSize(ptr, offset, field);
		offset += size;

		m_lock.lock();

		entry->m_endOffsetArray.ensureCount(i + 1);
		entry->m_endOffsetArray[i] = offset;
	}

	m_lock.unlock();

	return offset;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
