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
#include "jnc_ct_StructType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
StructType::createItemString(size_t index) {
	if (m_stdType != StdType_AbstractData)
		return DerivableType::createItemString(index);

	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return "anydata";

	default:
		return DerivableType::createItemString(index);
	}
}

void
StructType::prepareLlvmType() {
	m_llvmType = llvm::StructType::create(*m_module->getLlvmContext(), getSignature() >> toLlvm);
}

Field*
StructType::createFieldImpl(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::List<Token>* constructor,
	sl::List<Token>* initializer
) {
	Field* field = m_module->m_typeMgr.createField(
		name,
		type,
		bitCount,
		ptrTypeFlags,
		constructor,
		initializer
	);

	field->m_parentNamespace = this;

	if (name.isEmpty()) {
		m_unnamedFieldArray.append(field);
	} else if (name[0] != '!') { // internal field
		bool result = addItem(field);
		if (!result)
			return NULL;
	}

	m_fieldArray.append(field);
	return field;
}

bool
StructType::append(StructType* type) {
	bool result;

	sl::Iterator<BaseTypeSlot> slot = type->m_baseTypeList.getHead();
	for (; slot; slot++) {
		result = addBaseType(slot->m_type) != NULL;
		if (!result)
			return false;
	}

	const sl::Array<Field*>& fieldArray = type->getFieldArray();
	size_t count = fieldArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = fieldArray[i];
		result = createField(field->m_name, field->m_type, field->m_bitCount, field->m_ptrTypeFlags) != NULL;
		if (!result)
			return false;
	}

	return true;
}

bool
StructType::calcLayoutTo(Field* targetField) {
	ASSERT(!(m_flags & TypeFlag_LayoutReady));
	ASSERT(!targetField || !(targetField->m_flags & FieldFlag_LayoutReady));

	bool result;

	if (!m_laidOutFieldCount) {
		result =
			ensureNamespaceReady() &&
			ensureAttributeValuesReady();

		if (!result)
			return false;

		sl::Iterator<BaseTypeSlot> slotIt = m_baseTypeList.getHead();
		for (; slotIt; slotIt++) {
			result = layoutBaseType(*slotIt);
			if (!result)
				return false;
		}
	}

	size_t count = m_fieldArray.getCount();
	for (size_t i = m_laidOutFieldCount; i < count; i++) {
		Field* field = m_fieldArray[i];
		result = layoutField(field);
		if (!result) {
			m_laidOutFieldCount = i; // excluding the failed one
			return false;
		}
	}

	m_laidOutFieldCount = count;

	if (targetField) {
		ASSERT(targetField->m_flags & FieldFlag_LayoutReady); // otherwise, targetField was not part of m_fieldArray
		return true;
	}

	m_size = sl::align(m_fieldSize, m_alignment);

	// scan members for gcroots and constructors (but not for auxilary structs such as class iface)

	if (m_structTypeKind == StructTypeKind_Normal) {
		size_t count = m_fieldArray.getCount();
		for (size_t i = 0; i < count; i++) {
			Field* field = m_fieldArray[i];
			Type* type = field->getType();

			uint_t fieldTypeFlags = type->getFlags();

			if (!(fieldTypeFlags & TypeFlag_Pod))
				m_flags &= ~TypeFlag_Pod;

			if (fieldTypeFlags & TypeFlag_GcRoot) {
				m_gcRootFieldArray.append(field);
				m_flags |= TypeFlag_GcRoot;
			}

			if (field->m_parentNamespace == this && // skip property fields
				(!field->m_initializer.isEmpty() || isConstructibleType(type)))
				m_fieldInitializeArray.append(field);
		}

		scanStaticVariables();
		scanPropertyCtorDtors();

		if (!m_propertyDestructArray.isEmpty()) {
			err::setError("invalid property destructor in 'struct'");
			return false;
		}

		if (!m_staticConstructor &&
			(!m_staticVariableInitializeArray.isEmpty() ||
			!m_propertyStaticConstructArray.isEmpty())) {
			result = createDefaultMethod<DefaultStaticConstructor>() != NULL;
			if (!result)
				return false;
		}

		if (!m_constructor &&
			(m_staticConstructor ||
			!m_baseTypeConstructArray.isEmpty() ||
			!m_fieldInitializeArray.isEmpty() ||
			!m_propertyConstructArray.isEmpty())) {
			result = createDefaultMethod<DefaultConstructor>() != NULL;
			if (!result)
				return false;
		}
	} else if (
		m_structTypeKind == StructTypeKind_IfaceStruct &&
		(((ClassType*)m_parentNamespace)->getFlags() & ClassTypeFlag_Opaque) &&
		!(m_module->getCompileFlags() & ModuleCompileFlag_IgnoreOpaqueClassTypeInfo)
	) {
		ClassType* classType = (ClassType*)m_parentNamespace;

		const OpaqueClassTypeInfo* typeInfo = m_module->m_extensionLibMgr.findOpaqueClassTypeInfo(classType->getItemName());
		if (!typeInfo) {
			err::setFormatStringError("opaque class type info is missing for '%s'", classType->getTypeString().sz());
			return false;
		}

		if (typeInfo->m_size < m_size) {
			err::setFormatStringError(
				"invalid opaque class type size for '%s' (specified %d bytes; must be at least %d bytes)",
				getTypeString().sz(),
				typeInfo->m_size,
				m_size
			);

			return false;
		}

		m_size = typeInfo->m_size;

		if (typeInfo->m_markOpaqueGcRootsFunc)
			classType->m_flags |= TypeFlag_GcRoot;

		if (typeInfo->m_isNonCreatable)
			classType->m_flags |= ClassTypeFlag_OpaqueNonCreatable;

		classType->m_opaqueClassTypeInfo = typeInfo;
	}

	if (m_module->hasCodeGen()) {
		if (m_size > m_fieldSize)
			addLlvmPadding(m_size - m_fieldSize);

		llvm::StructType* llvmStructType = (llvm::StructType*)getLlvmType();
		llvmStructType->setBody(
			llvm::ArrayRef<llvm::Type*>(m_llvmFieldTypeArray, m_llvmFieldTypeArray.getCount()),
			true
		);
	}

	if (m_size > TypeSizeLimit_StackAllocSize)
		m_flags |= TypeFlag_NoStack;

	return true;
}

bool
StructType::layoutBaseType(BaseTypeSlot* slot) {
	bool result = slot->m_type->ensureLayout();
	if (!result)
		return false;

	if (!(slot->m_type->getTypeKindFlags() & TypeKindFlag_Derivable) || slot->m_type->getTypeKind() == TypeKind_Class) {
		err::setFormatStringError("'%s' cannot be a base type of a struct", slot->m_type->getTypeString().sz());
		return false;
	}

	sl::StringHashTableIterator<BaseTypeSlot*> it = m_baseTypeMap.visit(slot->m_type->getSignature());
	if (it->m_value) {
		err::setFormatStringError(
			"'%s' is already a base type",
			slot->m_type->getTypeString().sz()
		);
		return false;
	}

	it->m_value = slot;

	result = slot->m_type->ensureLayout();
	if (!result)
		return false;

	if (slot->m_type->getFlags() & TypeFlag_GcRoot) {
		m_gcRootBaseTypeArray.append(slot);
		m_flags |= TypeFlag_GcRoot;
	}

	if (slot->m_type->getConstructor())
		m_baseTypeConstructArray.append(slot);

	return layoutFieldImpl(
		slot->m_type,
		&slot->m_offset,
		&slot->m_llvmIndex
	);
}

bool
StructType::layoutField(Field* field) {
	ASSERT(!(field->m_flags & FieldFlag_LayoutReady));

	bool result =
		field->ensureAttributeValuesReady() &&
		field->m_type->ensureLayout();

	if (!result)
		return false;

	if (m_structTypeKind != StructTypeKind_IfaceStruct && field->m_type->getTypeKind() == TypeKind_Class) {
		err::setFormatStringError("class '%s' cannot be a struct member", field->m_type->getTypeString().sz());
		field->pushSrcPosError();
		return false;
	}

	result = field->m_bitCount ?
		layoutBitField(field) :
		layoutFieldImpl(
			field->m_type,
			&field->m_offset,
			&field->m_llvmIndex
		);

	if (!result)
		return false;

	field->m_flags |= FieldFlag_LayoutReady;
	return true;
}

bool
StructType::layoutFieldImpl(
	Type* type,
	size_t* offset_o,
	uint_t* llvmIndex
) {
	size_t offset = getFieldOffset(type);
	*offset_o = offset;

	if (m_module->hasCodeGen()) {
		if (offset > m_fieldSize)
			addLlvmPadding(offset - m_fieldSize);

		*llvmIndex = (uint_t) m_llvmFieldTypeArray.getCount();
		m_llvmFieldTypeArray.append(type->getLlvmType());
	}

	m_fieldSize = offset + type->getSize();
	m_lastBitField = NULL;
	return true;
}

bool
StructType::layoutBitField(Field* field) {
	size_t baseBitCount = field->m_type->getSize() * 8;
	if (field->m_bitCount > baseBitCount) {
		err::setError("type of bit field too small for number of bits");
		return false;
	}

	bool isMerged = m_lastBitField && m_lastBitField->getType()->isEqual(field->m_type);

	size_t bitOffset;
	if (field->m_ptrTypeFlags & PtrTypeFlag_BigEndian) {
		if (!isMerged) {
			bitOffset = baseBitCount - field->m_bitCount;
		} else {
			size_t lastBitOffset = m_lastBitField->getBitOffset();
			isMerged = lastBitOffset >= field->m_bitCount;
			bitOffset = isMerged ? lastBitOffset - field->m_bitCount : baseBitCount - field->m_bitCount;
		}
	} else {
		if (!isMerged) {
			bitOffset = 0;
		} else {
			size_t lastBitOffset = m_lastBitField->getBitOffset() + m_lastBitField->getBitCount();
			isMerged = lastBitOffset + field->m_bitCount <= baseBitCount;
			bitOffset = isMerged ? lastBitOffset : 0;
		}
	}

	field->m_bitOffset = bitOffset;
	field->m_ptrTypeFlags |= PtrTypeFlag_BitField;

	if (isMerged) {
		field->m_offset = m_lastBitField->m_offset;
		field->m_llvmIndex = m_lastBitField->m_llvmIndex;
		m_lastBitField = field;
		return true;
	}

	size_t offset = getFieldOffset(field->m_type);
	field->m_offset = offset;

	if (m_module->hasCodeGen()) {
		if (offset > m_fieldSize)
			addLlvmPadding(offset - m_fieldSize);

		field->m_llvmIndex = (uint_t) m_llvmFieldTypeArray.getCount();
		m_llvmFieldTypeArray.append(field->m_type->getLlvmType());
	}

	m_fieldSize = offset + field->m_type->getSize();
	m_lastBitField = field;
	return true;
}

size_t
StructType::getFieldOffset(Type* type) {
	size_t alignment = type->getAlignment();
	if (alignment > m_fieldAlignment)
		alignment = m_fieldAlignment;

	if (alignment > m_alignment)
		m_alignment = alignment;

	return sl::align(m_fieldSize, alignment);
}

void
StructType::addLlvmPadding(size_t size) {
	ASSERT(m_module->hasCodeGen());
	llvm::ArrayType* arrayType = llvm::ArrayType::get(m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getLlvmType(), size);
	m_llvmFieldTypeArray.append(arrayType);
}

void
StructType::prepareLlvmDiType() {
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyStructType(this);
	m_module->m_llvmDiBuilder.setStructTypeBody(this);
}

void
StructType::markGcRoots(
	const void* p0,
	rt::GcHeap* gcHeap
) {
	char* p = (char*)p0;

	size_t count = m_gcRootBaseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		BaseTypeSlot* slot = m_gcRootBaseTypeArray[i];
		slot->getType()->markGcRoots(p + slot->getOffset(), gcHeap);
	}

	count = m_gcRootFieldArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = m_gcRootFieldArray[i];
		field->getType()->markGcRoots(p + field->getOffset(), gcHeap);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
