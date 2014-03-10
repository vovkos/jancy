// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_Scope.h"

namespace jnc {

class CScope;
class CStructField;

//.............................................................................

class CVariable: 
	public CUserModuleItem,
	public CModuleItemInitializer
{
	friend class CVariableMgr;
	friend class CParser;

protected:
	CType* m_pType;
	CImportType* m_pType_i;
	uint_t m_PtrTypeFlags;
	rtl::CBoxListT <CToken> m_Constructor;

	CScope* m_pScope;
	CStructField* m_pTlsField;
	llvm::Value* m_pLlvmValue; // AllocaInst* / GlobalVariable* / GEPInst*
	llvm::Value* m_pLlvmAllocValue;
	llvm::DIDescriptor m_LlvmDiDescriptor; // DIVariable / DIGlobalVariable /

public:
	CVariable ();

	CType*
	GetType ()
	{
		return m_pType;
	}

	CImportType*
	GetType_i ()
	{
		return m_pType_i;
	}

	uint_t
	GetPtrTypeFlags ()
	{
		return m_PtrTypeFlags;
	}

	rtl::CConstBoxListT <CToken>
	GetConstructor ()
	{
		return m_Constructor;
	}

	CScope*
	GetScope ()
	{
		return m_pScope;
	}

	size_t
	GetScopeLevel ()
	{
		return m_pScope ? m_pScope->GetLevel () : 0;
	}

	CValue
	GetScopeLevelObjHdr ();

	CStructField*
	GetTlsField ()
	{
		return m_pTlsField;
	}

	llvm::Value*
	GetLlvmValue ()
	{
		EnsureLlvmValue ();
		return m_pLlvmValue;
	}

	llvm::Value*
	GetLlvmAllocValue ()
	{
		EnsureLlvmValue ();
		return m_pLlvmAllocValue;
	}

	llvm::DIDescriptor
	GetLlvmDiDescriptor ()
	{
		EnsureLlvmValue ();
		return m_LlvmDiDescriptor;
	}

protected:
	virtual
	bool
	CalcLayout ();

	void
	EnsureLlvmValue ();
};

//.............................................................................

// after compiling and generating LLVM IR, we need to calc layout of TLS struct type
// then we can insert instructions to get TLS block in every function and then replace
// all alloca's temporarily representing TLS variables with GEPs into this TLS block

struct TTlsVariable
{
	CVariable* m_pVariable;
	llvm::AllocaInst* m_pLlvmAlloca;
};

//.............................................................................

} // namespace jnc {


