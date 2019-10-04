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
#include "jnc_ct_Field.h"

namespace jnc {
namespace ct {

class Namespace;
class Property;
class Value;

//..............................................................................

class MemberBlock
{
	friend class VariableMgr;

protected:
	ModuleItem* m_parent; // derivable type or property

	sl::Array<Variable*> m_staticVariableArray;
	sl::Array<Field*> m_fieldArray;
	sl::Array<Field*> m_unnamedFieldArray;
	sl::Array<Function*> m_methodArray;
	sl::Array<Property*> m_propertyArray;

	Variable* m_staticConstructorOnceFlagVariable;
	Function* m_staticConstructor;
	Function* m_constructor;
	Function* m_destructor;

	sl::Array<Field*> m_gcRootFieldArray;
	sl::Array<Variable*> m_staticVariablePrimeArray;
	sl::Array<Variable*> m_staticVariableInitializeArray;
	sl::Array<Field*> m_fieldInitializeArray;
	sl::Array<Property*> m_propertyStaticConstructArray;
	sl::Array<Property*> m_propertyConstructArray;
	sl::Array<Property*> m_propertyDestructArray;

public:
	MemberBlock(ModuleItem* parent);

	const sl::Array<Variable*>&
	getStaticVariableArray() const
	{
		return m_staticVariableArray;
	}

	const sl::Array<Field*>&
	getFieldArray() const
	{
		return m_fieldArray;
	}

	const sl::Array<Field*>&
	getUnnamedFieldArray() const
	{
		return m_unnamedFieldArray;
	}

	const sl::Array<Field*>&
	getGcRootFieldArray() const
	{
		return m_gcRootFieldArray;
	}

	const sl::Array<Function*>&
	getMethodArray() const
	{
		return m_methodArray;
	}

	const sl::Array<Property*>&
	getPropertyArray() const
	{
		return m_propertyArray;
	}

	Function*
	getStaticConstructor() const
	{
		return m_staticConstructor;
	}

	Function*
	getConstructor() const
	{
		return m_constructor;
	}

	Function*
	getDestructor() const
	{
		return m_destructor;
	}

	Field*
	createField(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList<Token>* constructor = NULL,
		sl::BoxList<Token>* initializer = NULL
		)
	{
		return createFieldImpl(name, type, bitCount, ptrTypeFlags, constructor, initializer);
	}

	Field*
	createField(
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0
		)
	{
		return createFieldImpl(sl::String(), type, bitCount, ptrTypeFlags);
	}

	template <typename T = Function>
	T*
	createMethod(
		const sl::StringRef& name,
		FunctionType* shortType
		);

	template <typename T = Function>
	T*
	createUnnamedMethod(
		FunctionKind functionKind,
		FunctionType* shortType
		);

	virtual
	bool
	addMethod(Function* function) = 0;

	virtual
	bool
	addProperty(Property* prop) = 0;

	void
	primeStaticVariables();

	bool
	initializeStaticVariables();

	bool
	initializeFields(const Value& thisValue);

	bool
	callPropertyStaticConstructors();

	bool
	callPropertyConstructors(const Value& thisValue);

	bool
	callPropertyDestructors(const Value& thisValue);

protected:
	Namespace*
	getParentNamespaceImpl();

	Unit*
	getParentUnitImpl();

	template <typename T>
	T*
	createDefaultMethod();

	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList<Token>* constructor = NULL,
		sl::BoxList<Token>* initializer = NULL
		) = 0;

	void
	scanStaticVariables();

	void
	scanPropertyCtorDtors();

	bool
	callStaticConstructor();
};

//..............................................................................

} // namespace ct
} // namespace jnc
