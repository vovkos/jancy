// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Scope.h"

namespace jnc {

class CModule;
class CFunctionType;
class CStructType;
class CUnionType;
class CArrayType;
class CVariable;
class CFunction;

//.............................................................................

enum
{
	DW_LANG_Jancy = llvm::dwarf::DW_LANG_lo_user + 1,
};

//.............................................................................

class CLlvmDiBuilder
{
	friend class CModule;

protected:
	CModule* m_pModule;
	llvm::DIBuilder* m_pLlvmDiBuilder;

public:
	CLlvmDiBuilder ();

	~CLlvmDiBuilder ()
	{
		Clear ();
	}

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	llvm::DIBuilder*
	GetLlvmDiBuilder ()
	{
		return m_pLlvmDiBuilder;
	}

	void
	Create ();

	void
	Clear ();

	void
	Finalize ()
	{
		m_pLlvmDiBuilder->finalize ();
	}

	llvm::DebugLoc
	GetDebugLoc (
		CScope* pScope,
		const CToken::CPos& Pos
		)
	{
		return llvm::DebugLoc::get (Pos.m_Line + 1, 0, pScope->GetLlvmDiScope ());
	}

	llvm::DebugLoc
	GetEmptyDebugLoc ();

	llvm::DIFile
	CreateFile (
		const char* pFileName,
		const char* pDir
		)
	{
		return m_pLlvmDiBuilder->createFile (pFileName, pDir);
	}

	llvm::DIType
	CreateBasicType (
		const char* pName,
		size_t Size,
		size_t AlignFactor,
		uint_t Code
		)
	{
		return m_pLlvmDiBuilder->createBasicType (pName, Size * 8, AlignFactor * 8, Code);
	}

	llvm::DIType
	CreateSubroutineType (CFunctionType* pFunctionType);

	llvm::DIType
	CreateEmptyStructType (CStructType* pStructType);

	void
	SetStructTypeBody (CStructType* pStructType);

	llvm::DIType
	CreateEmptyUnionType (CUnionType* pUnionType);

	void
	SetUnionTypeBody (CUnionType* pUnionType);

	llvm::DIType
	CreateArrayType (CArrayType* pArrayType);

	llvm::DIType
	CreatePointerType (CType* pType);

	llvm::DIGlobalVariable
	CreateGlobalVariable (CVariable* pVariable);

	llvm::DIVariable
	CreateLocalVariable (
		CVariable* pVariable,
		uint_t Tag = llvm::dwarf::DW_TAG_auto_variable
		);

	llvm::DISubprogram
	CreateFunction (CFunction* pFunction);

	llvm::DILexicalBlock
	CreateLexicalBlock (
		CScope* pParentScope,
		const CToken::CPos& Pos
		);

	llvm::Instruction*
	CreateDeclare (CVariable* pVariable);
};

//.............................................................................

} // namespace jnc {
