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
#include "jnc_ct_FunctionTypeOverload.h"
#include "jnc_ct_BasicBlock.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_Closure.h"
#include "jnc_ct_UnOp.h"
#include "jnc_ct_BinOp.h"
#include "jnc_ct_Variable.h"
#include "jnc_Function.h"

namespace jnc {
namespace ct {

class DerivableType;
class ClassType;
class PropertyType;
class Property;
class Scope;

//..............................................................................

// shared between CFunction and COrphan

class FunctionName
{
	friend class Parser;

protected:
	FunctionKind m_functionKind;

	union
	{
		UnOpKind m_unOpKind;
		BinOpKind m_binOpKind;
		Type* m_castOpType;
		Function* m_asyncLauncher;
	};

	QualifiedName m_declaratorName;
	uint_t m_thisArgTypeFlags;

public:
	FunctionName ()
	{
		m_functionKind = FunctionKind_Undefined;
		m_castOpType = NULL;
	}

	FunctionKind
	getFunctionKind ()
	{
		return m_functionKind;
	}

	UnOpKind
	getUnOpKind ()
	{
		ASSERT (m_functionKind == FunctionKind_UnaryOperator);
		return m_unOpKind;
	}

	BinOpKind
	getBinOpKind ()
	{
		ASSERT (m_functionKind == FunctionKind_BinaryOperator);
		return m_binOpKind;
	}

	Type*
	getCastOpType ()
	{
		ASSERT (m_functionKind == FunctionKind_CastOperator);
		return m_castOpType;
	}

	Function*
	getAsyncLauncher ()
	{
		ASSERT (m_functionKind == FunctionKind_Async);
		return m_asyncLauncher;
	}

	const QualifiedName*
	getDeclaratorName ()
	{
		return &m_declaratorName;
	}

	uint_t
	getThisArgTypeFlags ()
	{
		return m_thisArgTypeFlags;
	}
};

//..............................................................................

class Function:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer,
	public FunctionName
{
	friend class Module;
	friend class Unit;
	friend class FunctionMgr;
	friend class TypeMgr;
	friend class NamedTypeBlock;
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
	FunctionTypeOverload m_typeOverload;
	sl::Array <Function*> m_overloadArray;

	// for non-static member methods

	Type* m_thisArgType;
	Type* m_thisType;
	intptr_t m_thisArgDelta;

	// for virtual member methods

	ClassType* m_virtualOriginClassType;

	union
	{
		size_t m_classVTableIndex;
		size_t m_libraryTableIndex;
	};

	// for property gettes/setters

	Property* m_property;

	ExtensionNamespace* m_extensionNamespace;

	sl::BoxList <Token> m_body;
	UsingSet m_usingSet;

	BasicBlock* m_allocaBlock;
	BasicBlock* m_prologueBlock;
	Scope* m_scope;

	llvm::Function* m_llvmFunction;
	llvm::DISubprogram_vn m_llvmDiSubprogram;

	sl::Array <TlsVariable> m_tlsVariableArray;

	void* m_machineCode; // native machine code

public:
	Function ();

	FunctionType*
	getType ()
	{
		return m_type;
	}

	FunctionTypeOverload*
	getTypeOverload ()
	{
		return &m_typeOverload;
	}

	bool
	isTlsRequired ()
	{
		return !m_tlsVariableArray.isEmpty ();
	}

	bool
	isAccessor ()
	{
		return m_functionKind == FunctionKind_Getter || m_functionKind == FunctionKind_Setter;
	}

	bool
	isVoid ()
	{
		return m_type->getReturnType ()->getTypeKind () == TypeKind_Void;
	}

	bool
	isMember ()
	{
		return m_thisType != NULL;
	}

	bool
	isVirtual ()
	{
		return m_storageKind >= StorageKind_Abstract && m_storageKind <= StorageKind_Override;
	}

	ClassType*
	getVirtualOriginClassType ()
	{
		return m_virtualOriginClassType;
	}

	DerivableType*
	getParentType ()
	{
		return m_parentNamespace->getNamespaceKind () == NamespaceKind_Type ?
			(DerivableType*) (NamedType*) m_parentNamespace : NULL;
	}

	Type*
	getThisArgType ()
	{
		return m_thisArgType;
	}

	Type*
	getThisType ()
	{
		return m_thisType;
	}

	intptr_t
	getThisArgDelta ()
	{
		return m_thisArgDelta;
	}

	size_t
	getClassVTableIndex ()
	{
		return m_classVTableIndex;
	}

	size_t
	getLibraryTableIndex ()
	{
		return m_libraryTableIndex;
	}

	Property*
	getProperty ()
	{
		return m_property;
	}

	void
	convertToMemberMethod (DerivableType* parentType);

	bool
	hasBody ()
	{
		return !m_body.isEmpty ();
	}

	sl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (sl::BoxList <Token>* tokenList);

	void
	addUsingSet (UsingSet* usingSet);

	void
	addUsingSet (Namespace* anchorNamespace);

	Scope*
	getScope ()
	{
		return m_scope;
	}

	BasicBlock*
	getAllocaBlock ()
	{
		return m_allocaBlock;
	}

	BasicBlock*
	getPrologueBlock ()
	{
		return m_prologueBlock;
	}

	llvm::Function*
	getLlvmFunction ();

	llvm::DISubprogram_vn
	getLlvmDiSubprogram ();

	void*
	getMachineCode ()
	{
		return m_machineCode;
	}

	sl::Array <TlsVariable>
	getTlsVariableArray ()
	{
		return m_tlsVariableArray;
	}

	void
	addTlsVariable (Variable* variable);

	bool
	isOverloaded ()
	{
		return !m_overloadArray.isEmpty ();
	}

	size_t
	getOverloadCount ()
	{
		return m_overloadArray.getCount () + 1;
	}

	Function*
	getOverload (size_t overloadIdx)
	{
		return
			overloadIdx == 0 ? this :
			overloadIdx <= m_overloadArray.getCount () ? m_overloadArray [overloadIdx - 1] : NULL;
	}

	Function*
	findOverload (FunctionType* type)
	{
		size_t i = m_typeOverload.findOverload (type);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	findShortOverload (FunctionType* type)
	{
		size_t i = m_typeOverload.findShortOverload (type);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseOverload (
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseOverload (argArray, argCount, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseOverload (
		const Value* argValueArray,
		size_t argCount,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseOverload (argValueArray, argCount, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseOverload (
		const sl::ConstBoxList <Value>& argList,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseOverload (argList, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseSetterOverload (
		Type* argType,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseSetterOverload (argType, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseSetterOverload (
		const Value& argValue,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseSetterOverload (argValue, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	Function*
	chooseSetterOverload (
		FunctionType* functionType,
		CastKind* castKind = NULL
		)
	{
		size_t i = m_typeOverload.chooseSetterOverload (functionType, castKind);
		return i != -1 ? getOverload (i) : NULL;
	}

	size_t
	addOverload (Function* function);

	virtual
	bool
	compile ();

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

protected:
	bool
	compileConstructorBody ();

	bool
	compileNormalBody ();

	bool
	compileAsyncLauncher ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
