// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"
#include "jnc_FunctionTypeOverload.h"
#include "jnc_NamedType.h"
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
	FunctionKind_PreConstructor,
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
	// void
	// jnc.RuntimeError (
	//		int Error,
	//		int8* pCodeAddr
	//		);

	StdFunction_RuntimeError,

	// void
	// jnc.CheckNullPtr (
	//		int8* p,
	//		int Error
	//		);

	StdFunction_CheckNullPtr,

	// void
	// jnc.CheckScopeLevel (
	//		jnc.TObjHdr* pSrcObject
	//		jnc.TObjHdr* pDstObject
	//		);

	StdFunction_CheckScopeLevel,

	// void
	// jnc.CheckClassPtrScopeLevel (
	//		object* p,
	//		jnc.TObjHdr* pDstObject
	//		);

	StdFunction_CheckClassPtrScopeLevel,

	// void
	// jnc.CheckDataPtrRange (
	//		int8* p,
	//		size_t Size,
	//		int8* pRangeBegin,
	//		int8* pRangeEnd
	//		);

	StdFunction_CheckDataPtrRange,

	// object*
	// jnc.DynamicCastClassPtr (
	//		object* p,
	//		int8* pType
	//		);

	StdFunction_DynamicCastClassPtr,

	// object*
	// jnc.StrengthenClassPtr (weak object* p);

	StdFunction_StrengthenClassPtr,

	// size_t
	// jnc.GetDataPtrSpan (jnc.TDataPtr Ptr);

	StdFunction_GetDataPtrSpan,

	// int8*
	// jnc.GcAllocate (
	//		int8* pType,
	//		size_t Count
	//		);

	StdFunction_GcAllocate,

	// int8*
	// jnc.GcAllocate (
	//		int8* pType,
	//		size_t Count
	//		);

	StdFunction_GcTryAllocate,

	// bool
	// jnc.GcEnter ();

	StdFunction_GcEnter,

	// void
	// jnc.GcLeave ();

	StdFunction_GcLeave,

	// void
	// jnc.GcPulse ();

	StdFunction_GcPulse,

	// void
	// jnc.MarkGcRoot (
	//		int8* p,
	//		int8* pType
	//		);

	StdFunction_MarkGcRoot,

	// void
	// jnc.RunGc ();

	StdFunction_RunGc,

	// i64
	// jnc.GetCurrentThreadId ();

	StdFunction_GetCurrentThreadId,

	// i64
	// jnc.CreateThread (function* pf ());

	StdFunction_CreateThread,

	// void
	// jnc.sleep (uint_t MsCount);

	StdFunction_Sleep,

	// uint64_t
	// jnc.getTimestamp ();

	StdFunction_GetTimestamp,

	// char const*
	// jnc.format (char const* format, ...);

	StdFunction_Format,

	// size_t
	// strlen (const char* nullable p);

	StdFunction_StrLen,

	// void
	// jnc.memcpy (
	//		void* pDst,
	//		const void* pSrc,
	//		size_t Size
	//		);

	StdFunction_MemCpy,

	// void*
	// jnc.memcat (
	//		const void* p1,
	//		size_t Size1,
	//		const void* p2,
	//		size_t Size2
	//		);

	StdFunction_MemCat,

	// int
	// rand ();

	StdFunction_Rand,

	// int
	// printf (char const* format, ...);

	StdFunction_Printf,

	// int
	// atoi (char const* format);

	StdFunction_Atoi,

	// jnc.TTlsStruct*
	// jnc.GetTls ();

	StdFunction_GetTls,

	// size_t
	// jnc.AppendFmtLiteral_a (
	//		jnc.TFmtLiteral* pLiteral,
	//		char* p,
	//		size_t Length
	//		);

	StdFunction_AppendFmtLiteral_a,

	// size_t
	// jnc.AppendFmtLiteral_p (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		jnc.TPtr Ptr
	//		);

	StdFunction_AppendFmtLiteral_p,

	// size_t
	// jnc.AppendFmtLiteral_i32 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i32 i
	//		);

	StdFunction_AppendFmtLiteral_i32,

	// size_t
	// jnc.AppendFmtLiteral_ui32 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i32 i
	//		);

	StdFunction_AppendFmtLiteral_ui32,

	// size_t
	// jnc.AppendFmtLiteral_i64 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i64 i
	//		);

	StdFunction_AppendFmtLiteral_i64,

	// size_t
	// jnc.AppendFmtLiteral_ui64 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i64 i
	//		);

	StdFunction_AppendFmtLiteral_ui64,

	// size_t
	// jnc.AppendFmtLiteral_f (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		double f
	//		);

	StdFunction_AppendFmtLiteral_f,

	StdFunction_SimpleMulticastCall,

	// jnc.Error*
	// jnc.g_lastError.get ();

	StdFunction_GetLastError,

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

	rtl::BoxList <Token> m_body;

	BasicBlock* m_entryBlock;
	Scope* m_scope;

	llvm::Function* m_llvmFunction;
	llvm::Instruction* m_llvmPostTlsPrologueInst;
	llvm::DISubprogram m_llvmDiSubprogram;

	rtl::Array <TlsVariable> m_tlsVariableArray;

	// native machine code

	void* m_pfMachineCode;
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
	convertToMemberMethod (NamedType* parentType);

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
		return m_pfMachineCode;
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
