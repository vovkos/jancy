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

#include "jnc_ct_Type.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

namespace jnc {
namespace ct {

class Scope;
class StructField;
class LeanDataPtrValidator;

//..............................................................................

enum StdVariable
{
	StdVariable_SjljFrame,
	StdVariable_GcShadowStackTop,
	StdVariable_GcSafePointTrigger,
	StdVariable_NullPtrCheckSink,
	StdVariable__Count,
};

//..............................................................................

enum VariableFlag
{
	VariableFlag_Arg = 0x010000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Variable:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class VariableMgr;
	friend class FunctionMgr;
	friend class Parser;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;
	sl::BoxList<Token> m_constructor;
	Scope* m_scope;
	StructField* m_tlsField;
	void* m_staticData;
	ref::Ptr<LeanDataPtrValidator> m_leanDataPtrValidator;
	llvm::GlobalVariable* m_llvmGlobalVariable; // for classes this is different from m_llvmValue
	llvm::Value* m_llvmValue;       // GlobalVariable* / AllocaInst* / GEPInst* / CallInst*
	llvm::DIVariable_vn m_llvmDiDescriptor;  // DIVariable / DIGlobalVariable

	llvm::AllocaInst* m_llvmPreLiftValue; // we have to keep original allocas until the very end

public:
	LlvmIrInsertPoint m_liftInsertPoint;

public:
	Variable();

	Type*
	getType()
	{
		return m_type;
	}

	uint_t
	getPtrTypeFlags()
	{
		return m_ptrTypeFlags;
	}

	sl::ConstBoxList<Token>
	getConstructor()
	{
		return m_constructor;
	}

	Scope*
	getScope()
	{
		return m_scope;
	}

	StructField*
	getTlsField()
	{
		ASSERT(m_storageKind == StorageKind_Tls);
		return m_tlsField;
	}

	void*
	getStaticData()
	{
		return m_staticData ? m_staticData : prepareStaticData(), m_staticData;
	}

	LeanDataPtrValidator*
	getLeanDataPtrValidator()
	{
		return m_leanDataPtrValidator ? m_leanDataPtrValidator : prepareLeanDataPtrValidator(), m_leanDataPtrValidator;
	}

	llvm::GlobalVariable*
	getLlvmGlobalVariable()
	{
		return m_llvmGlobalVariable;
	}

	llvm::Value*
	getLlvmValue()
	{
		return m_llvmValue ? m_llvmValue : prepareLlvmValue(), m_llvmValue;
	}

	llvm::DIVariable_vn
	getLlvmDiDescriptor()
	{
		return m_llvmDiDescriptor;
	}

	bool
	isInitializationNeeded()
	{
		return
			!m_constructor.isEmpty() ||
			!m_initializer.isEmpty() ||
			m_type->getTypeKind() == TypeKind_Class; // static class variable
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

//..............................................................................

// after compiling and generating LLVM IR, we need to calc layout of TLS struct type
// then we can insert instructions to get TLS block in every function and then replace
// all alloca's temporarily representing TLS variables with GEPs into this TLS block

struct TlsVariable
{
	Variable* m_variable;
	llvm::AllocaInst* m_llvmAlloca;
};

//..............................................................................

} // namespace ct
} // namespace jnc
