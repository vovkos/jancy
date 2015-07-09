// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"

namespace jnc {

class Namespace;
class StructField;
class Property;
class Variable;
class Value;

//.............................................................................

class NamedTypeBlock
{
protected:
	ModuleItem* m_parent; // derivable type or property
	Namespace* m_namespace; 

	rtl::Array <Variable*> m_staticFieldArray;
	rtl::Array <StructField*> m_memberFieldArray;
	rtl::Array <Function*> m_memberMethodArray;	
	rtl::Array <Property*> m_memberPropertyArray;

	rtl::Array <StructField*> m_importFieldArray;
	rtl::Array <StructField*> m_unnamedFieldArray;
	rtl::Array <StructField*> m_gcRootMemberFieldArray;

	rtl::Array <Variable*> m_initializedStaticFieldArray;
	rtl::Array <StructField*> m_initializedMemberFieldArray;

	rtl::Array <StructField*> m_memberFieldConstructArray;
	rtl::Array <StructField*> m_memberFieldDestructArray;
	rtl::Array <Property*> m_memberPropertyConstructArray;
	rtl::Array <Property*> m_memberPropertyDestructArray;

	Function* m_staticConstructor;
	Function* m_staticDestructor;
	Function* m_preconstructor;
	Function* m_constructor;
	Function* m_destructor;

public:
	NamedTypeBlock (ModuleItem* parent);

	rtl::Array <Variable*>
	getStaticFieldArray ()
	{
		return m_staticFieldArray;
	}

	rtl::Array <StructField*>
	getMemberFieldArray ()
	{
		return m_memberFieldArray;
	}

	rtl::Array <Function*>
	getMemberMethodArray ()
	{
		return m_memberMethodArray;
	}

	rtl::Array <Property*>
	getMemberPropertyArray ()
	{
		return m_memberPropertyArray;
	}

	rtl::Array <Variable*>
	getInitializedStaticFieldArray ()
	{
		return m_initializedStaticFieldArray;
	}

	rtl::Array <StructField*>
	getInitializedMemberFieldArray ()
	{
		return m_initializedMemberFieldArray;
	}

	rtl::Array <StructField*>
	getImportFieldArray ()
	{
		return m_importFieldArray;
	}

	rtl::Array <StructField*>
	getUnnamedFieldArray ()
	{
		return m_unnamedFieldArray;
	}

	rtl::Array <StructField*>
	getGcRootMemberFieldArray ()
	{
		return m_gcRootMemberFieldArray;
	}

	Function*
	getPreconstructor ()
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
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
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
		return createFieldImpl (rtl::String (), type, bitCount, ptrTypeFlags);
	}

	Function*
	createMethod (
		StorageKind storageKind,
		const rtl::String& name,
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
		const rtl::String& name,
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
	virtual
	StructField*
	createFieldImpl (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		) = 0;

	bool
	resolveImportFields ();
};

//.............................................................................

} // namespace jnc {
