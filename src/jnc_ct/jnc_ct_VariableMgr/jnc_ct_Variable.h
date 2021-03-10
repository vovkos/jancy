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

enum StdVariable
{
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
	public ModuleItemInitializer
{
	friend class VariableMgr;
	friend class FunctionMgr;
	friend class MemberBlock;
	friend class Property;
	friend class Type;
	friend class Parser;
	friend class Module;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;
	StdVariable m_stdVariable;
	sl::BoxList<Token> m_constructor;
	Scope* m_scope;

	// codegen-only

	Field* m_tlsField;
	void* m_staticData;
	ref::Ptr<LeanDataPtrValidator> m_leanDataPtrValidator;
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
	getType()
	{
		return m_type;
	}

	uint_t
	getPtrTypeFlags()
	{
		return m_ptrTypeFlags;
	}

	StdVariable
	getStdVariable()
	{
		return m_stdVariable;
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

	Field*
	getTlsField()
	{
		ASSERT(m_storageKind == StorageKind_Tls);
		return m_tlsField;
	}

	void*
	getStaticData();

	LeanDataPtrValidator*
	getLeanDataPtrValidator();

	llvm::GlobalVariable*
	getLlvmGlobalVariable()
	{
		ASSERT(m_llvmGlobalVariable);
		return m_llvmGlobalVariable;
	}

	llvm::Value*
	getLlvmValue();

	llvm::DIVariable_vn
	getLlvmDiDescriptor()
	{
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
Variable::getLlvmValue()
{
	if (!m_llvmValue)
		prepareLlvmValue();

	return m_llvmValue;
}

inline
void*
Variable::getStaticData()
{
	if (!m_staticData)
		prepareStaticData();

	return m_staticData;
}

inline
LeanDataPtrValidator*
Variable::getLeanDataPtrValidator()
{
	if (!m_leanDataPtrValidator)
		prepareLeanDataPtrValidator();

	return m_leanDataPtrValidator;
}

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
