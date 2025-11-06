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

#include "jnc_ct_FunctionType.h"
#include "jnc_ct_FunctionName.h"
#include "jnc_ct_BasicBlock.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_Closure.h"
#include "jnc_ct_UnOp.h"
#include "jnc_ct_BinOp.h"
#include "jnc_ct_Variable.h"
#include "jnc_ct_AttributeBlock.h"
#include "jnc_Function.h"

namespace jnc {
namespace ct {

class DerivableType;
class ClassType;
class PropertyType;
class Property;
class Scope;
struct TemplateInstance;

//..............................................................................

class Function:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public ModuleItemInitializer,
	public ModuleItemUsingSet,
	public FunctionName {
	friend class Module;
	friend class Jit;
	friend class Unit;
	friend class FunctionMgr;
	friend class ControlFlowMgr;
	friend class CodeAssistMgr;
	friend class TypeMgr;
	friend class MemberBlock;
	friend class DerivableType;
	friend class StructType;
	friend class ClassType;
	friend class ArrayType;
	friend class Property;
	friend class ExtensionNamespace;
	friend class Template;
	friend class Orphan;
	friend class Parser;

protected:
	TemplateInstance* m_templateInstance;
	FunctionType* m_type;

	// for non-static member methods

	Type* m_thisArgType;
	Type* m_thisType;
	intptr_t m_thisArgDelta;

	// for virtual member methods

	ClassType* m_virtualOriginClassType;

	union {
		size_t m_classVtableIndex;
		size_t m_libraryTableIndex;
	};

	Property* m_property; // for property gettes/setters
	ExtensionNamespace* m_extensionNamespace;

	BasicBlock* m_allocaBlock;
	BasicBlock* m_prologueBlock;
	Scope* m_scope;
	Variable* m_declVariable;

	// codegen-only

	sl::Array<FieldVariable> m_tlsVariableArray;
	sl::Array<FieldVariable> m_reactorVariableArray;

	sl::String m_llvmFunctionName;
	llvm::Function* m_llvmFunction;
	llvm::DISubprogram_vn m_llvmDiSubprogram;
	void* m_machineCode; // native machine code

public:
	Function();

	FunctionType*
	getType() {
		return m_type;
	}

	TemplateInstance*
	getTemplateInstance() {
		return m_templateInstance;
	}

	bool
	isEmpty() {
		return !m_prologueBlock;
	}

	bool
	isAccessor() {
		return m_functionKind == FunctionKind_Getter || m_functionKind == FunctionKind_Setter;
	}

	bool
	isVoid() {
		return m_type->getReturnType()->getTypeKind() == TypeKind_Void;
	}

	bool
	isMember() {
		return m_thisType != NULL;
	}

	bool
	isVirtual() {
		return m_storageKind >= StorageKind_Abstract && m_storageKind <= StorageKind_Override;
	}

	bool
	isPrototype() {
		return m_attributeBlock && m_attributeBlock->findAttribute("prototype");
	}

	bool
	isUnusedExternal() {
		return m_llvmFunction == NULL;
	}

	ClassType*
	getVirtualOriginClassType() {
		return m_virtualOriginClassType;
	}

	DerivableType*
	getParentType() {
		return m_parentNamespace->getNamespaceKind() == NamespaceKind_Type ?
			(DerivableType*)(NamedType*)m_parentNamespace : NULL;
	}

	Type*
	getThisArgType() {
		return m_thisArgType;
	}

	Type*
	getThisType() {
		return m_thisType;
	}

	intptr_t
	getThisArgDelta() {
		return m_thisArgDelta;
	}

	size_t
	getClassVtableIndex() {
		return m_classVtableIndex;
	}

	size_t
	getLibraryTableIndex() {
		return m_libraryTableIndex;
	}

	Property*
	getProperty() {
		return m_property;
	}

	void
	convertToMemberMethod(DerivableType* parentType);

	Scope*
	getScope() {
		return m_scope;
	}

	BasicBlock*
	getAllocaBlock() {
		return m_allocaBlock;
	}

	BasicBlock*
	getPrologueBlock() {
		return m_prologueBlock;
	}

	Variable*
	getDeclVariable();

	bool
	hasLlvmFunction() {
		return m_llvmFunction != NULL;
	}

	const sl::String&
	getLlvmFunctionName() {
		return m_llvmFunctionName;
	}

	llvm::Function*
	getLlvmFunction();

	llvm::DISubprogram_vn
	getLlvmDiSubprogram();

	void*
	getMachineCode() {
		return m_machineCode;
	}

	sl::Array<FieldVariable>
	getTlsVariableArray() {
		return m_tlsVariableArray;
	}

	sl::Array<FieldVariable>
	getReactorVariableArray() {
		return m_reactorVariableArray;
	}

	void
	addTlsVariable(Variable* variable);

	void
	addReactorVariable(Variable* variable);

	bool
	canCompile() {
		return hasBody() || hasInitializer() || (m_flags & FunctionFlag_Compilable);
	}

	virtual
	bool
	require();

	virtual
	bool
	compile();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

protected:
	void
	prepareLlvmFunction();

	void
	prepareLlvmDiSubprogram();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Function::Function() {
	m_itemKind = ModuleItemKind_Function;
	m_functionKind = FunctionKind_Normal;
	m_templateInstance = NULL;
	m_type = NULL;
	m_castOpType = NULL;
	m_thisArgType = NULL;
	m_thisType = NULL;
	m_thisArgDelta = 0;
	m_thisArgTypeFlags = 0;
	m_virtualOriginClassType = NULL;
	m_property = NULL;
	m_extensionNamespace = NULL;
	m_classVtableIndex = -1;
	m_allocaBlock = NULL;
	m_prologueBlock = NULL;
	m_scope = NULL;
	m_declVariable = NULL;
	m_llvmFunction = NULL;
	m_machineCode = NULL;
}

inline
llvm::Function*
Function::getLlvmFunction() {
	if (!m_llvmFunction)
		prepareLlvmFunction();

	return m_llvmFunction;
}

inline
llvm::DISubprogram_vn
Function::getLlvmDiSubprogram() {
	if (!m_llvmDiSubprogram)
		prepareLlvmDiSubprogram();

	return m_llvmDiSubprogram;
}

inline
void
Function::addTlsVariable(Variable* variable) {
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*)variable->getLlvmValue();
	ASSERT(llvmAlloca && llvm::isa<llvm::AllocaInst>(*llvmAlloca));
	m_tlsVariableArray.append(FieldVariable(variable, llvmAlloca));
}

inline
void
Function::addReactorVariable(Variable* variable) {
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*)variable->getLlvmValue();
	ASSERT(llvmAlloca && llvm::isa<llvm::AllocaInst>(*llvmAlloca));
	m_reactorVariableArray.append(FieldVariable(variable, llvmAlloca));
}


//..............................................................................

class CompilableFunction: public Function {
public:
	CompilableFunction() {
		m_flags |= FunctionFlag_Compilable;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
