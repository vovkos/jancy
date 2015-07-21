// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Variable.h"
#include "jnc_Alias.h"

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

	// tls variables

	rtl::Array <Variable*> m_tlsVariableArray;
	StructType* m_tlsStructType;

	Variable* m_stdVariableArray [StdVariable__Count];

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
	createSimpleStackVariable (
		const rtl::String& name,
		Type* type,
		uint_t ptrTypeFlags = 0
		)
	{
		return createVariable (StorageKind_Stack, name, name, type, ptrTypeFlags);
	}

	Variable*
	createSimpleStaticVariable (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		Type* type,
		const Value& value = Value (),
		uint_t ptrTypeFlags = 0
		);

	Variable*
	createOnceFlagVariable (StorageKind storageKind = StorageKind_Static);

	Variable*
	createStaticDataPtrValidatorVariable (Variable* variable);

	Variable*
	createArgVariable (FunctionArg* arg);

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
	allocateInitializeGlobalVariables ();
	
	bool
	initializeVariable (Variable* variable);

	void
	liftStackVariable (Variable* variable);

protected:
	llvm::GlobalVariable*
	createLlvmGlobalVariable (
		Type* type,
		const char* tag,
		const Value& initValue = Value ()
		);

	void
	primeStaticClassVariable (Variable* variable);
};

//.............................................................................

} // namespace jnc {

