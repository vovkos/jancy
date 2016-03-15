// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Variable.h"
#include "jnc_ct_Alias.h"

namespace jnc {
namespace ct {

class ClassType;
class Function;

//.............................................................................

class VariableMgr
{
	friend class Module;
	friend class Variable;

protected:
	Module* m_module;

	sl::StdList <Variable> m_variableList;
	sl::StdList <Alias> m_aliasList;
	sl::Array <Variable*> m_staticVariableArray;
	sl::Array <Variable*> m_staticGcRootArray;
	sl::Array <Variable*> m_globalStaticVariableArray;
	sl::Array <Variable*> m_liftedStackVariableArray;
	sl::Array <Variable*> m_tlsVariableArray;
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

	void
	createStdVariables ();

	void
	finalizeLiftedStackVariables ();

	Variable*
	getStdVariable (StdVariable variable);

	sl::Array <Variable*>
	getStaticVariableArray ()
	{
		return m_staticVariableArray;
	}

	sl::Array <Variable*>
	getStaticGcRootArray ()
	{
		return m_staticGcRootArray;
	}

	sl::Array <Variable*>
	getGlobalStaticVariableArray ()
	{
		return m_globalStaticVariableArray;
	}

	sl::Array <Variable*>
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
		const sl::String& name,
		const sl::String& qualifiedName,
		Type* type,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		);

	Variable*
	createSimpleStackVariable (
		const sl::String& name,
		Type* type,
		uint_t ptrTypeFlags = 0
		)
	{
		return createVariable (StorageKind_Stack, name, name, type, ptrTypeFlags);
	}

	Variable*
	createSimpleStaticVariable (
		const sl::String& name,
		const sl::String& qualifiedName,
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
		const sl::String& name,
		const sl::String& qualifiedName,
		Type* type,
		sl::BoxList <Token>* initializer
		);

	bool
	createTlsStructType ();
	
	bool
	allocateInitializeGlobalVariables ();
	
	bool
	initializeVariable (Variable* variable);

	void
	liftStackVariable (Variable* variable);

	bool
	finalizeDisposableVariable (Variable* variable);

protected:
	llvm::GlobalVariable*
	createLlvmGlobalVariable (
		Type* type,
		const char* tag,
		const Value& initValue = Value ()
		);

	void
	primeStaticClassVariable (Variable* variable);

	bool
	allocateHeapVariable (Variable* variable);
};

//.............................................................................

} // namespace ct
} // namespace jnc

