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

//..............................................................................

class Function:
	public ModuleItem,
	public ModuleItemBodyDecl,
	public ModuleItemInitializer,
	public FunctionName {
	friend class Module;
	friend class Unit;
	friend class FunctionMgr;
	friend class TypeMgr;
	friend class MemberBlock;
	friend class DerivableType;
	friend class StructType;
	friend class ClassType;
	friend class ArrayType;
	friend class Property;
	friend class ExtensionNamespace;
	friend class Orphan;
	friend class Parser;

protected:
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
	UsingSet m_usingSet;

	BasicBlock* m_allocaBlock;
	BasicBlock* m_prologueBlock;
	Scope* m_scope;

	// codegen-only

	sl::Array<TlsVariable> m_tlsVariableArray;
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

	bool
	isEmpty() {
		return !m_prologueBlock;
	}

	bool
	isTlsRequired() {
		return !m_tlsVariableArray.isEmpty();
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

	void
	addUsingSet(UsingSet* usingSet);

	void
	addUsingSet(Namespace* anchorNamespace);

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

	bool
	hasLlvmFunction() {
		return m_llvmFunction != NULL;
	}

	llvm::Function*
	getLlvmFunction();

	llvm::DISubprogram_vn
	getLlvmDiSubprogram();

	void*
	getMachineCode() {
		return m_machineCode;
	}

	sl::Array<TlsVariable>
	getTlsVariableArray() {
		return m_tlsVariableArray;
	}

	void
	addTlsVariable(Variable* variable);

	bool
	canCompile() {
		return hasBody() || hasInitializer() || (m_flags & (m_flags & ModuleItemFlag_Compilable));
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

//..............................................................................

class CompilableFunction: public Function {
public:
	CompilableFunction() {
		m_flags |= ModuleItemFlag_Compilable;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
