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
#include "jnc_ct_PropertyType.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getPropertyTypeFlagString (PropertyTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"const",     // EPropertyTypeFlag_Const    = 0x010000,
		"bindable",  // EPropertyTypeFlag_Bindable = 0x020000,
	};

	size_t i = sl::getLoBitIdx32 (flag >> 16);

	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-property-type-flag";
}

sl::String
getPropertyTypeFlagString (uint_t flags)
{
	if (!flags)
		return sl::String ();

	PropertyTypeFlag flag = getFirstPropertyTypeFlag (flags);
	sl::String string = getPropertyTypeFlagString (flag);
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

//..............................................................................

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

sl::String
PropertyType::createSignature (
	FunctionType* getterType,
	const FunctionTypeOverload& setterType,
	uint_t flags
	)
{
	sl::String string = "X";

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

sl::String
PropertyType::getTypeModifierString ()
{
	sl::String string;

	if (m_flags & PropertyTypeFlag_Const)
		string += "const ";

	if (m_flags & PropertyTypeFlag_Bindable)
		string += "bindable ";

	// to make output cleaner: don't append 'indexed' to simple member properties
	// -- even though they ARE indexed

	size_t argCount = m_getterType->getArgArray ().getCount ();
	if (argCount > 2 || argCount == 1 && !m_getterType->isMemberMethodType ())
		string += "indexed ";

	if (!string.isEmpty ())
		string.reduceLength (1);

	return string;
}

void
PropertyType::prepareTypeString ()
{
	TypeStringTuple* tuple = getTypeStringTuple ();
	Type* returnType = getReturnType ();

	tuple->m_typeStringPrefix = returnType->getTypeStringPrefix ();

	sl::String modifierString = getTypeModifierString ();
	if (!modifierString.isEmpty ())
	{
		tuple->m_typeStringPrefix += ' ';
		tuple->m_typeStringPrefix += modifierString;
	}

	tuple->m_typeStringPrefix += " property";

	if (isIndexed ())
		tuple->m_typeStringSuffix = m_getterType->getTypeStringSuffix ();

	tuple->m_typeStringSuffix += returnType->getTypeStringSuffix ();
}

void
PropertyType::prepareDoxyLinkedText ()
{
	TypeStringTuple* tuple = getTypeStringTuple ();
	Type* returnType = getReturnType ();

	tuple->m_doxyLinkedTextPrefix = returnType->getDoxyLinkedTextPrefix ();
	tuple->m_doxyLinkedTextPrefix += ' ';
	tuple->m_doxyLinkedTextPrefix += getTypeModifierString ();
	tuple->m_doxyLinkedTextPrefix += "property";

	if (isIndexed ())
		tuple->m_doxyLinkedTextSuffix = m_getterType->getDoxyLinkedTextSuffix ();

	tuple->m_doxyLinkedTextSuffix += returnType->getDoxyLinkedTextSuffix ();
}

void
PropertyType::prepareDoxyTypeString ()
{
	Type::prepareDoxyTypeString ();

	if (isIndexed ())
		getTypeStringTuple ()->m_doxyTypeString += m_getterType->getDoxyArgString ();
}

//..............................................................................

} // namespace ct
} // namespace jnc
