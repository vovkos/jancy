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
#include "jnc_ct_NamespaceSet.h"

namespace jnc {
namespace ct {

class Namespace;
class Property;
class Value;

//..............................................................................

class MemberBlock {
	friend class Parser;
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
	OverloadableFunction m_constructor;
	Function* m_destructor;
	uint_t m_constructorThinThisFlag;

	sl::Array<Field*> m_gcRootFieldArray;
	sl::Array<Variable*> m_staticVariablePrimeArray;
	sl::Array<Variable*> m_staticVariableInitializeArray;
	sl::Array<Field*> m_fieldInitializeArray;
	sl::Array<Property*> m_propertyStaticConstructArray;
	sl::Array<Property*> m_propertyConstructArray;
	sl::Array<Property*> m_propertyDestructArray;

	FriendSet m_friendSet;

public:
	MemberBlock(ModuleItem* parent);

	const sl::Array<Variable*>&
	getStaticVariableArray() const {
		return m_staticVariableArray;
	}

	const sl::Array<Field*>&
	getFieldArray() const {
		return m_fieldArray;
	}

	const sl::Array<Field*>&
	getUnnamedFieldArray() const {
		return m_unnamedFieldArray;
	}

	const sl::Array<Field*>&
	getGcRootFieldArray() const {
		return m_gcRootFieldArray;
	}

	const sl::Array<Function*>&
	getMethodArray() const {
		return m_methodArray;
	}

	const sl::Array<Property*>&
	getPropertyArray() const {
		return m_propertyArray;
	}

	Function*
	getStaticConstructor() const {
		return m_staticConstructor;
	}

	OverloadableFunction
	getConstructor() const {
		return m_constructor;
	}

	uint_t
	getConstructorThinThisFlag() const {
		return m_constructorThinThisFlag;
	}

	Function*
	getDestructor() const {
		return m_destructor;
	}

	const FriendSet&
	getFriendSet() const {
		return m_friendSet;
	}

	Field*
	createField(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::List<Token>* constructor = NULL,
		sl::List<Token>* initializer = NULL
	) {
		return createFieldImpl(name, type, bitCount, ptrTypeFlags, constructor, initializer);
	}

	Field*
	createField(
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0
	) {
		return createFieldImpl(sl::String(), type, bitCount, ptrTypeFlags, NULL, NULL);
	}

	template <typename T>
	T*
	createMethod(
		const sl::StringRef& name,
		FunctionType* shortType
	);

	Function*
	createMethod(
		const sl::StringRef& name,
		FunctionType* shortType
	) {
		return createMethod<Function>(name, shortType);
	}

	template <typename T>
	T*
	createUnnamedMethod(
		FunctionKind functionKind,
		FunctionType* shortType
	);

	Function*
	createUnnamedMethod(
		FunctionKind functionKind,
		FunctionType* shortType
	) {
		return createUnnamedMethod<Function>(functionKind, shortType);
	}

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
	createDefaultMethod(uint_t thisArgTypeFlags = 0);

	virtual
	Field*
	createFieldImpl(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount,
		uint_t ptrTypeFlags,
		sl::List<Token>* constructor,
		sl::List<Token>* initializer
	) = 0;

	void
	scanStaticVariables();

	void
	scanPropertyCtorDtors();

	bool
	callStaticConstructor();

	bool
	addUnnamedMethod(
		Function* function,
		Function** targetFunction,
		OverloadableFunction* targetOverloadableFunction
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
MemberBlock::MemberBlock(ModuleItem* parent) {
	m_parent = parent;
	m_staticConstructorOnceFlagVariable = NULL;
	m_staticConstructor = NULL;
	m_destructor = NULL;
	m_constructorThinThisFlag = PtrTypeFlag_ThinThis;
}

//..............................................................................

} // namespace ct
} // namespace jnc
