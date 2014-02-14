// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"
#include "jnc_FunctionTypeOverload.h"
#include "jnc_NamedType.h"
#include "jnc_BasicBlock.h"
#include "jnc_Scope.h"
#include "jnc_Value.h"
#include "jnc_Closure.h"
#include "jnc_UnOp.h"
#include "jnc_BinOp.h"
#include "jnc_Variable.h"

namespace jnc {

class CDerivableType;
class CClassType;
class CPropertyType;
class CReactorClassType;
class CProperty;
class CJitEventListener;

//.............................................................................

enum EFunction
{
	EFunction_Undefined = 0,
	EFunction_Named,
	EFunction_Getter,
	EFunction_Setter,
	EFunction_Binder,
	EFunction_Primer,
	EFunction_PreConstructor,
	EFunction_Constructor,
	EFunction_Destructor,
	EFunction_StaticConstructor,
	EFunction_StaticDestructor,
	EFunction_ModuleConstructor,
	EFunction_ModuleDestructor,
	EFunction_CallOperator,
	EFunction_CastOperator,
	EFunction_UnaryOperator,
	EFunction_BinaryOperator,
	EFunction_Internal,
	EFunction_Thunk,
	EFunction_Reaction,
	EFunction_ScheduleLauncher,
	EFunction__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EFunctionKindFlag
{
	EFunctionKindFlag_NoStorage   = 0x01,
	EFunctionKindFlag_NoOverloads = 0x02,
	EFunctionKindFlag_NoArgs      = 0x04,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetFunctionKindString (EFunction FunctionKind);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

int
GetFunctionKindFlags (EFunction FunctionKind);

//.............................................................................

enum EStdFunc
{
	// void
	// jnc.RuntimeError (
	//		int Error,
	//		int8* pCodeAddr
	//		);

	EStdFunc_RuntimeError,

	// void
	// jnc.CheckNullPtr (
	//		int8* p,
	//		int Error
	//		);

	EStdFunc_CheckNullPtr,

	// void
	// jnc.CheckScopeLevel (
	//		jnc.TObjHdr* pSrcObject
	//		jnc.TObjHdr* pDstObject
	//		);

	EStdFunc_CheckScopeLevel,

	// void
	// jnc.CheckClassPtrScopeLevel (
	//		object* p,
	//		jnc.TObjHdr* pDstObject
	//		);

	EStdFunc_CheckClassPtrScopeLevel,

	// void
	// jnc.CheckDataPtrRange (
	//		int8* p,
	//		size_t Size,
	//		int8* pRangeBegin,
	//		int8* pRangeEnd
	//		);

	EStdFunc_CheckDataPtrRange,

	// object*
	// jnc.DynamicCastClassPtr (
	//		object* p,
	//		int8* pType
	//		);

	EStdFunc_DynamicCastClassPtr,

	// object*
	// jnc.StrengthenClassPtr (weak object* p);

	EStdFunc_StrengthenClassPtr,

	// size_t
	// jnc.GetDataPtrSpan (jnc.TDataPtr Ptr);

	EStdFunc_GetDataPtrSpan,

	// int8*
	// jnc.GcAllocate (
	//		int8* pType,
	//		size_t Count
	//		);

	EStdFunc_GcAllocate,

	// int8*
	// jnc.GcAllocate (
	//		int8* pType,
	//		size_t Count
	//		);

	EStdFunc_GcTryAllocate,

	// bool
	// jnc.GcEnter ();

	EStdFunc_GcEnter,

	// void
	// jnc.GcLeave ();

	EStdFunc_GcLeave,

	// void
	// jnc.GcPulse ();

	EStdFunc_GcPulse,

	// void
	// jnc.MarkGcRoot (
	//		int8* p,
	//		int8* pType
	//		);

	EStdFunc_MarkGcRoot,

	// void
	// jnc.RunGc ();

	EStdFunc_RunGc,

	// i64
	// jnc.GetCurrentThreadId ();

	EStdFunc_GetCurrentThreadId,

	// i64
	// jnc.CreateThread (function* pf ());

	EStdFunc_CreateThread,

	// void
	// jnc.sleep (uint_t MsCount);

	EStdFunc_Sleep,

	// uint64_t
	// jnc.getTimestamp ();

	EStdFunc_GetTimestamp,

	// char const*
	// jnc.format (char const* format, ...);

	EStdFunc_Format,

	// size_t
	// strlen (nullable const char* p);

	EStdFunc_StrLen,

	// void
	// jnc.memcpy (
	//		void* pDst,
	//		const void* pSrc,
	//		size_t Size
	//		);

	EStdFunc_MemCpy,

	// void*
	// jnc.memcat (
	//		const void* p1,
	//		size_t Size1,
	//		const void* p2,
	//		size_t Size2
	//		);

	EStdFunc_MemCat,

	// int
	// rand ();

	EStdFunc_Rand,

	// int
	// printf (char const* format, ...);

	EStdFunc_Printf,

	// jnc.TTlsStruct*
	// jnc.GetTls ();

	EStdFunc_GetTls,

	// size_t
	// jnc.AppendFmtLiteral_a (
	//		jnc.TFmtLiteral* pLiteral,
	//		char* p,
	//		size_t Length
	//		);

	EStdFunc_AppendFmtLiteral_a,

	// size_t
	// jnc.AppendFmtLiteral_p (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		jnc.TPtr Ptr
	//		);

	EStdFunc_AppendFmtLiteral_p,

	// size_t
	// jnc.AppendFmtLiteral_i32 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i32 i
	//		);

	EStdFunc_AppendFmtLiteral_i32,

	// size_t
	// jnc.AppendFmtLiteral_ui32 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i32 i
	//		);

	EStdFunc_AppendFmtLiteral_ui32,

	// size_t
	// jnc.AppendFmtLiteral_i64 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i64 i
	//		);

	EStdFunc_AppendFmtLiteral_i64,

	// size_t
	// jnc.AppendFmtLiteral_ui64 (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		i64 i
	//		);

	EStdFunc_AppendFmtLiteral_ui64,

	// size_t
	// jnc.AppendFmtLiteral_f (
	//		jnc.TFmtLiteral* pFmtLiteral,
	//		char* pFmtSpecifier,
	//		double f
	//		);

	EStdFunc_AppendFmtLiteral_f,

	EStdFunc_SimpleMulticastCall,

	// jnc.Error*
	// jnc.g_lastError.get ();

	EStdFunc_GetLastError,

	EStdFunc__Count
};

//.............................................................................

// shared between CFunction and COrphan

class CFunctionName
{
	friend class CParser;

protected:
	EFunction m_FunctionKind;

	union
	{
		EUnOp m_UnOpKind;
		EBinOp m_BinOpKind;
		CType* m_pCastOpType;
	};

	CQualifiedName m_DeclaratorName;
	uint_t m_ThisArgTypeFlags;

public:
	CFunctionName ()
	{
		m_FunctionKind = EFunction_Undefined;
		m_pCastOpType = NULL;
	}

	EFunction
	GetFunctionKind ()
	{
		return m_FunctionKind;
	}

	EUnOp
	GetUnOpKind ()
	{
		ASSERT (m_FunctionKind == EFunction_UnaryOperator);
		return m_UnOpKind;
	}

	EBinOp
	GetBinOpKind ()
	{
		ASSERT (m_FunctionKind == EFunction_BinaryOperator);
		return m_BinOpKind;
	}

	CType*
	GetCastOpType ()
	{
		ASSERT (m_FunctionKind == EFunction_CastOperator);
		return m_pCastOpType;
	}

	const CQualifiedName*
	GetDeclaratorName ()
	{
		return &m_DeclaratorName;
	}

	uint_t
	GetThisArgTypeFlags ()
	{
		return m_ThisArgTypeFlags;
	}
};

//.............................................................................

class CFunction:
	public CUserModuleItem,
	public CFunctionName
{
	friend class CModule;
	friend class CFunctionMgr;
	friend class CTypeMgr;
	friend class CDerivableType;
	friend class CClassType;
	friend class CProperty;
	friend class COrphan;
	friend class CParser;
	friend class CCast_FunctionPtr;
	friend class CJitEventListener;

protected:
	CFunctionType* m_pType;
	CFunctionTypeOverload m_TypeOverload;
	rtl::CArrayT <CFunction*> m_OverloadArray;

	// for non-static member methods

	CType* m_pThisArgType;
	CType* m_pThisType;
	intptr_t m_ThisArgDelta;

	// for virtual member methods

	CClassType* m_pVirtualOriginClassType;
	size_t m_ClassVTableIndex;

	// for property gettes/setters

	CProperty* m_pProperty;
	size_t m_PropertyVTableIndex;

	rtl::CBoxListT <CToken> m_Body;

	CBasicBlock* m_pEntryBlock;
	CScope* m_pScope;

	llvm::Function* m_pLlvmFunction;
	llvm::Instruction* m_pLlvmPostTlsPrologueInst;
	llvm::DISubprogram m_LlvmDiSubprogram;

	rtl::CArrayT <TTlsVariable> m_TlsVariableArray;

	// native machine code

	void* m_pfMachineCode;
	size_t m_MachineCodeSize;

public:
	CFunction ();

	CFunctionType*
	GetType ()
	{
		return m_pType;
	}

	CFunctionTypeOverload*
	GetTypeOverload ()
	{
		return &m_TypeOverload;
	}

	bool
	IsAccessor ()
	{
		return m_FunctionKind == EFunction_Getter || m_FunctionKind == EFunction_Setter;
	}

	bool
	IsMember ()
	{
		return m_pThisType != NULL;
	}

	bool
	IsVirtual ()
	{
		return m_StorageKind >= EStorage_Abstract && m_StorageKind <= EStorage_Override;
	}

	CClassType*
	GetVirtualOriginClassType ()
	{
		return m_pVirtualOriginClassType;
	}

	CDerivableType*
	GetParentType ()
	{
		return m_pParentNamespace->GetNamespaceKind () == ENamespace_Type ?
			(CDerivableType*) (CNamedType*) m_pParentNamespace : NULL;
	}

	CType*
	GetThisArgType ()
	{
		return m_pThisArgType;
	}

	CType*
	GetThisType ()
	{
		return m_pThisType;
	}

	intptr_t
	GetThisArgDelta ()
	{
		return m_ThisArgDelta;
	}

	size_t
	GetClassVTableIndex ()
	{
		return m_ClassVTableIndex;
	}

	CProperty*
	GetProperty ()
	{
		return m_pProperty;
	}

	size_t
	GetPropertyVTableIndex ()
	{
		return m_PropertyVTableIndex;
	}

	void
	ConvertToMemberMethod (CNamedType* pParentType);

	bool
	HasBody ()
	{
		return !m_Body.IsEmpty ();
	}

	rtl::CConstBoxListT <CToken>
	GetBody ()
	{
		return m_Body;
	}

	bool
	SetBody (rtl::CBoxListT <CToken>* pTokenList);

	CScope*
	GetScope ()
	{
		return m_pScope;
	}

	CBasicBlock*
	GetEntryBlock ()
	{
		return m_pEntryBlock;
	}

	llvm::Function*
	GetLlvmFunction ();

	llvm::Instruction*
	GetLlvmPostTlsPrologueInst ()
	{
		return m_pLlvmPostTlsPrologueInst;
	}

	llvm::DISubprogram
	GetLlvmDiSubprogram ();

	void*
	GetMachineCode ()
	{
		return m_pfMachineCode;
	}

	size_t
	GetMachineCodeSize ()
	{
		return m_MachineCodeSize;
	}

	rtl::CArrayT <TTlsVariable>
	GetTlsVariableArray ()
	{
		return m_TlsVariableArray;
	}

	void
	AddTlsVariable (CVariable* pVariable);

	bool
	IsOverloaded ()
	{
		return !m_OverloadArray.IsEmpty ();
	}

	size_t
	GetOverloadCount ()
	{
		return m_OverloadArray.GetCount () + 1;
	}

	CFunction*
	GetOverload (size_t Overload = 0)
	{
		return
			Overload == 0 ? this :
			Overload <= m_OverloadArray.GetCount () ? m_OverloadArray [Overload - 1] : NULL;
	}

	CFunction*
	FindOverload (CFunctionType* pType)
	{
		size_t i = m_TypeOverload.FindOverload (pType);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	FindShortOverload (CFunctionType* pType)
	{
		size_t i = m_TypeOverload.FindShortOverload (pType);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	ChooseOverload (
		CFunctionArg* const* pArgArray,
		size_t ArgCount,
		ECast* pCastKind = NULL
		)
	{
		size_t i = m_TypeOverload.ChooseOverload (pArgArray, ArgCount, pCastKind);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	ChooseOverload (
		const rtl::CConstBoxListT <CValue>& ArgList,
		ECast* pCastKind = NULL
		)
	{
		size_t i = m_TypeOverload.ChooseOverload (ArgList, pCastKind);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	ChooseSetterOverload (
		CType* pArgType,
		ECast* pCastKind = NULL
		)
	{
		size_t i = m_TypeOverload.ChooseSetterOverload (pArgType, pCastKind);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	ChooseSetterOverload (
		const CValue& ArgValue,
		ECast* pCastKind = NULL
		)
	{
		size_t i = m_TypeOverload.ChooseSetterOverload (ArgValue, pCastKind);
		return i != -1 ? GetOverload (i) : NULL;
	}

	CFunction*
	ChooseSetterOverload (
		CFunctionType* pFunctionType,
		ECast* pCastKind = NULL
		)
	{
		size_t i = m_TypeOverload.ChooseSetterOverload (pFunctionType, pCastKind);
		return i != -1 ? GetOverload (i) : NULL;
	}

	bool
	AddOverload (CFunction* pFunction);

	virtual
	bool
	Compile ();
};

//.............................................................................

class CLazyStdFunction: public CLazyModuleItem
{
	friend class CFunctionMgr;

protected:
	EStdFunc m_Func;

public:
	CLazyStdFunction ()
	{
		m_Func = (EStdFunc) -1;
	}

	virtual
	CModuleItem*
	GetActualItem ();
};

//.............................................................................

} // namespace jnc {
