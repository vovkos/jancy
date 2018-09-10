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

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

class Namespace;
class StructField;
class Property;
class Value;

//..............................................................................

class NamedTypeBlock
{
protected:
	ModuleItem* m_parent; // derivable type or property

	sl::Array <Variable*> m_staticFieldArray;
	sl::Array <StructField*> m_memberFieldArray;
	sl::Array <Function*> m_memberMethodArray;
	sl::Array <Property*> m_memberPropertyArray;

	sl::Array <StructField*> m_unnamedFieldArray;
	sl::Array <StructField*> m_gcRootMemberFieldArray;

	sl::Array <Variable*> m_initializedStaticFieldArray;
	sl::Array <StructField*> m_initializedMemberFieldArray;

	sl::Array <StructField*> m_memberFieldConstructArray;
	sl::Array <Property*> m_memberPropertyConstructArray;
	sl::Array <Property*> m_memberPropertyDestructArray;

	Function* m_staticConstructor;
	Function* m_staticDestructor;
	Function* m_preconstructor;
	Function* m_constructor;
	Function* m_destructor;

public:
	NamedTypeBlock (ModuleItem* parent);

	sl::Array <Variable*>
	getStaticFieldArray () const
	{
		return m_staticFieldArray;
	}

	sl::Array <StructField*>
	getMemberFieldArray () const
	{
		return m_memberFieldArray;
	}

	sl::Array <Function*>
	getMemberMethodArray () const
	{
		return m_memberMethodArray;
	}

	sl::Array <Property*>
	getMemberPropertyArray () const
	{
		return m_memberPropertyArray;
	}

	sl::Array <Variable*>
	getInitializedStaticFieldArray () const
	{
		return m_initializedStaticFieldArray;
	}

	sl::Array <StructField*>
	getInitializedMemberFieldArray () const
	{
		return m_initializedMemberFieldArray;
	}

	sl::Array <StructField*>
	getUnnamedFieldArray () const
	{
		return m_unnamedFieldArray;
	}

	sl::Array <StructField*>
	getGcRootMemberFieldArray () const
	{
		return m_gcRootMemberFieldArray;
	}

	Function*
	getPreConstructor () const
	{
		return m_preconstructor;
	}

	Function*
	getConstructor () const
	{
		return m_constructor;
	}

	Function*
	getDestructor () const
	{
		return m_destructor;
	}

	Function*
	getStaticConstructor () const
	{
		return m_staticConstructor;
	}

	Function*
	getStaticDestructor () const
	{
		return m_staticDestructor;
	}

	StructField*
	createField (
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		)
	{
		return createFieldImpl (name, type, bitCount, ptrTypeFlags, constructor, initializer);
	}

	StructField*
	createField (
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0
		)
	{
		return createFieldImpl (sl::String (), type, bitCount, ptrTypeFlags);
	}

	Function*
	createMethod (
		StorageKind storageKind,
		const sl::StringRef& name,
		FunctionType* shortType
		);

	Function*
	createUnnamedMethod (
		StorageKind storageKind,
		FunctionKind functionKind,
		FunctionType* shortType
		);

	Property*
	createProperty (
		StorageKind storageKind,
		const sl::StringRef& name,
		PropertyType* shortType
		);

	virtual
	bool
	addMethod (Function* function) = 0;

	virtual
	bool
	addProperty (Property* prop) = 0;

	bool
	initializeStaticFields ();

	bool
	initializeMemberFields (const Value& thisValue);

	bool
	callMemberFieldConstructors (const Value& thisValue);

	bool
	callMemberPropertyConstructors (const Value& thisValue);

	bool
	callMemberPropertyDestructors (const Value& thisValue);

protected:
	Namespace*
	getParentNamespaceImpl ();

	Unit*
	getParentUnitImpl ();

	virtual
	StructField*
	createFieldImpl (
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		) = 0;
};

//..............................................................................

} // namespace ct
} // namespace jnc
