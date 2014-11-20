// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Variable.h"
#include "jnc_Alias.h"
#include "jnc_DestructList.h"

namespace jnc {

class ClassType;
class Function;

//.............................................................................

class VariableMgr
{
	friend class Module;
	friend class Variable;

protected:
	Module* m_module;

	rtl::StdList <Variable> m_variableList;
	rtl::StdList <Alias> m_aliasList;

	// static variables

	rtl::Array <Variable*> m_staticVariableArray;
	rtl::Array <Variable*> m_staticGcRootArray;
	rtl::Array <Variable*> m_globalStaticVariableArray;

	rtl::Array <llvm::GlobalVariable*> m_llvmGlobalVariableArray;

	// tls variables

	rtl::Array <Variable*> m_tlsVariableArray;
	rtl::Array <Variable*> m_tlsGcRootArray;
	StructType* m_tlsStructType;
	llvm::Value* m_llvmTlsObjHdrValue;

	Variable* m_stdVariableArray [StdVariable__Count];

public:
	DestructList m_staticDestructList;

public:
	VariableMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	Variable*
	getStdVariable (StdVariable variable);

	rtl::Array <Variable*>
	getStaticVariableArray ()
	{
		return m_staticVariableArray;
	}

	rtl::Array <Variable*>
	getStaticGcRootArray ()
	{
		return m_staticGcRootArray;
	}

	rtl::Array <Variable*>
	getGlobalStaticVariableArray ()
	{
		return m_globalStaticVariableArray;
	}

	rtl::Array <Variable*>
	getTlsVariableArray ()
	{
		return m_tlsVariableArray;
	}

	rtl::Array <Variable*>
	getTlsGcRootArray ()
	{
		return m_tlsGcRootArray;
	}

	StructType*
	getTlsStructType ()
	{
		ASSERT (m_tlsStructType);
		return m_tlsStructType;
	}

	Variable*
	createVariable (
		StorageKind storageKind,
		const rtl::String& name,
		const rtl::String& qualifiedName,
		Type* type,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		);

	Variable*
	createStackVariable (
		const rtl::String& name,
		Type* type,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		)
	{
		return createVariable (
			StorageKind_Stack,
			name,
			name,
			type,
			ptrTypeFlags,
			constructor,
			initializer
			);
	}

	Variable*
	createOnceFlagVariable (StorageKind storageKind = StorageKind_Static);

	Variable*
	createArgVariable (
		FunctionArg* arg,
		llvm::Value* llvmArgValue
		);

	llvm::GlobalVariable*
	createLlvmGlobalVariable (
		Type* type,
		const char* tag
		);

	Alias*
	createAlias (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		Type* type,
		rtl::BoxList <Token>* initializer
		);

	bool
	createTlsStructType ();

	bool
	allocatePrimeStaticVariable (Variable* variable);

	bool
	allocatePrimeStaticVariables ();

	bool
	initializeGlobalStaticVariables ();

	bool
	allocatePrimeInitializeVariable (Variable* variable);

	void
	allocateTlsVariable (Variable* variable);

	void
	allocateVariableObjHdr (Variable* variable);

	void
	deallocateTlsVariableArray (
		const TlsVariable* array,
		size_t count
		);

	void
	restoreTlsVariableArray (
		const TlsVariable* array,
		size_t count
		);

	void
	deallocateTlsVariableArray (const rtl::Array <TlsVariable>& array)
	{
		deallocateTlsVariableArray (array, array.getCount ());
	}

	void
	restoreTlsVariableArray (const rtl::Array <TlsVariable>& array)
	{
		restoreTlsVariableArray (array, array.getCount ());
	}

protected:
	bool
	allocatePrimeInitializeStaticVariable (Variable* variable);

	bool
	allocatePrimeInitializeTlsVariable (Variable* variable);

	bool
	allocatePrimeInitializeNonStaticVariable (Variable* variable);

	void
	allocateStaticVariableObjHdr (Variable* variable);

	void
	allocateTlsVariableObjHdr (Variable* variable);

	void
	getHeapVariableObjHdr (Variable* variable);

	void
	allocateStackVariableObjHdr (Variable* variable);

	void
	initializeVariableObjHdr (
		const Value& objHdrValue,
		const Value& scopeLevelValue,
		Type* type,
		uint_t flags,
		const Value& ptrValue
		);

	void
	initializeVariableObjHdr (
		const Value& objHdrValue,
		size_t scopeLevel,
		Type* type,
		uint_t flags,
		const Value& ptrValue
		)
	{
		initializeVariableObjHdr (objHdrValue, Value (scopeLevel, TypeKind_SizeT), type, flags, ptrValue);
	}

	void
	createStdVariables ();
};

//.............................................................................

} // namespace jnc {

