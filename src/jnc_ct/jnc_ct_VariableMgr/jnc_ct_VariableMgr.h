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

#include "jnc_ct_Variable.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class ClassType;
class Function;

//..............................................................................

class VariableMgr {
	friend class Module;
	friend class Variable;

protected:
	Module* m_module;

	sl::List<Variable> m_variableList;
	sl::Array<Variable*> m_staticVariableArray;
	sl::Array<Variable*> m_staticGcRootArray;
	sl::Array<Variable*> m_globalVariablePrimeArray;
	sl::Array<Variable*> m_globalVariableInitializeArray;
	sl::Array<Variable*> m_liftedStackVariableArray;
	sl::Array<Variable*> m_argVariableArray;
	sl::Array<Variable*> m_reactorVariableArray;
	sl::Array<Variable*> m_tlsVariableArray;
	Variable* m_currentLiftedStackVariable;
	StructType* m_tlsStructType;
	uint_t m_extraStackPtrFlags;

	Variable* m_stdVariableArray[StdVariable__Count];

public:
	VariableMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	void
	createStdVariables();

	void
	prepareForLandingPads() {
		m_extraStackPtrFlags |= PtrTypeFlag_Volatile;
	}

	void
	finalizeFunction();

	bool
	isStdVariableUsed(StdVariable variable) {
		ASSERT(variable < StdVariable__Count);
		return m_stdVariableArray[variable] != NULL;
	}

	Variable*
	getStdVariable(StdVariable variable);

	const sl::List<Variable>&
	getVariableList() {
		return m_variableList;
	}

	const sl::Array<Variable*>&
	getStaticGcRootArray() {
		return m_staticGcRootArray;
	}

	const sl::Array<Variable*>&
	getStaticVariableArray() {
		return m_staticVariableArray;
	}

	const sl::Array<Variable*>&
	getGlobalVariablePrimeArray() {
		return m_globalVariablePrimeArray;
	}

	const sl::Array<Variable*>&
	getGlobalVariableInitializeArray() {
		return m_globalVariableInitializeArray;
	}

	const sl::Array<Variable*>&
	getArgVariableArray() {
		return m_argVariableArray;
	}

	const sl::Array<Variable*>&
	getReactorVariableArray() {
		return m_reactorVariableArray;
	}

	const sl::Array<Variable*>&
	getTlsVariableArray() {
		return m_tlsVariableArray;
	}

	StructType*
	getTlsStructType() {
		ASSERT(m_tlsStructType);
		return m_tlsStructType;
	}

	Variable*
	getCurrentLiftedStackVariable() {
		return m_currentLiftedStackVariable;
	}

	Variable*
	createVariable(
		StorageKind storageKind,
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0,
		sl::List<Token>* constructor = NULL,
		sl::List<Token>* initializer = NULL
	);

	Variable*
	createSimpleStackVariable(
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0
	);

	Variable*
	createSimpleStaticVariable(
		const sl::StringRef& name,
		Type* type,
		const Value& value = Value(),
		uint_t ptrTypeFlags = 0
	);

	Variable*
	createOnceFlagVariable(StorageKind storageKind = StorageKind_Static);

	LeanDataPtrValidator*
	createStaticDataPtrValidator(Variable* variable);

	Variable*
	createArgVariable(
		FunctionArg* arg,
		size_t argIdx
	);

	Variable*
	createAsyncArgVariable(
		const sl::StringRef& name,
		Type* type,
		const Value& value
	);

	Variable*
	createStaticRegexVariable(const re2::Regex& regex);

	Variable*
	getRegexMatchVariable();

	Variable*
	createRtlItemVariable(
		StdType type,
		const sl::StringRef& name,
		ModuleItem* item
	);

	bool
	createTlsStructType();

	ClassType*
	createReactorUserDataType(const sl::StringRef& name);

	bool
	allocateVariable(Variable* variable);

	void
	primeStaticClassVariable(Variable* variable);

	bool
	initializeVariable(Variable* variable);

	void
	liftStackVariable(Variable* variable);

	bool
	finalizeDisposableVariable(Variable* variable);

	bool
	allocateNamespaceVariables(const sl::ConstIterator<Variable>& prevIt);

	void
	primeGlobalVariables();

	bool
	initializeGlobalVariables();

	void
	appendGlobalVariablePrimeArray(const sl::ArrayRef<Variable*>& array) {
		m_globalVariablePrimeArray.append(array);
	}

protected:
	llvm::GlobalVariable*
	createLlvmGlobalVariable(
		Type* type,
		const sl::StringRef& name,
		const Value& initValue = Value()
	);

	bool
	allocateHeapVariable(Variable* variable);
};

//..............................................................................

} // namespace ct
} // namespace jnc
