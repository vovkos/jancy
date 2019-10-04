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
#include "jnc_ct_UnionType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

UnionType::UnionType()
{
	m_typeKind = TypeKind_Union;
	m_flags = TypeFlag_Pod;
	m_structType = NULL;
}

Field*
UnionType::createFieldImpl(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList<Token>* constructor,
	sl::BoxList<Token>* initializer
	)
{
	Field* field = m_module->m_typeMgr.createField(
		name,
		type,
		bitCount,
		ptrTypeFlags,
		constructor,
		initializer
		);

	field->m_parentNamespace = this;

	if (name.isEmpty())
	{
		m_unnamedFieldArray.append(field);
	}
	else if (name[0] != '!') // internal field
	{
		bool result = addItem(field);
		if (!result)
			return NULL;
	}

	m_fieldArray.append(field);
	return field;
}

bool
UnionType::calcLayout()
{
	bool result = ensureNamespaceReady();
	if (!result)
		return false;

	Type* largestFieldType = NULL;
	size_t largestAlignment = 0;

	size_t count = m_fieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Field* field = m_fieldArray[i];

		result = field->m_type->ensureLayout();
		if (!result)
			return false;

		uint_t fieldTypeFlags = field->m_type->getFlags();
		size_t fieldAlignment = field->m_type->getAlignment();

		if (!(fieldTypeFlags & TypeFlag_Pod))
		{
			err::setFormatStringError("non-POD '%s' cannot be a union member", field->m_type->getTypeString().sz());
			field->pushSrcPosError();
			return false;
		}
		else if (fieldTypeFlags & TypeFlag_Dynamic)
		{
			err::setFormatStringError("dynamic '%s' cannot be a union member", field->m_type->getTypeString().sz());
			field->pushSrcPosError();
			return false;
		}

		if (field->m_bitCount)
		{
			field->m_type = m_module->m_typeMgr.getBitFieldType(field->m_bitFieldBaseType, 0, field->m_bitCount);
			if (!field->m_type)
				return false;
		}

		if (!largestFieldType || field->m_type->getSize() > largestFieldType->getSize())
			largestFieldType = field->m_type;

		if (largestAlignment < field->m_type->getAlignment())
			largestAlignment = field->m_type->getAlignment();

		field->m_llvmIndex = i; // llvmIndex is used in unions!

		if (field->m_parentNamespace == this && // skip property fields
			(!field->m_initializer.isEmpty() || isConstructibleType(field->m_type)))
			m_fieldInitializeArray.append(field);
	}

	ASSERT(largestFieldType);

	m_structType->createField(largestFieldType);
	m_structType->m_alignment = AXL_MIN(largestAlignment, m_structType->m_fieldAlignment);

	result = m_structType->ensureLayout();
	if (!result)
		return false;

	scanStaticVariables();
	scanPropertyCtorDtors();

	if (!m_propertyDestructArray.isEmpty())
	{
		err::setError("invalid property destructor in 'union'");
		return false;
	}

	if (!m_staticConstructor &&
		(!m_staticVariableInitializeArray.isEmpty() ||
		!m_propertyStaticConstructArray.isEmpty()))
	{
		result = createDefaultMethod<DefaultStaticConstructor>() != NULL;
		if (!result)
			return false;
	}

	if (!m_constructor &&
		(m_staticConstructor ||
		!m_fieldInitializeArray.isEmpty() ||
		!m_propertyConstructArray.isEmpty()))
	{
		result = createDefaultMethod<DefaultConstructor>() != NULL;
		if (!result)
			return false;
	}

	m_size = m_structType->getSize();
	m_alignment = m_structType->getAlignment();

	if (m_size > TypeSizeLimit_StackAllocSize)
		m_flags |= TypeFlag_NoStack;

	return true;
}

void
UnionType::prepareLlvmDiType()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyUnionType(this);
	m_module->m_llvmDiBuilder.setUnionTypeBody(this);
}

//..............................................................................

} // namespace ct
} // namespace jnc
