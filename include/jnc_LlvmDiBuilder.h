// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Scope.h"

namespace jnc {

class Module;
class FunctionType;
class StructType;
class UnionType;
class ArrayType;
class Variable;
class Function;

//.............................................................................

enum
{
	DW_LANG_Jancy = llvm::dwarf::DW_LANG_lo_user + 1,
};

//.............................................................................

class LlvmDiBuilder
{
	friend class Module;

protected:
	Module* m_module;
	llvm::DIBuilder* m_llvmDiBuilder;

public:
	LlvmDiBuilder ();

	~LlvmDiBuilder ()
	{
		clear ();
	}

	Module*
	getModule ()
	{
		return m_module;
	}

	llvm::DIBuilder*
	getLlvmDiBuilder ()
	{
		return m_llvmDiBuilder;
	}

	void
	create ();

	void
	clear ();

	void
	finalize ()
	{
		m_llvmDiBuilder->finalize ();
	}

	llvm::DebugLoc
	getDebugLoc (
		Scope* scope,
		const Token::Pos& pos
		)
	{
		return llvm::DebugLoc::get (pos.m_line + 1, 0, scope->getLlvmDiScope ());
	}

	llvm::DebugLoc
	getEmptyDebugLoc ();

	llvm::DIFile
	createFile (
		const char* fileName,
		const char* dir
		)
	{
		return m_llvmDiBuilder->createFile (fileName, dir);
	}

	llvm::DIType
	createBasicType (
		const char* name,
		size_t size,
		size_t alignment,
		uint_t code
		)
	{
		return m_llvmDiBuilder->createBasicType (name, size * 8, alignment * 8, code);
	}

	llvm::DIType
	createSubroutineType (FunctionType* functionType);

	llvm::DIType
	createEmptyStructType (StructType* structType);

	void
	setStructTypeBody (StructType* structType);

	llvm::DIType
	createEmptyUnionType (UnionType* unionType);

	void
	setUnionTypeBody (UnionType* unionType);

	llvm::DIType
	createArrayType (ArrayType* arrayType);

	llvm::DIType
	createPointerType (Type* type);

	llvm::DIGlobalVariable
	createGlobalVariable (Variable* variable);

	llvm::DIVariable
	createLocalVariable (
		Variable* variable,
		uint_t tag = llvm::dwarf::DW_TAG_auto_variable
		);

	llvm::DISubprogram
	createFunction (Function* function);

	llvm::DILexicalBlock
	createLexicalBlock (
		Scope* parentScope,
		const Token::Pos& pos
		);

	llvm::Instruction*
	createDeclare (Variable* variable);
};

//.............................................................................

} // namespace jnc {
