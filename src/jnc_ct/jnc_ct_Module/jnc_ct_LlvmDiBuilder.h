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

	llvm::DIFile_vn
	createFile (
		const sl::StringRef& fileName,
		const sl::StringRef& dir
		)
	{
		return m_llvmDiBuilder->createFile (fileName >> toLlvm, dir >> toLlvm);
	}

	llvm::DIType_vn
	createBasicType (
		const sl::StringRef& name,
		size_t size,
		size_t alignment,
		uint_t code
		)
	{
#if (LLVM_VERSION < 0x0400)
		return m_llvmDiBuilder->createBasicType (name >> toLlvm, size * 8, alignment * 8, code);
#else
		return m_llvmDiBuilder->createBasicType (name >> toLlvm, size * 8, code);
#endif
	}

	llvm::DISubroutineType_vn
	createSubroutineType (FunctionType* functionType);

	llvm::DIType_vn
	createEmptyStructType (StructType* structType);

	void
	setStructTypeBody (StructType* structType);

	llvm::DIType_vn
	createEmptyUnionType (UnionType* unionType);

	void
	setUnionTypeBody (UnionType* unionType);

	llvm::DIType_vn
	createArrayType (ArrayType* arrayType);

	llvm::DIType_vn
	createPointerType (Type* type);

#if (LLVM_VERSION < 0x0400)
	// this method requires some serious porting on LLVM 4, but it's not used now anyway, so...
	llvm::DIGlobalVariable_vn
	createGlobalVariable (Variable* variable);
#endif

	llvm::DIVariable_vn
	createParameterVariable (
		Variable* variable,
		size_t argIdx
		);

	llvm::DISubprogram_vn
	createFunction (Function* function);

	llvm::DILexicalBlock_vn
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
