// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

class Namespace;
class StructField;
class Property;
class Value;

//.............................................................................

class NamedTypeBlock
{
protected:
	ModuleItem* m_parent; // derivable type or property

	sl::Array <Variable*> m_staticFieldArray;
	sl::Array <StructField*> m_memberFieldArray;
	sl::Array <Function*> m_memberMethodArray;	
	sl::Array <Property*> m_memberPropertyArray;

	sl::Array <StructField*> m_importFieldArray;
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
	getStaticFieldArray ()
	{
		return m_staticFieldArray;
	}

	sl::Array <StructField*>
	getMemberFieldArray ()
	{
		return m_memberFieldArray;
	}

	sl::Array <Function*>
	getMemberMethodArray ()
	{
		return m_memberMethodArray;
	}

	sl::Array <Property*>
	getMemberPropertyArray ()
	{
		return m_memberPropertyArray;
	}

	sl::Array <Variable*>
	getInitializedStaticFieldArray ()
	{
		return m_initializedStaticFieldArray;
	}

	sl::Array <StructField*>
	getInitializedMemberFieldArray ()
	{
		return m_initializedMemberFieldArray;
	}

	sl::Array <StructField*>
	getImportFieldArray ()
	{
		return m_importFieldArray;
	}

	sl::Array <StructField*>
	getUnnamedFieldArray ()
	{
		return m_unnamedFieldArray;
	}

	sl::Array <StructField*>
	getGcRootMemberFieldArray ()
	{
		return m_gcRootMemberFieldArray;
	}

	Function*
	getPreConstructor ()
	{
		return m_preconstructor;
	}

	Function*
	getConstructor ()
	{
		return m_constructor;
	}

	Function*
	getDestructor ()
	{
		return m_destructor;
	}

	Function*
	getStaticConstructor ()
	{
		return m_staticConstructor;
	}

	Function*
	getStaticDestructor ()
	{
		return m_staticDestructor;
	}

	StructField*
	createField (
		const sl::String& name,
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
		const sl::String& name,
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
		const sl::String& name,
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
		const sl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		) = 0;

	bool
	resolveImportFields ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
