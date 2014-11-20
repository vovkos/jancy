// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_ScheduleLauncherFunction.h"
#include "jnc_ThunkFunction.h"
#include "jnc_ThunkProperty.h"
#include "jnc_PropertyTemplate.h"
#include "jnc_NamespaceMgr.h"
#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

class FunctionMgr
{
	friend class Module;
	friend class DerivableType;
	friend class ClassType;
	friend class Function;
	friend class Parser;

protected:
	struct EmissionContext: rtl::ListLink
	{
		Function* m_currentFunction;

		Namespace* m_currentNamespace;
		Scope* m_currentScope;
		ScopeLevelStack m_scopeLevelStack;

		rtl::Array <BasicBlock*> m_returnBlockArray;
		BasicBlock* m_currentBlock;
		BasicBlock* m_unreachableBlock;
		uint_t m_controlFlowMgrFlags;

		Value m_thisValue;
		Value m_scopeLevelValue;
		rtl::BoxList <Value> m_tmpStackGcRootList;

		llvm::DebugLoc m_llvmDebugLoc;
	};

protected:
	Module* m_module;

	// unfortunately LLVM does not provide a slot for back-pointer from llvm::Function, hence the map

	rtl::HashTableMap <llvm::Function*, Function*, rtl::HashId <llvm::Function*> > m_llvmFunctionMap;

	rtl::StdList <Function> m_functionList;
	rtl::StdList <Property> m_propertyList;
	rtl::StdList <PropertyTemplate> m_propertyTemplateList;
	rtl::StdList <ScheduleLauncherFunction> m_scheduleLauncherFunctionList;
	rtl::StdList <ThunkFunction> m_thunkFunctionList;
	rtl::StdList <ThunkProperty> m_thunkPropertyList;
	rtl::StdList <DataThunkProperty> m_dataThunkPropertyList;
	rtl::StdList <LazyStdFunction> m_lazyStdFunctionList;
	rtl::StringHashTableMap <Function*> m_thunkFunctionMap;
	rtl::StringHashTableMap <Property*> m_thunkPropertyMap;
	rtl::StringHashTableMap <Function*> m_scheduleLauncherFunctionMap;

	Function* m_currentFunction;

	Value m_thisValue;
	Value m_scopeLevelValue;

	rtl::StdList <EmissionContext> m_emissionContextStack;

	Function* m_stdFunctionArray [StdFunction__Count];
	LazyStdFunction* m_lazyStdFunctionArray [StdFunction__Count];

public:
	FunctionMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	Function*
	getCurrentFunction ()
	{
		return m_currentFunction;
	}

	Function*
	setCurrentFunction (Function* function);

	Function*
	findFunctionByLlvmFunction (llvm::Function* llvmFunction)
	{
		rtl::HashTableMapIterator <llvm::Function*, Function*> it = m_llvmFunctionMap.find (llvmFunction);
		return it ? it->m_value : NULL;
	}

	Property*
	getCurrentProperty ()
	{
		return m_currentFunction ? m_currentFunction->getProperty () : NULL;
	}

	Value
	getThisValue ()
	{
		return m_thisValue;
	}

	Value
	overrideThisValue (const Value& value);

	Value
	getScopeLevel ()
	{
		return m_scopeLevelValue;
	}

	void
	clear ();

	rtl::ConstList <Function>
	getFunctionList ()
	{
		return m_functionList;
	}

	rtl::ConstList <Property>
	getPropertyList ()
	{
		return m_propertyList;
	}

	rtl::ConstList <ThunkFunction>
	getThunkFunctionList ()
	{
		return m_thunkFunctionList;
	}

	rtl::ConstList <ThunkProperty>
	getThunkPropertyList ()
	{
		return m_thunkPropertyList;
	}

	rtl::ConstList <DataThunkProperty>
	getDataThunkPropertyList ()
	{
		return m_dataThunkPropertyList;
	}

	Function*
	createFunction (
		FunctionKind functionKind,
		const rtl::String& name,
		const rtl::String& qualifiedName,
		const rtl::String& tag,
		FunctionType* type
		);

	Function*
	createFunction (
		FunctionKind functionKind,
		FunctionType* type
		)
	{
		return createFunction (functionKind, rtl::String (), rtl::String (), rtl::String (), type);
	}

	Function*
	createFunction (
		FunctionKind functionKind,
		const rtl::String& tag,
		FunctionType* type
		)
	{
		return createFunction (functionKind, rtl::String (), rtl::String (), tag, type);
	}

	Function*
	createFunction (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		FunctionType* type
		)
	{
		return createFunction (FunctionKind_Named, name, qualifiedName, qualifiedName, type);
	}

	Property*
	createProperty (
		PropertyKind propertyKind,
		const rtl::String& name,
		const rtl::String& qualifiedName,
		const rtl::String& tag
		);

	Property*
	createProperty (PropertyKind propertyKind)
	{
		return createProperty (propertyKind, rtl::String (), rtl::String (), rtl::String ());
	}

	Property*
	createProperty (
		PropertyKind propertyKind,
		const rtl::String& tag
		)
	{
		return createProperty (propertyKind, rtl::String (), rtl::String (), tag);
	}

	Property*
	createProperty (
		const rtl::String& name,
		const rtl::String& qualifiedName
		)
	{
		return createProperty (PropertyKind_Normal, name, qualifiedName, qualifiedName);
	}

	PropertyTemplate*
	createPropertyTemplate ();

	bool
	prologue (
		Function* function,
		const Token::Pos& pos
		);

	bool
	epilogue ();

	bool
	fireOnChanged ();

	void
	internalPrologue (
		Function* function,
		Value* argValueArray = NULL,
		size_t argCount = 0
		);

	void
	internalEpilogue ();

	bool
	injectTlsPrologues ();

	bool
	jitFunctions ();

	// std functions

	bool
	isStdFunctionUsed (StdFunction func)
	{
		ASSERT (func < StdFunction__Count);
		return m_stdFunctionArray [func] != NULL;
	}

	Function*
	getStdFunction (StdFunction func);

	LazyStdFunction*
	getLazyStdFunction (StdFunction func);

	Function*
	getDirectThunkFunction (
		Function* targetFunction,
		FunctionType* thunkFunctionType,
		bool hasUnusedClosure = false
		);

	Property*
	getDirectThunkProperty (
		Property* targetProperty,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure = false
		);

	Property*
	getDirectDataThunkProperty (
		Variable* targetVariable,
		PropertyType* thunkPropertyType,
		bool hasUnusedClosure = false
		);

	Function*
	getScheduleLauncherFunction (
		FunctionPtrType* targetFunctionPtrType,
		ClassPtrTypeKind schedulerPtrTypeKind = ClassPtrTypeKind_Normal
		);

protected:
	void
	createThisValue ();

	void
	pushEmissionContext ();

	void
	popEmissionContext ();

	void
	injectTlsPrologue (Function* function);

	// LLVM code support functions

	Function*
	parseStdFunction (
		StdNamespace stdNamespace,
		const char* source,
		size_t length
		);

	Function*
	createCheckNullPtr ();

	Function*
	createCheckScopeLevel ();

	Function*
	createCheckScopeLevelDirect ();

	Function*
	createCheckDataPtrRange ();

	Function*
	createCheckClassPtrScopeLevel ();

	Function*
	createGetDataPtrSpan ();
};

//.............................................................................

} // namespace jnc {
