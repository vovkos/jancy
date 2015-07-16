// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_Scope.h"
#include "jnc_LeanDataPtrValidator.h"

namespace jnc {

class Scope;
class StructField;

//.............................................................................

enum StdVariable
{
	StdVariable_GcShadowStackTop,
	StdVariable__Count,
};

//.............................................................................

class Variable: 
	public UserModuleItem,
	public ModuleItemInitializer
{
	friend class VariableMgr;
	friend class Parser;

protected:
	Type* m_type;
	ImportType* m_type_i;
	uint_t m_ptrTypeFlags;
	rtl::BoxList <Token> m_constructor;
	Scope* m_scope;
	StructField* m_tlsField;
	ref::Ptr <LeanDataPtrValidator> m_leanDataPtrValidator;
	llvm::Value* m_llvmValue;               // GlobalVariable* / AllocaInst* / GEPInst*
	llvm::DIDescriptor m_llvmDiDescriptor;  // DIVariable / DIGlobalVariable

public:
	Variable ();

	Type*
	getType ()
	{
		return m_type;
	}

	ImportType*
	getType_i ()
	{
		return m_type_i;
	}

	uint_t
	getPtrTypeFlags ()
	{
		return m_ptrTypeFlags;
	}

	rtl::ConstBoxList <Token>
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
		return m_tlsField;
	}

	LeanDataPtrValidator*
	getLeanDataPtrValidator ();

	llvm::Value*
	getLlvmValue ()
	{
		return m_llvmValue;
	}
	
	llvm::DIDescriptor
	getLlvmDiDescriptor ()
	{
		return m_llvmDiDescriptor;
	}

protected:
	virtual
	bool
	calcLayout ();
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

} // namespace jnc {


