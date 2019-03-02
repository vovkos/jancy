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
#include "jnc_ct_Property.h"
#include "jnc_ct_PropertyTemplate.h"
#include "jnc_ct_StdFunction.h"
#include "jnc_ClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

class FunctionMgr
{
	friend class Module;
	friend class DerivableType;
	friend class ClassType;
	friend class Function;
	friend class Parser;
	friend class AsyncFunction;

protected:
	Module* m_module;

	sl::List<Function> m_functionList;
	sl::List<Property> m_propertyList;
	sl::List<PropertyTemplate> m_propertyTemplateList;
	sl::List<LazyStdFunction> m_lazyStdFunctionList;

	sl::StringHashTable<Function*> m_thunkFunctionMap;
	sl::StringHashTable<Property*> m_thunkPropertyMap;
	sl::StringHashTable<Function*> m_scheduleLauncherFunctionMap;

	sl::Array<AsyncFunction*> m_asyncFunctionArray;
	sl::Array<NamedTypeBlock*> m_staticConstructArray;

	Function* m_stdFunctionArray[StdFunc__Count];
	LazyStdFunction* m_lazyStdFunctionArray[StdFunc__Count];
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

	sl::ConstList<Function>
	getFunctionList()
	{
		return m_functionList;
	}

	sl::ConstList<Property>
	getPropertyList()
	{
		return m_propertyList;
	}

	void
	addStaticConstructor(NamedTypeBlock* namedTypeBlock)
	{
		m_staticConstructArray.append(namedTypeBlock);
	}

	void
	callStaticConstructors();

	Function*
	createFunction(
		FunctionKind functionKind,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		FunctionType* type
		);

/*	Function*
	createFunction(
		FunctionKind functionKind,
		FunctionType* type
		)
	{
		return createFunction(functionKind, sl::String(), sl::String(), sl::String(), type);
	} */

	Function*
	createFunction(
		FunctionKind functionKind,
		const sl::StringRef& name,
		FunctionType* type
		)
	{
		return createFunction(functionKind, name, name, type);
	}

	Function*
	createFunction(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		FunctionType* type
		)
	{
		return createFunction(FunctionKind_Normal, name, qualifiedName, type);
	}

	Property*
	createProperty(
		PropertyKind propertyKind,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName
		);

/*	Property*
	createProperty(PropertyKind propertyKind)
	{
		return createProperty(propertyKind, sl::String(), sl::String(), sl::String());
	} */

	Property*
	createProperty(
		PropertyKind propertyKind,
		const sl::StringRef& name
		)
	{
		return createProperty(propertyKind, name, name);
	}

	Property*
	createProperty(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName
		)
	{
		return createProperty(PropertyKind_Normal, name, qualifiedName);
	}

	PropertyTemplate*
	createPropertyTemplate();

	void
	prologue(
		Function* function,
		const Token::Pos& pos
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
		const Token::Pos* pos = NULL
		);

	void
	internalEpilogue();

	void
	injectTlsPrologues();

	void
	replaceAsyncAllocas();

	bool
	jitFunctions();

	// std functions

	bool
	isStdFunctionUsed(StdFunc func)
	{
		ASSERT(func < StdFunc__Count);
		return m_stdFunctionArray[func] != NULL;
	}

	Function*
	getStdFunction(StdFunc func);

	LazyStdFunction*
	getLazyStdFunction(StdFunc func);

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
	getScheduleLauncherFunction(
		FunctionPtrType* targetFunctionPtrType,
		ClassPtrTypeKind schedulerPtrTypeKind = ClassPtrTypeKind_Normal
		);

	void
	createThisValue();

protected:
	void
	injectTlsPrologue(Function* function);

	void
	finalizeFunction(
		Function* function,
		bool wasNamespaceOpened
		);

	Function*
	parseStdFunction(
		StdNamespace stdNamespace,
		const sl::StringRef& source
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
