// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"
#include "jnc_UnOp_Arithmetic.h"
#include "jnc_UnOp_LogNot.h"
#include "jnc_UnOp_Ptr.h"
#include "jnc_UnOp_Inc.h"
#include "jnc_BinOp.h"
#include "jnc_BinOp_Arithmetic.h"
#include "jnc_BinOp_Logic.h"
#include "jnc_BinOp_Cmp.h"
#include "jnc_BinOp_At.h"
#include "jnc_BinOp_Idx.h"
#include "jnc_BinOp_Assign.h"
#include "jnc_CastOp.h"
#include "jnc_CastOp_Bool.h"
#include "jnc_CastOp_Int.h"
#include "jnc_CastOp_Fp.h"
#include "jnc_CastOp_Array.h"
#include "jnc_CastOp_Struct.h"
#include "jnc_CastOp_DataPtr.h"
#include "jnc_CastOp_ClassPtr.h"
#include "jnc_CastOp_FunctionPtr.h"
#include "jnc_CastOp_PropertyPtr.h"
#include "jnc_StructType.h"
#include "jnc_UnionType.h"
#include "jnc_ClassType.h"
#include "jnc_VariableMgr.h"
#include "jnc_FunctionMgr.h"

namespace jnc {

class CModule;

//.............................................................................

enum EStdCast
{
	EStdCast_Copy,
	EStdCast_SwapByteOrder,
	EStdCast_PtrFromInt,
	EStdCast_Int,
	EStdCast_Fp,
	EStdCast__Count
};

//.............................................................................

class COperatorMgr
{
	friend class CModule;
	friend class CVariableMgr;
	friend class CFunctionMgr;
	friend class CParser;
	friend class CCast_FunctionPtr;

protected:
	CModule* m_pModule;

	// unary arithmetics

	CUnOp_Plus m_UnOp_Plus;
	CUnOp_Minus m_UnOp_Minus;
	CUnOp_BwNot m_UnOp_BwNot;
	CUnOp_LogNot m_UnOp_LogNot;

	// pointer operators

	CUnOp_Addr m_UnOp_Addr;
	CUnOp_Indir m_UnOp_Indir;

	// increment operators

	CUnOp_PreInc m_UnOp_PreInc;
	CUnOp_PreInc m_UnOp_PreDec;
	CUnOp_PostInc m_UnOp_PostInc;
	CUnOp_PostInc m_UnOp_PostDec;

	// binary arithmetics

	CBinOp_Add m_BinOp_Add;
	CBinOp_Sub m_BinOp_Sub;
	CBinOp_Mul m_BinOp_Mul;
	CBinOp_Div m_BinOp_Div;
	CBinOp_Mod m_BinOp_Mod;
	CBinOp_Shl m_BinOp_Shl;
	CBinOp_Shr m_BinOp_Shr;
	CBinOp_BwAnd m_BinOp_BwAnd;
	CBinOp_BwXor m_BinOp_BwXor;
	CBinOp_BwOr m_BinOp_BwOr;

	// special operators

	CBinOp_At m_BinOp_At;
	CBinOp_Idx m_BinOp_Idx;

	// binary logic operators

	CBinOp_LogAnd m_BinOp_LogAnd;
	CBinOp_LogOr m_BinOp_LogOr;

	// comparison operators

	CBinOp_Eq m_BinOp_Eq;
	CBinOp_Ne m_BinOp_Ne;
	CBinOp_Lt m_BinOp_Lt;
	CBinOp_Le m_BinOp_Le;
	CBinOp_Gt m_BinOp_Gt;
	CBinOp_Ge m_BinOp_Ge;

	// assignment operators

	CBinOp_Assign m_BinOp_Assign;
	CBinOp_RefAssign m_BinOp_RefAssign;
	CBinOp_OpAssign m_BinOp_AddAssign;
	CBinOp_OpAssign m_BinOp_SubAssign;
	CBinOp_OpAssign m_BinOp_MulAssign;
	CBinOp_OpAssign m_BinOp_DivAssign;
	CBinOp_OpAssign m_BinOp_ModAssign;
	CBinOp_OpAssign m_BinOp_ShlAssign;
	CBinOp_OpAssign m_BinOp_ShrAssign;
	CBinOp_OpAssign m_BinOp_AndAssign;
	CBinOp_OpAssign m_BinOp_XorAssign;
	CBinOp_OpAssign m_BinOp_OrAssign;
	CBinOp_OpAssign m_BinOp_AtAssign;

	// cast operators

	CCast_Default m_Cast_Default;
	CCast_Copy m_Cast_Copy;
	CCast_SwapByteOrder m_Cast_SwapByteOrder;
	CCast_PtrFromInt m_Cast_PtrFromInt;
	CCast_Bool m_Cast_Bool;
	CCast_IntFromBool m_CastIntFromBool;
	CCast_Int m_Cast_Int;
	CCast_BeInt m_Cast_BeInt;
	CCast_Fp m_Cast_Fp;
	CCast_Array m_Cast_Array;
	CCast_Enum m_Cast_Enum;
	CCast_Struct m_Cast_Struct;
	CCast_DataPtr m_Cast_DataPtr;
	CCast_DataRef m_Cast_DataRef;
	CCast_ClassPtr m_Cast_ClassPtr;
	CCast_FunctionPtr m_Cast_FunctionPtr;
	CCast_FunctionRef m_Cast_FunctionRef;
	CCast_PropertyPtr m_Cast_PropertyPtr;
	CCast_PropertyRef m_Cast_PropertyRef;

	// tables

	CUnaryOperator* m_UnaryOperatorTable [EUnOp__Count];
	CBinaryOperator* m_BinaryOperatorTable [EBinOp__Count];
	CCastOperator* m_CastOperatorTable [EType__Count];
	CCastOperator* m_StdCastOperatorTable [EStdCast__Count];

	rtl::CBoxListT <CValue> m_TmpStackGcRootList;

public:
	COperatorMgr ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	void
	Clear ()
	{
		m_TmpStackGcRootList.Clear ();
	}

	void
	NullifyTmpStackGcRootList ()
	{
		NullifyGcRootList (m_TmpStackGcRootList);
		m_TmpStackGcRootList.Clear ();
	}

	void
	CreateTmpStackGcRoot (const CValue& Value);

	void
	MarkStackGcRoot (
		const CValue& PtrValue,
		CType* pType,
		bool IsTmpGcRoot = false
		);

	// load reference, get property, enum->int, bool->int, array->ptr -- unless specified otherwise with Flags

	void
	PrepareOperandType (
		const CValue& OpValue,
		CValue* pOpValue,
		uint_t OpFlags = 0
		);

	void
	PrepareOperandType (
		CValue* pOpValue,
		uint_t OpFlags = 0
		)
	{
		PrepareOperandType (*pOpValue, pOpValue, OpFlags);
	}

	CType*
	PrepareOperandType (
		const CValue& OpValue,
		uint_t OpFlags = 0
		);

	bool
	PrepareOperand (
		const CValue& OpValue,
		CValue* pOpValue,
		uint_t OpFlags = 0
		);

	bool
	PrepareOperand (
		CValue* pOpValue,
		uint_t OpFlags = 0
		)
	{
		return PrepareOperand (*pOpValue, pOpValue, OpFlags);
	}

	bool
	PrepareArgumentReturnValue (CValue* pValue);

	bool
	PrepareDataPtr (
		const CValue& Value,
		CValue* pResultValue
		);

	bool
	PrepareDataPtr (CValue* pValue)
	{
		return PrepareDataPtr (*pValue, pValue);
	}

	// unary operators

	CType*
	GetUnaryOperatorResultType (
		EUnOp OpKind,
		const CValue& OpValue
		);

	bool
	GetUnaryOperatorResultType (
		EUnOp OpKind,
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetUnaryOperatorResultType (
		EUnOp OpKind,
		CValue* pValue
		)
	{
		return GetUnaryOperatorResultType (OpKind, *pValue, pValue);
	}

	bool
	UnaryOperator (
		EUnOp OpKind,
		const CValue& OpValue,
		CValue* pResultValue = NULL
		);

	bool
	UnaryOperator (
		EUnOp OpKind,
		CValue* pValue
		)
	{
		return UnaryOperator (OpKind, *pValue, pValue);
	}

	// binary operators

	CType*
	GetBinaryOperatorResultType (
		EBinOp OpKind,
		const CValue& OpValue1,
		const CValue& OpValue2
		);

	bool
	GetBinaryOperatorResultType (
		EBinOp OpKind,
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue
		);

	bool
	GetBinaryOperatorResultType (
		EBinOp OpKind,
		CValue* pValue,
		const CValue& OpValue2
		)
	{
		return GetBinaryOperatorResultType (OpKind, *pValue, OpValue2, pValue);
	}

	bool
	BinaryOperator (
		EBinOp OpKind,
		const CValue& OpValue1,
		const CValue& OpValue2,
		CValue* pResultValue = NULL
		);

	bool
	BinaryOperator (
		EBinOp OpKind,
		CValue* pValue,
		const CValue& OpValue2
		)
	{
		return BinaryOperator (OpKind, *pValue, OpValue2, pValue);
	}

	// conditional operator

	CType*
	GetConditionalOperatorResultType (
		const CValue& TrueValue,
		const CValue& FalseValue
		);

	bool
	GetConditionalOperatorResultType (
		const CValue& TrueValue,
		const CValue& FalseValue,
		CValue* pResultValue
		);

	bool
	ConditionalOperator (
		const CValue& TrueValue,
		const CValue& FalseValue,
		CBasicBlock* pThenBlock,
		CBasicBlock* pPhiBlock,
		CValue* pResultValue = NULL
		);

	// cast operators

	CCastOperator*
	GetStdCastOperator (EStdCast CastKind)
	{
		ASSERT ((size_t) CastKind < EStdCast__Count);
		return m_StdCastOperatorTable [CastKind];
	}

	ECast
	GetCastKind (
		const CValue& OpValue,
		CType* pType
		);

	bool
	CheckCastKind (
		const CValue& OpValue,
		CType* pType
		);

	ECast
	GetArgCastKind (
		CFunctionType* pFunctionType,
		CFunctionArg* const* pArgArray,
		size_t ArgCount
		);

	ECast
	GetArgCastKind (
		CFunctionType* pFunctionType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray
		)
	{
		return GetArgCastKind (pFunctionType, ArgArray, ArgArray.GetCount ());
	}

	ECast
	GetArgCastKind (
		CFunctionType* pFunctionType,
		const rtl::CConstBoxListT <CValue>& ArgValueList
		);

	ECast
	GetFunctionCastKind (
		CFunctionType* pSrcType,
		CFunctionType* pDstType
		);

	ECast
	GetPropertyCastKind (
		CPropertyType* pSrcType,
		CPropertyType* pDstType
		);

	bool
	CastOperator (
		EStorage StorageKind,
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue = NULL
		);

	bool
	CastOperator (
		const CValue& OpValue,
		CType* pType,
		CValue* pResultValue = NULL
		)
	{
		return CastOperator (EStorage_Heap, OpValue, pType, pResultValue);
	}

	bool
	CastOperator (
		EStorage StorageKind,
		CValue* pValue,
		CType* pType
		)
	{
		return CastOperator (StorageKind, *pValue, pType, pValue);
	}

	bool
	CastOperator (
		CValue* pValue,
		CType* pType
		)
	{
		return CastOperator (EStorage_Heap, *pValue, pType, pValue);
	}

	bool
	CastOperator (
		EStorage StorageKind,
		const CValue& OpValue,
		EType TypeKind,
		CValue* pResultValue = NULL
		);

	bool
	CastOperator (
		const CValue& OpValue,
		EType TypeKind,
		CValue* pResultValue = NULL
		)
	{
		return CastOperator (EStorage_Heap, OpValue, TypeKind, pResultValue);
	}

	bool
	CastOperator (
		EStorage StorageKind,
		CValue* pValue,
		EType TypeKind
		)
	{
		return CastOperator (StorageKind, *pValue, TypeKind, pValue);
	}

	bool
	CastOperator (
		CValue* pValue,
		EType TypeKind
		)
	{
		return CastOperator (EStorage_Heap, *pValue, TypeKind, pValue);
	}

	// new & delete operators

	bool
	Allocate (
		EStorage StorageKind,
		CType* pType,
		const CValue& ElementCountValue,
		const char* pTag,
		CValue* pResultValue
		);

	bool
	Allocate (
		EStorage StorageKind,
		CType* pType,
		const char* pTag,
		CValue* pResultValue
		)
	{
		return Allocate (StorageKind, pType, CValue (), pTag, pResultValue);
	}

	bool
	Prime (
		EStorage StorageKind,
		const CValue& PtrValue,
		CType* pType,
		const CValue& ElementCountValue,
		CValue* pResultValue
		);

	bool
	Prime (
		EStorage StorageKind,
		const CValue& PtrValue,
		CType* pType,
		CValue* pResultValue
		)
	{
		return Prime (StorageKind, PtrValue, pType, CValue (), pResultValue);
	}

	bool
	Construct (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList = NULL
		);

	bool
	ParseInitializer (
		const CValue& Value,
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ConstructorTokenList,
		const rtl::CConstBoxListT <CToken>& InitializerTokenList
		);

	bool
	ParseExpressionEx (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		const CValue& ThrowReturnValue,
		uint_t ParserFlags,
		CValue* pResultValue
		);

	bool
	ParseThrowCondition (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		const CValue& ThrowReturnValue,
		CValue* pResultValue
		)
	{
		return ParseExpressionEx (pUnit, ExpressionTokenList, ThrowReturnValue, 0, pResultValue);
	}

	bool
	ParseExpression (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		uint_t ParserFlags,
		CValue* pResultValue
		)
	{
		return ParseExpressionEx (pUnit, ExpressionTokenList, CValue (), ParserFlags, pResultValue);
	}

	bool
	ParseExpression (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		CValue* pResultValue
		)
	{
		return ParseExpressionEx (pUnit, ExpressionTokenList, CValue (), 0, pResultValue);
	}

	bool
	ParseConstExpression (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		CValue* pResultValue
		);

	bool
	ParseConstIntegerExpression (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken>& ExpressionTokenList,
		intptr_t* pInteger
		);

	bool
	ParseAutoSizeArrayInitializer (
		const rtl::CConstBoxListT <CToken>& InitializerTokenList,
		size_t* pElementCount
		);

	size_t
	ParseAutoSizeArrayLiteralInitializer (const rtl::CConstBoxListT <CToken>& InitializerTokenList);

	size_t
	ParseAutoSizeArrayCurlyInitializer (const rtl::CConstBoxListT <CToken>& InitializerTokenList);

	CType*
	GetNewOperatorResultType (CType* pType)
	{
		return pType->GetTypeKind () == EType_Class ?
			(CType*) ((CClassType*) pType)->GetClassPtrType () :
			pType->GetDataPtrType ();
	}

	bool
	NewOperator (
		EStorage StorageKind,
		CType* pType,
		const CValue& ElementCountValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	NewOperator (
		EStorage StorageKind,
		CType* pType,
		CValue* pResultValue
		)
	{
		return NewOperator (StorageKind, pType, CValue (), NULL, pResultValue);
	}

	bool
	NewOperator (
		EStorage StorageKind,
		CType* pType,
		const CValue& ElementCountValue,
		CValue* pResultValue
		)
	{
		return NewOperator (StorageKind, pType, ElementCountValue, NULL, pResultValue);
	}

	bool
	NewOperator (
		EStorage StorageKind,
		CType* pType,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		)
	{
		return NewOperator (StorageKind, pType, CValue (), pArgValueList, pResultValue);
	}

	bool
	EvaluateAlias (
		CUnit* pUnit,
		const rtl::CConstBoxListT <CToken> TokenList,
		CValue* pResultValue
		);

	// member operators

	bool
	MemberOperator (
		const CValue& OpValue,
		size_t Index,
		CValue* pResultValue
		);

	bool
	MemberOperator (
		CValue* pValue,
		size_t Index
		)
	{
		return MemberOperator (*pValue, Index, pValue);
	}

	bool
	GetMemberOperatorResultType (
		const CValue& OpValue,
		const char* pName,
		CValue* pResultValue
		);

	bool
	GetMemberOperatorResultType (
		CValue* pValue,
		const char* pName
		)
	{
		return GetMemberOperatorResultType (*pValue, pName, pValue);
	}

	bool
	MemberOperator (
		const CValue& OpValue,
		const char* pName,
		CValue* pResultValue
		);

	bool
	MemberOperator (
		CValue* pValue,
		const char* pName
		)
	{
		return MemberOperator (*pValue, pName, pValue);
	}

	bool
	GetOffsetOf (
		const CValue& Value,
		CValue* pResultValue
		);

	bool
	GetOffsetOf (CValue* pValue)
	{
		return GetOffsetOf (*pValue, pValue);
	}

	// call operators

	void
	CallTraceFunction (
		const char* pFunctionName,
		const char* pString
		);

	CType*
	GetCallOperatorResultType (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList
		);

	bool
	GetCallOperatorResultType (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	GetCallOperatorResultType (
		CValue* pValue,
		rtl::CBoxListT <CValue>* pArgValueList
		)
	{
		return GetCallOperatorResultType (*pValue, pArgValueList, pValue);
	}

	bool
	CallOperator (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue = NULL
		);

	bool
	CallOperator (
		CValue* pValue,
		rtl::CBoxListT <CValue>* pArgValueList
		)
	{
		return CallOperator (*pValue, pArgValueList, pValue);
	}

	bool
	CallOperator (
		const CValue& OpValue,
		CValue* pResultValue = NULL
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		return CallOperator (OpValue, &ArgValueList, pResultValue);
	}

	bool
	CallOperator (
		const CValue& OpValue,
		const CValue& ArgValue,
		CValue* pResultValue = NULL
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		ArgValueList.InsertTail (ArgValue);
		return CallOperator (OpValue, &ArgValueList, pResultValue);
	}

	bool
	CallOperator2 (
		const CValue& OpValue,
		const CValue& ArgValue1,
		const CValue& ArgValue2,
		CValue* pResultValue = NULL
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		ArgValueList.InsertTail (ArgValue1);
		ArgValueList.InsertTail (ArgValue2);
		return CallOperator (OpValue, &ArgValueList, pResultValue);
	}

	void
	GcPulse ();

	// closure operators

	CType*
	GetClosureOperatorResultType (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList
		);

	bool
	GetClosureOperatorResultType (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	GetClosureOperatorResultType (
		CValue* pValue,
		rtl::CBoxListT <CValue>* pArgValueList
		)
	{
		return GetClosureOperatorResultType (*pValue,  pArgValueList, pValue);
	}

	bool
	ClosureOperator (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	ClosureOperator (
		CValue* pValue,
		rtl::CBoxListT <CValue>* pArgValueList
		)
	{
		return ClosureOperator (*pValue,  pArgValueList, pValue);
	}

	bool
	ClosureOperator (
		const CValue& OpValue,
		CValue* pResultValue
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		return ClosureOperator (OpValue, &ArgValueList, pResultValue);
	}

	bool
	ClosureOperator (
		const CValue& OpValue,
		const CValue& ArgValue,
		CValue* pResultValue
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		ArgValueList.InsertTail (ArgValue);
		return ClosureOperator (OpValue, &ArgValueList, pResultValue);
	}

	bool
	ClosureOperator2 (
		const CValue& OpValue,
		const CValue& ArgValue1,
		const CValue& ArgValue2,
		CValue* pResultValue
		)
	{
		rtl::CBoxListT <CValue> ArgValueList;
		ArgValueList.InsertTail (ArgValue1);
		ArgValueList.InsertTail (ArgValue2);
		return ClosureOperator (OpValue, &ArgValueList, pResultValue);
	}

	CType*
	GetFunctionType (
		const CValue& OpValue,
		CFunctionType* pFunctionType
		);

	// property getter

	CType*
	GetPropertyGetterType (const CValue& OpValue);

	bool
	GetPropertyGetterType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyGetterType (CValue* pValue)
	{
		return GetPropertyGetterType (*pValue, pValue);
	}

	bool
	GetPropertyGetter (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyGetter (CValue* pValue)
	{
		return GetPropertyGetter (*pValue, pValue);
	}

	// property setter

	CType*
	GetPropertySetterType (
		const CValue& OpValue,
		const CValue& ArgValue
		);

	bool
	GetPropertySetterType (
		const CValue& OpValue,
		const CValue& ArgValue,
		CValue* pResultValue
		);

	bool
	GetPropertySetterType (
		CValue* pValue,
		const CValue& ArgValue
		)
	{
		return GetPropertySetterType (*pValue, ArgValue, pValue);
	}

	bool
	GetPropertySetter (
		const CValue& OpValue,
		const CValue& ArgValue,
		CValue* pResultValue
		);

	bool
	GetPropertySetter (
		CValue* pValue,
		const CValue& ArgValue
		)
	{
		return GetPropertySetter (*pValue, ArgValue, pValue);
	}

	CType*
	GetPropertySetterType (const CValue& OpValue)
	{
		return GetPropertySetterType (OpValue, CValue ());
	}

	bool
	GetPropertySetterType (
		const CValue& OpValue,
		CValue* pResultValue
		)
	{
		return GetPropertySetterType (OpValue, CValue (), pResultValue);
	}

	bool
	GetPropertySetterType (CValue* pValue)
	{
		return GetPropertySetterType (*pValue, CValue (), pValue);
	}

	bool
	GetPropertySetter (
		const CValue& OpValue,
		CValue* pResultValue
		)
	{
		return GetPropertySetter (OpValue, CValue (), pResultValue);
	}

	bool
	GetPropertySetter (CValue* pValue)
	{
		return GetPropertySetter (*pValue, CValue (), pValue);
	}

	// property binder

	CType*
	GetPropertyBinderType (const CValue& OpValue);

	bool
	GetPropertyBinderType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyBinderType (CValue* pValue)
	{
		return GetPropertyBinderType (*pValue, pValue);
	}

	bool
	GetPropertyBinder (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyBinder (CValue* pValue)
	{
		return GetPropertyBinder (*pValue, pValue);
	}

	// autoget & onchanged

	CType*
	GetPropertyAutoGetValueType (const CValue& OpValue);

	bool
	GetPropertyAutoGetValueType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyAutoGetValueType (CValue* pValue)
	{
		return GetPropertyAutoGetValueType (*pValue, pValue);
	}

	bool
	GetPropertyAutoGetValue (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyAutoGetValue (CValue* pValue)
	{
		return GetPropertyAutoGetValue (*pValue, pValue);
	}

	CType*
	GetPropertyOnChangedType (const CValue& OpValue);

	bool
	GetPropertyOnChangedType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyOnChangedType (CValue* pValue)
	{
		return GetPropertyOnChangedType (*pValue, pValue);
	}

	bool
	GetPropertyOnChanged (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetPropertyOnChanged (CValue* pValue)
	{
		return GetPropertyOnChanged (*pValue, pValue);
	}

	// misc property functions

	bool
	GetProperty (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	SetProperty (
		const CValue& OpValue,
		const CValue& SrcValue
		);

	bool
	GetPropertyThinPtr (
		CProperty* pProperty,
		CClosure* pClosure,
		CPropertyPtrType* pPtrType,
		CValue* pResultValue
		);

	bool
	GetPropertyThinPtr (
		CProperty* pProperty,
		CClosure* pClosure,
		CValue* pResultValue
		)
	{
		return GetPropertyThinPtr (
			pProperty,
			pClosure,
			pProperty->GetType ()->GetPropertyPtrType (EPropertyPtrType_Thin),
			pResultValue
			);
	}

	bool
	GetPropertyVTable (
		CProperty* pProperty,
		CClosure* pClosure,
		CValue* pResultValue
		);

	bool
	GetPropertyVTable (
		const CValue& OpValue,
		CValue* pResultValue
		);

	// load & store operators

	bool
	LoadDataRef (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	LoadDataRef (CValue* pValue)
	{
		return LoadDataRef (*pValue, pValue);
	}

	bool
	StoreDataRef (
		const CValue& DstValue,
		const CValue& SrcValue
		);

	// weakening

	CClassPtrType*
	GetWeakenOperatorResultType (const CValue& OpValue);

	bool
	GetWeakenOperatorResultType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	GetWeakenOperatorResultType (CValue* pValue)
	{
		return GetWeakenOperatorResultType (*pValue, pValue);
	}

	bool
	WeakenOperator (
		const CValue& OpValue,
		CValue* pResultValue
		);

	bool
	WeakenOperator (CValue* pValue)
	{
		return WeakenOperator (*pValue, pValue);
	}

	// fields

	bool
	GetField (
		const CValue& OpValue,
		CStructField* pMember,
		CMemberCoord* pCoord,
		CValue* pResultValue
		);

	bool
	GetField (
		const CValue& OpValue,
		CStructField* pMember,
		CValue* pResultValue
		)
	{
		return GetField (OpValue, pMember, NULL, pResultValue);
	}

	bool
	GetField (
		CValue* pValue,
		CStructField* pMember
		)
	{
		return GetField (*pValue, pMember, NULL, pValue);
	}

	// impl

	bool
	GetFieldPtrImpl (
		const CValue& OpValue,
		CMemberCoord* pCoord,
		CType* pResultType,
		CValue* pResultValue
		);

	bool
	GetStructField (
		const CValue& OpValue,
		CStructField* pMember,
		CMemberCoord* pCoord,
		CValue* pResultValue
		);

	bool
	GetUnionField (
		const CValue& OpValue,
		CStructField* pMember,
		CValue* pResultValue
		);

	bool
	GetClassField (
		const CValue& OpValue,
		CStructField* pMember,
		CMemberCoord* pCoord,
		CValue* pResultValue
		);

	bool
	GetPropertyField (
		const CValue& OpValue,
		CModuleItem* pMember,
		CValue* pResultValue
		);

	// misc

	bool
	GetVirtualMethod (
		CFunction* pFunction,
		CClosure* pClosure,
		CValue* pResultValue
		);

	bool
	GetVirtualProperty (
		CProperty* pProperty,
		CClosure* pClosure,
		CValue* pResultValue
		);

	void
	GetLeanDataPtrRange (
		const CValue& Value,
		CValue* pRangeBeginValue,
		CValue* pRangeEndValue
		);

	void
	GetLeanDataPtrObjHdr (
		const CValue& Value,
		CValue* pResultValue
		);

	void
	NullifyGcRootList (const rtl::CConstBoxListT <CValue>& List);

	// closures

	bool
	CreateClosureObject (
		EStorage StorageKind,
		const CValue& OpValue, // thin function or property ptr with closure
		CType* pThunkType, // function or property type
		bool IsWeak,
		CValue* pResultValue
		);

	bool
	CreateDataClosureObject (
		EStorage StorageKind,
		const CValue& OpValue, // data ptr
		CPropertyType* pThunkType, // function or property type
		CValue* pResultValue
		);

	// checks

	void
	GetDataRefObjHdr (
		const CValue& Value,
		CValue* pResultValue
		);

	void
	CheckDataPtrRange (const CValue& Value);

	void
	CheckDataPtrRange (
		const CValue& PtrValue,
		size_t Size,
		const CValue& RangeBeginValue,
		const CValue& RangeEndValue
		);

	bool
	CheckDataPtrScopeLevel (
		const CValue& SrcValue,
		const CValue& DstValue
		); // can sometimes detect invalid assigns at compile time

	void
	CheckClassPtrNull (const CValue& Value);

	void
	CheckClassPtrScopeLevel (
		const CValue& SrcValue,
		const CValue& DstValue
		);

	void
	CheckFunctionPtrNull (const CValue& Value);

	void
	CheckFunctionPtrScopeLevel (
		const CValue& SrcValue,
		const CValue& DstValue
		);

	void
	CheckPropertyPtrNull (const CValue& Value);

	void
	CheckPropertyPtrScopeLevel (
		const CValue& SrcValue,
		const CValue& DstValue
		);

protected:
	// bit fields

	bool
	ExtractBitField (
		const CValue& Value,
		CBitFieldType* pBitFieldType,
		CValue* pResultValue
		);

	bool
	MergeBitField (
		const CValue& Value,
		const CValue& ShadowValue,
		CBitFieldType* pBitFieldType,
		CValue* pResultValue
		);

	// member operators

	bool
	GetNamespaceMemberType (
		CNamespace* pNamespace,
		const char* pName,
		CValue* pResultValue
		);

	bool
	GetNamespaceMember (
		CNamespace* pNamespace,
		const char* pName,
		CValue* pResultValue
		);

	bool
	GetNamedTypeMemberType (
		const CValue& OpValue,
		CNamedType* pNamedType,
		const char* pName,
		CValue* pResultValue
		);

	bool
	GetNamedTypeMember (
		const CValue& OpValue,
		CNamedType* pNamedType,
		const char* pName,
		CValue* pResultValue
		);

	bool
	GetClassVTable (
		const CValue& OpValue,
		CClassType* pClassType,
		CValue* pResultValue
		);

	bool
	CallImpl (
		const CValue& PfnValue,
		CFunctionType* pFunctionType,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	CallClosureFunctionPtr (
		const CValue& OpValue,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	bool
	CastArgValueList (
		CFunctionType* pFunctionType,
		rtl::CBoxListT <CValue>* pArgValueList
		);

	CType*
	GetUnsafeVarArgType (CType* pType);

	bool
	DeleteDataPtr (const CValue& OpValue);

	bool
	DeleteClassPtr (const CValue& OpValue);
};

//.............................................................................

} // namespace jnc {

