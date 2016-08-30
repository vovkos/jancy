#include "pch.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

UnionType::UnionType ()
{
	m_typeKind = TypeKind_Union;
	m_flags = TypeFlag_Pod;
	m_structType = NULL;
}

StructField*
UnionType::createFieldImpl (
	const sl::String& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList <Token>* constructor,
	sl::BoxList <Token>* initializer
	)
{
	if (m_flags & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError ("'%s' is completed, cannot add fields to it", getTypeString ().cc ());
		return NULL;
	}

	StructField* field = m_module->m_typeMgr.createStructField (
		name,
		type,
		bitCount,
		ptrTypeFlags,
		constructor,
		initializer
		);

	field->m_parentNamespace = this;
	field->m_tag = m_tag + "." + name;

	if (!field->m_constructor.isEmpty () ||
		!field->m_initializer.isEmpty ())
	{
		if (m_initializedMemberFieldArray.isEmpty ())
		{
			err::setFormatStringError (
				"'%s' already has initialized field '%s'",
				type->getTypeString ().cc (),
				m_initializedMemberFieldArray [0]->getName ().cc ()
				);
			return NULL;
		}

		m_initializedMemberFieldArray.append (field);
	}

	m_memberFieldArray.append (field);

	if (name.isEmpty ())
	{
		m_unnamedFieldArray.append (field);
	}
	else if (name [0] != '!') // internal field
	{
		bool result = addItem (field);
		if (!result)
			return NULL;
	}

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		field->m_type_i = (ImportType*) type;
		m_importFieldArray.append (field);
	}

	m_memberFieldArray.append (field);
	return field;
}

bool
UnionType::calcLayout ()
{
	bool result;

	result = resolveImportTypes ();
	if (!result)
		return false;

	Type* largestFieldType = NULL;
	size_t largestAlignment = 0;

	size_t count = m_memberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldArray [i];

		result = field->m_type->ensureLayout ();
		if (!result)
			return false;

		uint_t fieldTypeFlags = field->m_type->getFlags ();
		size_t fieldAlignment = field->m_type->getAlignment ();

		if (!(fieldTypeFlags & TypeFlag_Pod))
		{
			err::setFormatStringError ("non-POD '%s' cannot be union member", field->m_type->getTypeString ().cc ());
			return false;
		}

		if (field->m_bitCount)
		{
			field->m_type = m_module->m_typeMgr.getBitFieldType (field->m_bitFieldBaseType, 0, field->m_bitCount);
			if (!field->m_type)
				return false;
		}

		if (!largestFieldType || field->m_type->getSize () > largestFieldType->getSize ())
			largestFieldType = field->m_type;

		if (largestAlignment < field->m_type->getAlignment ())
			largestAlignment = field->m_type->getAlignment ();

		field->m_llvmIndex = i; // llvmIndex is used in unions!
	}

	ASSERT (largestFieldType);

	m_structType->createField (largestFieldType);
	m_structType->m_alignment = AXL_MIN (largestAlignment, m_structType->m_fieldAlignment);

	result = m_structType->ensureLayout ();
	if (!result)
		return false;

	if (!m_staticConstructor && !m_initializedStaticFieldArray.isEmpty ())
	{
		result = createDefaultMethod (FunctionKind_StaticConstructor, StorageKind_Static);
		if (!result)
			return false;
	}

	if (!m_constructor && 
		(m_preconstructor ||
		!m_initializedMemberFieldArray.isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_Constructor);
		if (!result)
			return false;
	}

	m_size = m_structType->getSize ();
	m_alignment = m_structType->getAlignment ();

	if (m_size > TypeSizeLimit_StackAllocSize)
		m_flags |= TypeFlag_NoStack;

	return true;
}

bool
UnionType::compile ()
{
	bool result;

	if (m_staticConstructor && !(m_staticConstructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultStaticConstructor ();
		if (!result)
			return false;
	}

	if (m_constructor && !(m_constructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultConstructor ();
		if (!result)
			return false;
	}

	return true;
}

void
UnionType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyUnionType (this);
	m_module->m_llvmDiBuilder.setUnionTypeBody (this);
}

//.............................................................................

} // namespace ct
} // namespace jnc
