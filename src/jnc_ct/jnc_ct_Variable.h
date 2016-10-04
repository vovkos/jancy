// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"
#include "jnc_ct_Scope.h"
#include "jnc_ct_LeanDataPtrValidator.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

namespace jnc {
namespace ct {

class Scope;
class StructField;

//.............................................................................

enum StdVariable
{
	StdVariable_SjljFrame,
	StdVariable_GcShadowStackTop,
	StdVariable_GcSafePointTrigger,
	StdVariable__Count,
};

//.............................................................................

enum VariableFlag
{
	VariableFlag_Arg        = 0x010000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	sl::BoxList <Token> m_constructor;
	Scope* m_scope;
	StructField* m_tlsField;
	void* m_staticData;
	ref::Ptr <LeanDataPtrValidator> m_leanDataPtrValidator;
	llvm::GlobalVariable* m_llvmGlobalVariable; // for classes this is different from m_llvmValue
	llvm::Value* m_llvmValue;       // GlobalVariable* / AllocaInst* / GEPInst* / CallInst*
	llvm::DIDescriptor m_llvmDiDescriptor;  // DIVariable / DIGlobalVariable

	llvm::AllocaInst* m_llvmPreLiftValue; // we have to keep original allocas until the very end

public:
	LlvmIrInsertPoint m_liftInsertPoint;

public:
	Variable ();

	Type*
	getType ()
	{
		return m_type;
	}

	uint_t
	getPtrTypeFlags ()
	{
		return m_ptrTypeFlags;
	}

	sl::ConstBoxList <Token>
	getConstructor ()
	{
		return m_constructor;
	}

	Scope*
	getScope ()
	{
		return m_scope;
	}

	StructField*
	getTlsField ()
	{
		ASSERT (m_storageKind == StorageKind_Tls);
		return m_tlsField;
	}

	void*
	getStaticData ();

	LeanDataPtrValidator*
	getLeanDataPtrValidator ();

	llvm::GlobalVariable*
	getLlvmGlobalVariable ()
	{
		return m_llvmGlobalVariable;
	}

	llvm::Value*
	getLlvmValue ();
	
	llvm::DIDescriptor
	getLlvmDiDescriptor ()
	{
		return m_llvmDiDescriptor;
	}

	bool
	isInitializationNeeded ();

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

//.............................................................................

// after compiling and generating LLVM IR, we need to calc layout of TLS struct type
// then we can insert instructions to get TLS block in every function and then replace
// all alloca's temporarily representing TLS variables with GEPs into this TLS block

struct TlsVariable
{
	Variable* m_variable;
	llvm::AllocaInst* m_llvmAlloca;
};

//.............................................................................

} // namespace ct
} // namespace jnc


