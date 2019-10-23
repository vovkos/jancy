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
#include "jnc_ct_FunctionOverload.h"
#include "jnc_ct_Property.h"
#include "jnc_ct_PropertyTemplate.h"
#include "jnc_ct_StdFunction.h"
#include "jnc_ClassType.h"

namespace jnc {
namespace ct {

class DerivableType;
class AsyncSequencerFunction;

//..............................................................................

enum GlobalCtorDtorKind
{
	GlobalCtorDtorKind_VariablePrimer,
	GlobalCtorDtorKind_VariableInitializer,
	GlobalCtorDtorKind_Constructor,
	GlobalCtorDtorKind_Destructor,
	GlobalCtorDtorKind__Count,
};

//..............................................................................

class FunctionMgr
{
	friend class Module;
	friend class DerivableType;
	friend class ClassType;
	friend class Function;
	friend class Parser;
	friend class AsyncLauncherFunction;
	friend class AsyncSequencerFunction;

protected:
	Module* m_module;

	sl::List<Function> m_functionList;
	sl::List<FunctionOverload> m_functionOverloadList;
	sl::List<Property> m_propertyList;
	sl::List<PropertyTemplate> m_propertyTemplateList;

	sl::StringHashTable<Function*> m_thunkFunctionMap;
	sl::StringHashTable<Property*> m_thunkPropertyMap;
	sl::StringHashTable<Function*> m_schedLauncherFunctionMap;

	sl::Array<AsyncSequencerFunction*> m_asyncSequencerFunctionArray;
	sl::Array<Function*> m_globalCtorDtorArrayTable[GlobalCtorDtorKind__Count];

	Function* m_stdFunctionArray[StdFunc__Count];
	Property* m_stdPropertyArray[StdProp__Count];

	Function* m_currentFunction;
	Value m_thisValue;
	Value m_promiseValue;

public:
	FunctionMgr();

	Module*
	getModule()
	{
		return m_module;
	}

	Function*
	getCurrentFunction()
	{
		return m_currentFunction;
	}

	Function*
	setCurrentFunction(Function* function);

	Property*
	getCurrentProperty()
	{
		return m_currentFunction ? m_currentFunction->getProperty() : NULL;
	}

	Value
	getThisValue()
	{
		return m_thisValue;
	}

	Value
	getPromiseValue()
	{
		return m_promiseValue;
	}

	Value
	overrideThisValue(const Value& value);

	void
	clear();

	const sl::List<Function>&
	getFunctionList()
	{
		return m_functionList;
	}

	const sl::List<Property>&
	getPropertyList()
	{
		return m_propertyList;
	}

	const sl::Array<Function*>&
	getGlobalCtorDtorArray(GlobalCtorDtorKind kind)
	{
		ASSERT((size_t)kind < countof(m_globalCtorDtorArrayTable));
		return m_globalCtorDtorArrayTable[kind];
	}

	bool
	addGlobalCtorDtor(
		GlobalCtorDtorKind kind,
		Function* function
		);

	template <typename T>
	T*
	createFunction(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		FunctionType* type
		);

	Function*
	createFunction(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		FunctionType* type
		)
	{
		return createFunction<Function>(name, qualifiedName, type);
	}

	template <typename T>
	T*
	createFunction(
		FunctionKind functionKind,
		FunctionType* type
		);

	Function*
	createFunction(
		FunctionKind functionKind,
		FunctionType* type
		)
	{
		return createFunction<Function>(functionKind, type);
	}

	template <typename T>
	T*
	createFunction(FunctionType* type)
	{
		return createFunction<T>(sl::String(), sl::String(), type);
	}

	Function*
	createFunction(FunctionType* type)
	{
		return createFunction<Function>(type);
	}

	template <typename T>
	T*
	createInternalFunction(
		const sl::StringRef& tag,
		FunctionType* type
		);

	Function*
	createInternalFunction(
		const sl::StringRef& tag,
		FunctionType* type
		)
	{
		return createInternalFunction<Function>(tag, type);
	}

	FunctionOverload*
	createFunctionOverload(Function* function);

	template <typename T>
	T*
	createProperty(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName
		);

	Property*
	createProperty(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName
		)
	{
		return createProperty<Property>(name, qualifiedName);
	}

	template <typename T>
	T*
	createInternalProperty(const sl::StringRef& tag);

	Property*
	createInternalProperty(const sl::StringRef& tag)
	{
		return createInternalProperty<Property>(tag);
	}

	PropertyTemplate*
	createPropertyTemplate();

	void
	prologue(
		Function* function,
		const lex::LineCol& pos
		);

	bool
	epilogue();

	bool
	fireOnChanged();

	void
	internalPrologue(
		Function* function,
		Value* argValueArray = NULL,
		size_t argCount = 0,
		const lex::LineCol* pos = NULL
		);

	void
	internalEpilogue();

	void
	injectTlsPrologues();

	void
	replaceAsyncAllocas();

	bool
	jitFunctions();

	// std functions & properties

	bool
	isStdFunctionUsed(StdFunc func)
	{
		ASSERT(func < StdFunc__Count);
		return m_stdFunctionArray[func] != NULL;
	}

	Function*
	getStdFunction(StdFunc func);

	bool
	isStdPropertyUsed(StdProp prop)
	{
		ASSERT(prop < StdProp__Count);
		return m_stdPropertyArray[prop] != NULL;
	}

	Property*
	getStdProperty(StdProp prop);

	Function*
	getDirectThunkFunction(
		Function* targetFunction,
		FunctionType* thunkFunctionType,
		bool hasUnusedClosure = false
		);

	Property*
	getDirectThunkProperty(
		Property* targetProperty,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure = false
		);

	Property*
	getDirectDataThunkProperty(
		Variable* targetVariable,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure = false
		);

	Function*
	getSchedLauncherFunction(FunctionPtrType* targetFunctionPtrType);

	void
	createThisValue();

	bool
	finalizeNamespaceProperties(const sl::ConstIterator<Property>& prevIt);

protected:
	void
	injectTlsPrologue(Function* function);

	void
	finalizeFunction(
		Function* function,
		bool wasNamespaceOpened
		);

	Function*
	parseStdFunction(StdFunc func);

	Function*
	parseStdFunction(
		StdNamespace stdNamespace,
		const sl::StringRef& source
		);

	void
	addFunction(
		Function* function,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		FunctionType* type
		);

	void
	addProperty(
		Property* prop,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
T*
FunctionMgr::createFunction(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	FunctionType* type
	)
{
	T* function = AXL_MEM_NEW(T);
	addFunction(function, name, qualifiedName, type);
	return function;
}

template <typename T>
T*
FunctionMgr::createFunction(
	FunctionKind functionKind,
	FunctionType* type
	)
{
	T* function = createFunction<T>(sl::String(), sl::String(), type);
	function->m_functionKind = functionKind;
	return function;
}

template <typename T>
T*
FunctionMgr::createInternalFunction(
	const sl::StringRef& tag,
	FunctionType* type
	)
{
	T* function = createFunction<T>(sl::StringRef(), tag, type);
	function->m_functionKind = FunctionKind_Internal;
	return function;
}

template <typename T>
T*
FunctionMgr::createProperty(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName
	)
{
	T* prop = AXL_MEM_NEW(T);
	addProperty(prop, name, qualifiedName);
	return prop;
}

template <typename T>
T*
FunctionMgr::createInternalProperty(const sl::StringRef& tag)
{
	T* prop = createProperty<T>(sl::StringRef(), tag);
	prop->m_propertyKind = PropertyKind_Internal;
	return prop;
}

//..............................................................................

} // namespace ct
} // namespace jnc
