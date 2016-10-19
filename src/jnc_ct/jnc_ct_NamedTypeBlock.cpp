#include "pch.h"
#include "jnc_ct_NamedTypeBlock.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

NamedTypeBlock::NamedTypeBlock (ModuleItem* parent)
{
	m_parent = parent;
	m_staticConstructor = NULL;
	m_staticDestructor = NULL;
	m_preconstructor = NULL;
	m_constructor = NULL;
	m_destructor = NULL;
}

Namespace*
NamedTypeBlock::getParentNamespaceImpl ()
{
	ASSERT (
		m_parent->getItemKind () == ModuleItemKind_Property ||
		m_parent->getItemKind () == ModuleItemKind_Type &&
		(((Type*) m_parent)->getTypeKindFlags () & TypeKindFlag_Derivable));

	return  m_parent->getItemKind () == ModuleItemKind_Property ?
		(Namespace*) (Property*) m_parent :
		(Namespace*) (DerivableType*) m_parent;
}

Unit*
NamedTypeBlock::getParentUnitImpl ()
{
	ASSERT (
		m_parent->getItemKind () == ModuleItemKind_Property ||
		m_parent->getItemKind () == ModuleItemKind_Type &&
		(((Type*) m_parent)->getTypeKindFlags () & TypeKindFlag_Derivable));

	return  m_parent->getItemKind () == ModuleItemKind_Property ?
		((Property*) m_parent)->getParentUnit () :
		((DerivableType*) m_parent)->getParentUnit ();
}

Function*
NamedTypeBlock::createMethod (
	StorageKind storageKind,
	const sl::StringRef& name,
	FunctionType* shortType
	)
{
	sl::String qualifiedName = getParentNamespaceImpl ()->createQualifiedName (name);

	Function* function = m_parent->getModule ()->m_functionMgr.createFunction (FunctionKind_Named, shortType);
	function->m_storageKind = storageKind;
	function->m_name = name;
	function->m_qualifiedName = qualifiedName;
	function->m_declaratorName = name;
	function->m_tag = qualifiedName;

	bool result = addMethod (function);
	if (!result)
		return NULL;

	return function;
}

Function*
NamedTypeBlock::createUnnamedMethod (
	StorageKind storageKind,
	FunctionKind functionKind,
	FunctionType* shortType
	)
{
	Function* function = m_parent->getModule ()->m_functionMgr.createFunction (functionKind, shortType);
	function->m_storageKind = storageKind;
	function->m_tag.format (
		"%s.%s",
		getParentNamespaceImpl ()->getQualifiedName ().sz (),
		getFunctionKindString (functionKind)
		);

	bool result = addMethod (function);
	if (!result)
		return NULL;

	return function;
}

Property*
NamedTypeBlock::createProperty (
	StorageKind storageKind,
	const sl::StringRef& name,
	PropertyType* shortType
	)
{
	sl::String qualifiedName = getParentNamespaceImpl ()->createQualifiedName (name);

	Property* prop = m_parent->getModule ()->m_functionMgr.createProperty (name, qualifiedName);

	bool result =
		addProperty (prop) &&
		prop->create (shortType);

	if (!result)
		return NULL;

	return prop;
}

bool
NamedTypeBlock::initializeStaticFields ()
{
	bool result;

	Module* module = m_parent->getModule ();

	size_t count = m_initializedStaticFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* staticField = m_initializedStaticFieldArray [i];
		result = module->m_variableMgr.initializeVariable (staticField);
		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::initializeMemberFields (const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule ();
	Unit* parentUnit = getParentUnitImpl ();

	size_t count = m_initializedMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_initializedMemberFieldArray [i];

		Value fieldValue;
		result = module->m_operatorMgr.getField (thisValue, field, NULL, &fieldValue);
		if (!result)
			return false;

		result = module->m_operatorMgr.parseInitializer (
			fieldValue,
			parentUnit,
			field->m_constructor,
			field->m_initializer
			);

		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::callMemberFieldConstructors (const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule ();

	size_t count = m_memberFieldConstructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldConstructArray [i];
		if (field->m_flags & ModuleItemFlag_Constructed)
		{
			field->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Value fieldValue;
		result = module->m_operatorMgr.getClassField (thisValue, field, NULL, &fieldValue);
		if (!result)
			return false;

		ASSERT (field->getType ()->getTypeKindFlags () & TypeKindFlag_Derivable);
		DerivableType* type = (DerivableType*) field->getType ();

		Function* constructor;

		sl::BoxList <Value> argList;
		argList.insertTail (fieldValue);

		if (!(type->getFlags () & TypeFlag_Child))
		{
			constructor = type->getDefaultConstructor ();
			if (!constructor)
				return false;
		}
		else
		{
			constructor = type->getConstructor ();
			ASSERT (constructor && !constructor->isOverloaded ());

			argList.insertTail (thisValue);
		}

		result = module->m_operatorMgr.callOperator (constructor, &argList);
		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::callMemberPropertyConstructors (const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule ();

	size_t count = m_memberPropertyConstructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_memberPropertyConstructArray [i];
		if (prop->m_flags & ModuleItemFlag_Constructed)
		{
			prop->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Function* constructor = prop->getConstructor ();
		ASSERT (constructor);

		result = module->m_operatorMgr.callOperator (constructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::callMemberPropertyDestructors (const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule ();

	size_t count = m_memberPropertyDestructArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		Property* prop = m_memberPropertyDestructArray [i];

		Function* destructor = prop->getDestructor ();
		ASSERT (destructor);

		result = module->m_operatorMgr.callOperator (destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
