// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"
#include "jnc_FunctionTypeOverload.h"
#include "jnc_BasicBlock.h"
#include "jnc_Value.h"
#include "jnc_Closure.h"
#include "jnc_UnOp.h"
#include "jnc_BinOp.h"
#include "jnc_Variable.h"

namespace jnc {

class DerivableType;
class ClassType;
class PropertyType;
class ReactorClassType;
class Property;
class JitEventListener;
class Scope;

//.............................................................................

enum FunctionKind
{
	FunctionKind_Undefined = 0,
	FunctionKind_Named,
	FunctionKind_Getter,
	FunctionKind_Setter,
	FunctionKind_Binder,
	FunctionKind_Primer,
	FunctionKind_Preconstructor,
	FunctionKind_Constructor,
	FunctionKind_Destructor,
	FunctionKind_StaticConstructor,
	FunctionKind_StaticDestructor,
	FunctionKind_ModuleConstructor,
	FunctionKind_ModuleDestructor,
	FunctionKind_CallOperator,
	FunctionKind_CastOperator,
	FunctionKind_UnaryOperator,
	FunctionKind_BinaryOperator,
	FunctionKind_OperatorNew,
	FunctionKind_OperatorVararg,
	FunctionKind_OperatorCdeclVararg,
	FunctionKind_Internal,
	FunctionKind_Thunk,
	FunctionKind_Reaction,
	FunctionKind_ScheduleLauncher,
	FunctionKind__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum FunctionKindFlag
{
	FunctionKindFlag_NoStorage   = 0x01,
	FunctionKindFlag_NoOverloads = 0x02,
	FunctionKindFlag_NoArgs      = 0x04,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getFunctionKindString (FunctionKind functionKind);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
getFunctionKindFlags (FunctionKind functionKind);

//.............................................................................

enum StdFunction
{
	StdFunction_RuntimeError,
	StdFunction_CheckScopeLevel,
	StdFunction_CheckScopeLevelDirect,
	StdFunction_CheckClassPtrScopeLevel,
	StdFunction_DynamicSizeOf,
	StdFunction_DynamicCountOf,
	StdFunction_DynamicCastDataPtr,
	StdFunction_DynamicCastClassPtr,
	StdFunction_StrengthenClassPtr,
	StdFunction_GcAllocate,
	StdFunction_GcTryAllocate,
	StdFunction_GcEnter,
	StdFunction_GcLeave,
	StdFunction_GcPulse,
	StdFunction_MarkGcRoot,
	StdFunction_RunGc,
	StdFunction_GetCurrentThreadId,
	StdFunction_CreateThread,
	StdFunction_Sleep,
	StdFunction_GetTimestamp,
	StdFunction_Format,
	StdFunction_StrLen,
	StdFunction_StrCmp,
	StdFunction_StriCmp,
	StdFunction_StrChr,
	StdFunction_StrCat,
	StdFunction_StrDup,
	StdFunction_MemCmp,
	StdFunction_MemChr,
	StdFunction_MemCpy,
	StdFunction_MemSet,
	StdFunction_MemCat,
	StdFunction_MemDup,
	StdFunction_Rand,
	StdFunction_Printf,
	StdFunction_Atoi,
	StdFunction_GetTls,
	StdFunction_AppendFmtLiteral_a,
	StdFunction_AppendFmtLiteral_p,
	StdFunction_AppendFmtLiteral_i32,
	StdFunction_AppendFmtLiteral_ui32,
	StdFunction_AppendFmtLiteral_i64,
	StdFunction_AppendFmtLiteral_ui64,
	StdFunction_AppendFmtLiteral_f,
	StdFunction_AppendFmtLiteral_v,
	StdFunction_AppendFmtLiteral_s,
	StdFunction_AppendFmtLiteral_sr,
	StdFunction_AppendFmtLiteral_cb,
	StdFunction_AppendFmtLiteral_cbr,
	StdFunction_AppendFmtLiteral_br,
	StdFunction_SimpleMulticastCall,
	StdFunction_Throw,
	StdFunction_GetLastError,
	StdFunction_SetPosixError,
	StdFunction_SetStringError,
	StdFunction_AssertionFailure,
	StdFunction_AddStaticDestructor,
	StdFunction_AddDestructor,
	StdFunction_TryCheckDataPtrRange,
	StdFunction_CheckDataPtrRange,
	StdFunction_TryCheckNullPtr,
	StdFunction_CheckNullPtr,
	StdFunction_DynamicCastVariant,
	StdFunction__Count
};

//.............................................................................

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

//.............................................................................

class Function:
	public UserModuleItem,
	public FunctionName
{
	friend class Module;
	friend class FunctionMgr;
	friend class TypeMgr;
	friend class NamedTypeBlock;
	friend class DerivableType;
	friend class ClassType;
	friend class Property;
	friend class Orphan;
	friend class Parser;
	friend class Cast_FunctionPtr;
	friend class JitEventListener;

protected:
	FunctionType* m_type;
	FunctionTypeOverload m_typeOverload;
	rtl::Array <Function*> m_overloadArray;

	// for non-static member methods

	Type* m_thisArgType;
	Type* m_thisType;
	intptr_t m_thisArgDelta;

	// for virtual member methods

	ClassType* m_virtualOriginClassType;
	size_t m_classVTableIndex;

	// for property gettes/setters

	Property* m_property;
	size_t m_propertyVTableIndex;

	ExtensionNamespace* m_extensionNamespace;

	rtl::BoxList <Token> m_body;

	BasicBlock* m_entryBlock;
	Scope* m_scope;
	UsingSet m_usingSet;

	llvm::Function* m_llvmFunction;
	llvm::Instruction* m_llvmPostTlsPrologueInst;
	llvm::DISubprogram m_llvmDiSubprogram;

	rtl::Array <TlsVariable> m_tlsVariableArray;

	// native machine code

	void* m_machineCode;
	size_t m_machineCodeSize;

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
	isAccessor ()
	{
		return m_functionKind == FunctionKind_Getter || m_functionKind == FunctionKind_Setter;
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

	Property*
	getProperty ()
	{
		return m_property;
	}

	size_t
	getPropertyVTableIndex ()
	{
		return m_propertyVTableIndex;
	}

	void
	convertToMemberMethod (DerivableType* parentType);

	void
	convertToOperatorNew ();

	bool
	hasBody ()
	{
		return !m_body.isEmpty ();
	}

	rtl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (rtl::BoxList <Token>* tokenList);

	void
	markGc ();

	Scope*
	getScope ()
	{
		return m_scope;
	}

	BasicBlock*
	getEntryBlock ()
	{
		return m_entryBlock;
	}

	llvm::Function*
	getLlvmFunction ();

	llvm::Instruction*
	getLlvmPostTlsPrologueInst ()
	{
		return m_llvmPostTlsPrologueInst;
	}

	llvm::DISubprogram
	getLlvmDiSubprogram ();

	void*
	getMachineCode ()
	{
		return m_machineCode;
	}

	size_t
	getMachineCodeSize ()
	{
		return m_machineCodeSize;
	}

	rtl::Array <TlsVariable>
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
		const rtl::ConstBoxList <Value>& argList,
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
};

//.............................................................................

class LazyStdFunction: public LazyModuleItem
{
	friend class FunctionMgr;

protected:
	StdFunction m_func;

public:
	LazyStdFunction ()
	{
		m_func = (StdFunction) -1;
	}

	virtual
	ModuleItem*
	getActualItem ();
};

//.............................................................................

} // namespace jnc {
