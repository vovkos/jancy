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

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

namespace jnc {
namespace ct {

class Scope;
class Field;
class LeanDataPtrValidator;

//..............................................................................

enum StdVariable {
	StdVariable_SjljFrame,
	StdVariable_GcShadowStackTop,
	StdVariable_GcSafePointTrigger,
	StdVariable_NullPtrCheckSink,
	StdVariable_AsyncScheduler,
	StdVariable__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Variable:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class VariableMgr;
	friend class FunctionMgr;
	friend class ControlFlowMgr;
	friend class MemberBlock;
	friend class Property;
	friend class Parser;
	friend class Module;
	friend class Jit;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;
	StdVariable m_stdVariable;
	sl::List<Token> m_constructor;
	Scope* m_scope;
	Variable* m_declVariable;

	// codegen-only

	Field* m_field; // field variables only (TLS and reactor locals)
	void* m_staticData;
	rc::Ptr<LeanDataPtrValidator> m_leanDataPtrValidator;
	sl::String m_llvmGlobalVariableName;
	llvm::GlobalVariable* m_llvmGlobalVariable; // for classes this is different from m_llvmValue
	llvm::Value* m_llvmValue;                   // GlobalVariable* / AllocaInst* / GEPInst* / CallInst*
	llvm::DIVariable_vn m_llvmDiDescriptor;     // DIVariable / DIGlobalVariable
	llvm::AllocaInst* m_llvmPreLiftValue;       // we have to keep original allocas until the very end

public:
	LlvmIrInsertPoint m_liftInsertPoint;

public:
	Variable();

	Type*
	getType() {
		return m_type;
	}

	uint_t
	getPtrTypeFlags() {
		return m_ptrTypeFlags;
	}

	StdVariable
	getStdVariable() {
		return m_stdVariable;
	}

	sl::List<Token>*
	getConstructor() {
		return &m_constructor;
	}

	Scope*
	getScope() {
		return m_scope;
	}

	Field*
	getField() {
		ASSERT(m_storageKind == StorageKind_Tls || m_storageKind == StorageKind_Reactor);
		return m_field;
	}

	void*
	getStaticData();

	LeanDataPtrValidator*
	getLeanDataPtrValidator();

	Variable*
	getDeclVariable();

	const sl::String&
	getLlvmGlobalVariableName() {
		return m_llvmGlobalVariableName;
	}

	llvm::GlobalVariable*
	getLlvmGlobalVariable() {
		ASSERT(m_llvmGlobalVariable);
		return m_llvmGlobalVariable;
	}

	llvm::Value*
	getLlvmValue();

	llvm::DIVariable_vn
	getLlvmDiDescriptor() {
		return m_llvmDiDescriptor;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

protected:
	void
	prepareLlvmValue();

	void
	prepareLeanDataPtrValidator();

	void
	prepareStaticData();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
llvm::Value*
Variable::getLlvmValue() {
	if (!m_llvmValue)
		prepareLlvmValue();

	return m_llvmValue;
}

inline
void*
Variable::getStaticData() {
	if (!m_staticData)
		prepareStaticData();

	return m_staticData;
}

inline
LeanDataPtrValidator*
Variable::getLeanDataPtrValidator() {
	if (!m_leanDataPtrValidator)
		prepareLeanDataPtrValidator();

	return m_leanDataPtrValidator;
}

//..............................................................................

// TLS and reactor varialbles are actually fields, but we don't know the exact
// structure of their respecitve containers the moment we need their LLVM
// values. As such, we temporarily use `alloca`-s to represent such variables
// and then replace these `alloca`-s with `gep`s later, when we can calculate
// the layouts of their containers.

struct FieldVariable {
	Variable* m_variable;
	llvm::AllocaInst* m_llvmAlloca;

	FieldVariable() {
		m_variable = NULL;
		m_llvmAlloca = NULL;
	}

	FieldVariable(
		Variable* variable,
		llvm::AllocaInst* llvmAlloca
	) {
		m_variable = variable;
		m_llvmAlloca = llvmAlloca;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
