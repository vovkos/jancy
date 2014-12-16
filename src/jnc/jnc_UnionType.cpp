#include "pch.h"
#include "jnc_UnionType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

UnionType::UnionType ()
{
	m_typeKind = TypeKind_Union;
	m_flags = TypeFlag_Pod;
	m_structType = NULL;
	m_initializedField = NULL;
}

StructField*
UnionType::createFieldImpl (
	const rtl::String& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	rtl::BoxList <Token>* constructor,
	rtl::BoxList <Token>* initializer
	)
{
	if (m_flags & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError ("'%s' is completed, cannot add fields to it", getTypeString ().cc ());
		return NULL;
	}

	StructField* field = AXL_MEM_NEW (StructField);
	field->m_name = name;
	field->m_parentNamespace = this;
	field->m_type = type;
	field->m_ptrTypeFlags = ptrTypeFlags;
	field->m_bitFieldBaseType = bitCount ? type : NULL;
	field->m_bitCount = bitCount;

	if (constructor)
		field->m_constructor.takeOver (constructor);

	if (initializer)
		field->m_initializer.takeOver (initializer);

	if (!field->m_constructor.isEmpty () ||
		!field->m_initializer.isEmpty ())
	{
		if (m_initializedField)
		{
			err::setFormatStringError (
				"'%s' already has initialized field '%s'",
				type->getTypeString ().cc (),
				m_initializedField->getName ().cc ()
				);
			return NULL;
		}

		m_initializedField = field;
	}

	m_fieldList.insertTail (field);

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

StructField*
UnionType::getFieldByIndex (size_t index)
{
	size_t count = m_fieldList.getCount ();
	if (index >= count)
	{
		err::setFormatStringError ("index '%d' is out of bounds", index);
		return NULL;
	}

	if (m_fieldArray.getCount () != count)
	{
		m_fieldArray.setCount (count);
		rtl::Iterator <StructField> field = m_fieldList.getHead ();
		for (size_t i = 0; i < count; i++, field++)
			m_fieldArray [i] = *field;
	}

	return m_fieldArray [index];
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

	rtl::Iterator <StructField> fieldIt = m_fieldList.getHead ();
	for (size_t i = 0; fieldIt; fieldIt++, i++)
	{
		StructField* field = *fieldIt;

		result = field->m_type->ensureLayout ();
		if (!result)
			return false;

		uint_t fieldTypeFlags = field->m_type->getFlags ();
		size_t fieldAlignment = field->m_type->getAlignment ();

		if (!(fieldTypeFlags & TypeFlag_Pod))
		{
			err::setFormatStringError ("non-POD '%s' cannot be union member", field->m_type->getTypeString ().cc ());
			return NULL;
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

		field->m_llvmIndex = i;
	}

	ASSERT (largestFieldType);

	m_structType->createField (largestFieldType);
	m_structType->m_alignment = AXL_MIN (largestAlignment, m_structType->m_fieldAlignment);

	result = m_structType->ensureLayout ();
	if (!result)
		return false;

	if (!m_staticConstructor && m_staticDestructor)
	{
		result = createDefaultMethod (FunctionKind_StaticConstructor, StorageKind_Static);
		if (!result)
			return false;
	}

	if (m_staticConstructor)
		m_staticOnceFlagVariable = m_module->m_variableMgr.createOnceFlagVariable ();

	if (m_staticDestructor)
		m_module->m_variableMgr.m_staticDestructList.addStaticDestructor (m_staticDestructor, m_staticOnceFlagVariable);

	if (!m_preConstructor &&
		(m_staticConstructor || m_initializedField))
	{
		result = createDefaultMethod (FunctionKind_PreConstructor);
		if (!result)
			return false;
	}

	if (!m_constructor && m_preConstructor)
	{
		result = createDefaultMethod (FunctionKind_Constructor);
		if (!result)
			return false;
	}

	m_size = m_structType->getSize ();
	m_alignment = m_structType->getAlignment ();
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

	if (m_preConstructor && !(m_preConstructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultPreConstructor ();
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

bool
UnionType::compileDefaultPreConstructor ()
{
	ASSERT (m_preConstructor);

	bool result;

	Value thisValue;
	m_module->m_functionMgr.internalPrologue (m_preConstructor, &thisValue, 1);

	result = initializeField (thisValue);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}


bool
UnionType::initializeField (const Value& thisValue)
{
	ASSERT (m_initializedField);

	Value fieldValue;
	return
		m_module->m_operatorMgr.getField (thisValue, m_initializedField, NULL, &fieldValue) &&
		m_module->m_operatorMgr.parseInitializer (
			fieldValue,
			m_parentUnit,
			m_initializedField->m_constructor,
			m_initializedField->m_initializer
			);
}

void
UnionType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyUnionType (this);
	m_module->m_llvmDiBuilder.setUnionTypeBody (this);
}

//.............................................................................

} // namespace jnc {
