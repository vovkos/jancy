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
#include "jnc_ct_Scope.h"

namespace jnc {
namespace ct {

class Module;
class FunctionType;
class StructType;
class UnionType;
class ArrayType;
class Variable;
class Function;

//..............................................................................

enum
{
	DW_LANG_Jancy = llvm::dwarf::DW_LANG_lo_user + 1,
};

//..............................................................................

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
		const sl::StringRef& fileName,
		const sl::StringRef& dir
		)
	{
		return m_llvmDiBuilder->createFile (fileName >> toLlvm, dir >> toLlvm);
	}

	llvm::DIType
	createBasicType (
		const sl::StringRef& name,
		size_t size,
		size_t alignment,
		uint_t code
		)
	{
		return m_llvmDiBuilder->createBasicType (name >> toLlvm, size * 8, alignment * 8, code);
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

//..............................................................................

} // namespace ct
} // namespace jnc
