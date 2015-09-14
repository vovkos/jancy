#include "pch.h"
#include "jnc_PropertyType.h"
#include "jnc_Decl.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char* 
getPropertyTypeFlagString (PropertyTypeFlag flag)
{
	static const char* stringTable [] = 
	{
		"const",     // EPropertyTypeFlag_Const    = 0x010000,
		"bindable",  // EPropertyTypeFlag_Bindable = 0x020000,
	};

	size_t i = rtl::getLoBitIdx32 (flag >> 16);

	return i < countof (stringTable) ? 
		stringTable [i] : 
		"undefined-property-type-flag";
}

rtl::String
getPropertyTypeFlagString (uint_t flags)
{
	if (!flags)
		return rtl::String ();

	PropertyTypeFlag flag = getFirstPropertyTypeFlag (flags);
	rtl::String string = getPropertyTypeFlagString (flag);
	flags &= ~flag;

	while (flags)
	{
		flag = getFirstPropertyTypeFlag (flags);

		string += ' ';
		string += getPropertyTypeFlagString (flag);

		flags &= ~flag;
	}

	return string;
}

uint_t
getPropertyTypeFlagsFromModifiers (uint_t modifiers)
{
	uint_t flags = 0;

	if (modifiers & TypeModifier_Const)
		flags |= PropertyTypeFlag_Const;

	if (modifiers & TypeModifier_Bindable)
		flags |= PropertyTypeFlag_Bindable;

	return flags;
}

//.............................................................................

PropertyType::PropertyType ()
{
	m_typeKind = TypeKind_Property;

	m_getterType = NULL;
	m_binderType = NULL;
	m_stdObjectMemberPropertyType = NULL;
	m_shortType = NULL;
	m_vtableStructType = NULL;
	m_propertyPtrTypeTuple = NULL;
}

PropertyPtrType* 
PropertyType::getPropertyPtrType (
	Namespace* nspace,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getPropertyPtrType (nspace, this, typeKind, ptrTypeKind, flags);
}

PropertyType*
PropertyType::getMemberPropertyType (ClassType* classType)
{
	return m_module->m_typeMgr.getMemberPropertyType (classType, this);
}

PropertyType*
PropertyType::getStdObjectMemberPropertyType ()
{
	return m_module->m_typeMgr.getStdObjectMemberPropertyType (this);
}

PropertyType*
PropertyType::getShortType  ()
{
	return m_module->m_typeMgr.getShortPropertyType (this);
}

StructType*
PropertyType::getVTableStructType ()
{
	return m_module->m_typeMgr.getPropertyVTableStructType (this);
}

rtl::String
PropertyType::createSignature (
	FunctionType* getterType,
	const FunctionTypeOverload& setterType,
	uint_t flags
	)
{
	rtl::String string = "X";
	
	if (flags & PropertyTypeFlag_Bindable)
		string += 'b';

	string += getterType->getSignature ();

	size_t overloadCount = setterType.getOverloadCount ();
	for (size_t i = 0; i < overloadCount; i++)
	{
		FunctionType* overloadType = setterType.getOverload (i);
		string += overloadType->getSignature ();
	}

	return string;
}

rtl::String
PropertyType::getTypeModifierString ()
{
	if (!m_typeModifierString.isEmpty ())
		return m_typeModifierString;

	if (m_flags & PropertyTypeFlag_Const)
		m_typeModifierString += "const ";

	if (m_flags & PropertyTypeFlag_Bindable)
		m_typeModifierString += "bindable ";

	if (isIndexed ())
		m_typeModifierString += "indexed ";

	return m_typeModifierString;
}

void
PropertyType::prepareTypeString ()
{
	Type* returnType = getReturnType ();

	m_typeString = returnType->getTypeString ();
	m_typeString += ' ';
	m_typeString += getTypeModifierString ();
	m_typeString += "property";
	
	if (isIndexed ())
	{
		m_typeString += ' ';
		m_typeString += m_getterType->getArgString ();
	}
}

//.............................................................................

} // namespace jnc {
