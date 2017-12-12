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

#include "jnc_ct_UnOp.h"
#include "jnc_ct_UnOp_Arithmetic.h"
#include "jnc_ct_UnOp_LogNot.h"
#include "jnc_ct_UnOp_Ptr.h"
#include "jnc_ct_UnOp_Inc.h"
#include "jnc_ct_BinOp.h"
#include "jnc_ct_BinOp_Arithmetic.h"
#include "jnc_ct_BinOp_Logic.h"
#include "jnc_ct_BinOp_Cmp.h"
#include "jnc_ct_BinOp_At.h"
#include "jnc_ct_BinOp_Idx.h"
#include "jnc_ct_BinOp_Assign.h"
#include "jnc_ct_CastOp.h"
#include "jnc_ct_CastOp_Bool.h"
#include "jnc_ct_CastOp_Int.h"
#include "jnc_ct_CastOp_Fp.h"
#include "jnc_ct_CastOp_Variant.h"
#include "jnc_ct_CastOp_Array.h"
#include "jnc_ct_CastOp_Struct.h"
#include "jnc_ct_CastOp_DataPtr.h"
#include "jnc_ct_CastOp_ClassPtr.h"
#include "jnc_ct_CastOp_FunctionPtr.h"
#include "jnc_ct_CastOp_PropertyPtr.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_VariableMgr.h"
#include "jnc_ct_FunctionMgr.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

enum StdCast
{
	StdCast_Copy,
	StdCast_SwapByteOrder,
	StdCast_PtrFromInt,
	StdCast_Int,
	StdCast_Fp,
	StdCast_FromVariant,
	StdCast__Count
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum OperatorDynamism
{
	OperatorDynamism_Static = 0,
	OperatorDynamism_Dynamic,
};

//..............................................................................

class OperatorMgr
{
	friend class Module;
	friend class VariableMgr;
	friend class FunctionMgr;
	friend class Parser;
	friend class Cast_FunctionPtr;

protected:
	Module* m_module;

	// unary arithmetics

	UnOp_Plus m_unOp_Plus;
	UnOp_Minus m_unOp_Minus;
	UnOp_BwNot m_unOp_BwNot;
	UnOp_LogNot m_unOp_LogNot;

	// pointer operators

	UnOp_Addr m_unOp_Addr;
	UnOp_Indir m_unOp_Indir;

	// increment operators

	UnOp_PreInc m_unOp_PreInc;
	UnOp_PreInc m_unOp_PreDec;
	UnOp_PostInc m_unOp_PostInc;
	UnOp_PostInc m_unOp_PostDec;

	// binary arithmetics

	BinOp_Add m_binOp_Add;
	BinOp_Sub m_binOp_Sub;
	BinOp_Mul m_binOp_Mul;
	BinOp_Div m_binOp_Div;
	BinOp_Mod m_binOp_Mod;
	BinOp_Shl m_binOp_Shl;
	BinOp_Shr m_binOp_Shr;
	BinOp_BwAnd m_binOp_BwAnd;
	BinOp_BwXor m_binOp_BwXor;
	BinOp_BwOr m_binOp_BwOr;

	// special operators

	BinOp_At m_binOp_At;
	BinOp_Idx m_binOp_Idx;

	// binary logic operators

	BinOp_LogAnd m_binOp_LogAnd;
	BinOp_LogOr m_binOp_LogOr;

	// comparison operators

	BinOp_Eq m_binOp_Eq;
	BinOp_Ne m_binOp_Ne;
	BinOp_Lt m_binOp_Lt;
	BinOp_Le m_binOp_Le;
	BinOp_Gt m_binOp_Gt;
	BinOp_Ge m_binOp_Ge;

	// assignment operators

	BinOp_Assign m_binOp_Assign;
	BinOp_RefAssign m_binOp_RefAssign;
	BinOp_OpAssign m_binOp_AddAssign;
	BinOp_OpAssign m_binOp_SubAssign;
	BinOp_OpAssign m_binOp_MulAssign;
	BinOp_OpAssign m_binOp_DivAssign;
	BinOp_OpAssign m_binOp_ModAssign;
	BinOp_OpAssign m_binOp_ShlAssign;
	BinOp_OpAssign m_binOp_ShrAssign;
	BinOp_OpAssign m_binOp_AndAssign;
	BinOp_OpAssign m_binOp_XorAssign;
	BinOp_OpAssign m_binOp_OrAssign;
	BinOp_OpAssign m_binOp_AtAssign;

	// cast operators

	Cast_Default m_cast_Default;
	Cast_Void m_cast_Void;
	Cast_Copy m_cast_Copy;
	Cast_SwapByteOrder m_cast_SwapByteOrder;
	Cast_PtrFromInt m_cast_PtrFromInt;
	Cast_Bool m_cast_Bool;
	Cast_IntFromBool m_castIntFromBool;
	Cast_Int m_cast_Int;
	Cast_BeInt m_cast_BeInt;
	Cast_Fp m_cast_Fp;
	Cast_Variant m_cast_Variant;
	Cast_FromVariant m_cast_FromVariant;
	Cast_Array m_cast_Array;
	Cast_Enum m_cast_Enum;
	Cast_Struct m_cast_Struct;
	Cast_DataPtr m_cast_DataPtr;
	Cast_DataRef m_cast_DataRef;
	Cast_ClassPtr m_cast_ClassPtr;
	Cast_FunctionPtr m_cast_FunctionPtr;
	Cast_FunctionRef m_cast_FunctionRef;
	Cast_PropertyPtr m_cast_PropertyPtr;
	Cast_PropertyRef m_cast_PropertyRef;

	// tables

	UnaryOperator* m_unaryOperatorTable [UnOpKind__Count];
	BinaryOperator* m_binaryOperatorTable [BinOpKind__Count];
	CastOperator* m_castOperatorTable [TypeKind__Count];
	CastOperator* m_stdCastOperatorTable [StdCast__Count];

	// unsafe blocks

	intptr_t m_unsafeEnterCount;

public:
	OperatorMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	void
	enterUnsafeRgn ()
	{
		m_unsafeEnterCount++;
	}

	void
	leaveUnsafeRgn ()
	{
		m_unsafeEnterCount--;
	}

	void
	resetUnsafeRgn ()
	{
		m_unsafeEnterCount = 0;
	}

	bool
	isUnsafeRgn ()
	{
		return m_unsafeEnterCount > 0;
	}

	// load reference, get property, enum->int, bool->int, array->ptr -- unless specified otherwise with Flags

	void
	prepareOperandType (
		const Value& opValue,
		Value* resultValue,
		uint_t opFlags = 0
		);

	void
	prepareOperandType (
		Value* opValue,
		uint_t opFlags = 0
		)
	{
		prepareOperandType (*opValue, opValue, opFlags);
	}

	Type*
	prepareOperandType (
		const Value& opValue,
		uint_t opFlags = 0
		);

	bool
	prepareOperand (
		const Value& opValue,
		Value* resultValue,
		uint_t opFlags = 0
		);

	bool
	prepareOperand (
		Value* opValue,
		uint_t opFlags = 0
		)
	{
		return prepareOperand (*opValue, opValue, opFlags);
	}

	bool
	prepareArgumentReturnValue (Value* value);

	bool
	prepareDataPtr (
		const Value& value,
		Value* resultValue
		);

	bool
	prepareDataPtr (Value* value)
	{
		return prepareDataPtr (*value, value);
	}

	void
	prepareArrayRef (
		const Value& value,
		Value* resultValue
		);

	void
	prepareArrayRef (Value* value)
	{
		prepareArrayRef (*value, value);
	}

	// unary operators

	Type*
	getUnaryOperatorResultType (
		UnOpKind opKind,
		const Value& opValue
		);

	bool
	getUnaryOperatorResultType (
		UnOpKind opKind,
		const Value& opValue,
		Value* resultValue
		);

	bool
	getUnaryOperatorResultType (
		UnOpKind opKind,
		Value* value
		)
	{
		return getUnaryOperatorResultType (opKind, *value, value);
	}

	bool
	unaryOperator (
		UnOpKind opKind,
		const Value& opValue,
		Value* resultValue = NULL
		);

	bool
	unaryOperator (
		UnOpKind opKind,
		Value* value
		)
	{
		return unaryOperator (opKind, *value, value);
	}

	// binary operators

	Type*
	getBinaryOperatorResultType (
		BinOpKind opKind,
		const Value& opValue1,
		const Value& opValue2
		);

	bool
	getBinaryOperatorResultType (
		BinOpKind opKind,
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);

	bool
	getBinaryOperatorResultType (
		BinOpKind opKind,
		Value* value,
		const Value& opValue2
		)
	{
		return getBinaryOperatorResultType (opKind, *value, opValue2, value);
	}

	bool
	binaryOperator (
		BinOpKind opKind,
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue = NULL
		);

	bool
	logicalOrOperator (
		BasicBlock* opBlock1,
		BasicBlock* opBlock2,
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue = NULL
		);

	bool
	logicalAndOperator (
		BasicBlock* opBlock1,
		BasicBlock* opBlock2,
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue = NULL
		);

	bool
	binaryOperator (
		BinOpKind opKind,
		Value* value,
		const Value& opValue2
		)
	{
		return binaryOperator (opKind, *value, opValue2, value);
	}

	bool
	logicalOrOperator (
		BasicBlock* opBlock1,
		BasicBlock* opBlock2,
		Value* value,
		const Value& opValue2
		)
	{
		return logicalOrOperator (opBlock1, opBlock2, *value, opValue2, value);
	}

	bool
	logicalAndOperator (
		BasicBlock* opBlock1,
		BasicBlock* opBlock2,
		Value* value,
		const Value& opValue2
		)
	{
		return logicalAndOperator (opBlock1, opBlock2, *value, opValue2, value);
	}

	// conditional operator

	Type*
	getConditionalOperatorResultType (
		const Value& trueValue,
		const Value& falseValue
		);

	bool
	getConditionalOperatorResultType (
		const Value& trueValue,
		const Value& falseValue,
		Value* resultValue
		);

	bool
	conditionalOperator (
		const Value& trueValue,
		const Value& falseValue,
		BasicBlock* thenBlock,
		BasicBlock* phiBlock,
		Value* resultValue = NULL
		);

	// cast operators

	void
	forceCast (
		const Value& value,
		Type* type,
		Value* resultValue
		);

	void
	forceCast (
		Value* value,
		Type* type
		)
	{
		forceCast (*value, type, value);
	}

	CastOperator*
	getStdCastOperator (StdCast castKind)
	{
		ASSERT ((size_t) castKind < StdCast__Count);
		return m_stdCastOperatorTable [castKind];
	}

	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	bool
	checkCastKind (
		const Value& opValue,
		Type* type
		);

	CastKind
	getArgCastKind (
		FunctionType* functionType,
		FunctionArg* const* argArray,
		size_t argCount
		);

	CastKind
	getArgCastKind (
		FunctionType* functionType,
		const Value* argValueArray,
		size_t argCount
		);

	CastKind
	getArgCastKind (
		FunctionType* functionType,
		const sl::Array <FunctionArg*>& argArray
		)
	{
		return getArgCastKind (functionType, argArray, argArray.getCount ());
	}

	CastKind
	getArgCastKind (
		FunctionType* functionType,
		const sl::ConstBoxList <Value>& argValueList
		);

	CastKind
	getFunctionCastKind (
		FunctionType* srcType,
		FunctionType* dstType
		);

	CastKind
	getPropertyCastKind (
		PropertyType* srcType,
		PropertyType* dstType
		);

	bool
	castOperator (
		OperatorDynamism dynamism,
		const Value& opValue,
		Type* type,
		Value* resultValue = NULL
		);

	bool
	castOperator (
		OperatorDynamism dynamism,
		Value* value,
		Type* type
		)
	{
		return castOperator (dynamism, *value, type, value);
	}

	bool
	castOperator (
		OperatorDynamism dynamism,
		const Value& opValue,
		TypeKind typeKind,
		Value* resultValue = NULL
		);

	bool
	castOperator (
		OperatorDynamism dynamism,
		Value* value,
		TypeKind typeKind
		)
	{
		return castOperator (dynamism, *value, typeKind, value);
	}

	bool
	castOperator (
		const Value& opValue,
		Type* type,
		Value* resultValue = NULL
		)
	{
		return castOperator (OperatorDynamism_Static, opValue, type, resultValue);
	}

	bool
	castOperator (
		Value* value,
		Type* type
		)
	{
		return castOperator (OperatorDynamism_Static, *value, type, value);
	}

	bool
	castOperator (
		const Value& opValue,
		TypeKind typeKind,
		Value* resultValue = NULL
		)
	{
		return castOperator (OperatorDynamism_Static, opValue, typeKind, resultValue);
	}

	bool
	castOperator (
		Value* value,
		TypeKind typeKind
		)
	{
		return castOperator (OperatorDynamism_Static, *value, typeKind, value);
	}

	// sizeof

	bool
	sizeofOperator (
		OperatorDynamism dynamism,
		const Value& opValue,
		Value* resultValue = NULL
		);

	bool
	sizeofOperator (
		OperatorDynamism dynamism,
		Value* value
		)
	{
		return sizeofOperator (dynamism, *value, value);
	}

	bool
	sizeofOperator (
		const Value& opValue,
		Value* resultValue = NULL
		)
	{
		return sizeofOperator (OperatorDynamism_Static, opValue, resultValue);
	}

	bool
	sizeofOperator (Value* value)
	{
		return sizeofOperator (OperatorDynamism_Static, *value, value);
	}

	// countof

	bool
	countofOperator (
		OperatorDynamism dynamism,
		const Value& opValue,
		Value* resultValue = NULL
		);

	bool
	countofOperator (
		OperatorDynamism dynamism,
		Value* value
		)
	{
		return countofOperator (dynamism, *value, value);
	}

	bool
	countofOperator (
		const Value& opValue,
		Value* resultValue = NULL
		)
	{
		return countofOperator (OperatorDynamism_Static, opValue, resultValue);
	}

	bool
	countofOperator (Value* value)
	{
		return countofOperator (OperatorDynamism_Static, *value, value);
	}

	// typeof

	bool
	typeofOperator (
		OperatorDynamism dynamism,
		const Value& opValue,
		Value* resultValue = NULL
		);

	bool
	typeofOperator (
		OperatorDynamism dynamism,
		Value* value
		)
	{
		return typeofOperator (dynamism, *value, value);
	}

	bool
	typeofOperator (
		const Value& opValue,
		Value* resultValue = NULL
		)
	{
		return typeofOperator (OperatorDynamism_Static, opValue, resultValue);
	}

	bool
	typeofOperator (Value* value)
	{
		return typeofOperator (OperatorDynamism_Static, *value, value);
	}

	// offsetof

	bool
	offsetofOperator (
		const Value& value,
		Value* resultValue
		);

	bool
	offsetofOperator (Value* value)
	{
		return offsetofOperator (*value, value);
	}

	void
	zeroInitialize (const Value& value);

	bool
	memSet (
		const Value& value,
		char c,
		size_t size,
		size_t alignment = 1,
		bool isVolatile = false
		);

	bool
	memCpy (
		StdFunc stdFunc,
		const Value& dstValue,
		const Value& srcValue,
		size_t size,
		size_t alignment = 1,
		bool isVolatile = false
		);

	bool
	memCpy (
		const Value& dstValue,
		const Value& srcValue,
		size_t size,
		size_t alignment = 1,
		bool isVolatile = false
		)
	{
		return memCpy (StdFunc_LlvmMemcpy, dstValue, srcValue, size, alignment, isVolatile);
	}

	bool
	memMove (
		const Value& dstValue,
		const Value& srcValue,
		size_t size,
		size_t alignment = 1,
		bool isVolatile = false
		)
	{
		return memCpy (StdFunc_LlvmMemmove, dstValue, srcValue, size, alignment, isVolatile);
	}

	bool
	construct (
		const Value& opValue,
		sl::BoxList <Value>* argValueList = NULL
		);

	bool
	parseInitializer (
		const Value& value,
		const sl::ConstBoxList <Token>& constructorTokenList,
		const sl::ConstBoxList <Token>& initializerTokenList
		);

	bool
	parseFunctionArgDefaultValue (
		ModuleItemDecl* decl,
		const sl::ConstBoxList <Token> tokenList,
		Value* resultValue
		);

	bool
	parseFunctionArgDefaultValue (
		ModuleItemDecl* decl,
		const Value& thisValue,
		const sl::ConstBoxList <Token> tokenList,
		Value* resultValue
		);

	bool
	parseFunctionArgDefaultValue (
		ModuleItemDecl* decl,
		Closure* closure,
		const sl::ConstBoxList <Token> tokenList,
		Value* resultValue
		)
	{
		return closure && closure->isMemberClosure () ?
			parseFunctionArgDefaultValue (decl, closure->getThisArgValue (), tokenList, resultValue) :
			parseFunctionArgDefaultValue (decl, Value (), tokenList, resultValue);
	}

	bool
	parseExpression (
		const sl::ConstBoxList <Token>& expressionTokenList,
		uint_t parserFlags,
		Value* resultValue
		);

	bool
	parseExpression (
		const sl::ConstBoxList <Token>& expressionTokenList,
		Value* resultValue
		)
	{
		return parseExpression (expressionTokenList, 0, resultValue);
	}

	bool
	parseConstExpression (
		const sl::ConstBoxList <Token>& expressionTokenList,
		Value* resultValue
		);

	bool
	parseConstIntegerExpression (
		const sl::ConstBoxList <Token>& expressionTokenList,
		int64_t* integer
		);

	size_t
	parseAutoSizeArrayInitializer (
		ArrayType* arrayType,
		const sl::ConstBoxList <Token>& initializerTokenList
		);

	size_t
	parseAutoSizeArrayLiteralInitializer (const sl::ConstBoxList <Token>& initializerTokenList);

	size_t
	parseAutoSizeArrayCurlyInitializer (
		ArrayType* arrayType,
		const sl::ConstBoxList <Token>& initializerTokenList
		);

	Type*
	getNewOperatorResultType (Type* type)
	{
		return type->getTypeKind () == TypeKind_Class ?
			(Type*) ((ClassType*) type)->getClassPtrType () :
			type->getDataPtrType ();
	}

	bool
	gcHeapAllocate (
		Type* type,
		const Value& rawElementCountValue,
		Value* resultValue
		);

	bool
	gcHeapAllocate (
		Type* type,
		Value* resultValue
		)
	{
		return gcHeapAllocate (type, Value (), resultValue);
	}

	bool
	newOperator (
		Type* type,
		const Value& elementCountValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	newOperator (
		Type* type,
		Value* resultValue
		)
	{
		return newOperator (type, Value (), NULL, resultValue);
	}

	bool
	newOperator (
		Type* type,
		const Value& elementCountValue,
		Value* resultValue
		)
	{
		return newOperator (type, elementCountValue, NULL, resultValue);
	}

	bool
	newOperator (
		Type* type,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		)
	{
		return newOperator (type, Value (), argValueList, resultValue);
	}

	// member operators

	bool
	createMemberClosure (
		Value* value,
		ModuleItemDecl* itemDecl = NULL
		);

	bool
	getThisValue (
		Value* value,
		ModuleItemDecl* itemDecl = NULL
		);

	bool
	getThisValueType (
		Value* value,
		ModuleItemDecl* itemDecl = NULL
		);

	bool
	memberOperator (
		const Value& opValue,
		size_t index,
		Value* resultValue
		);

	bool
	memberOperator (
		Value* value,
		size_t index
		)
	{
		return memberOperator (*value, index, value);
	}

	bool
	getMemberOperatorResultType (
		const Value& opValue,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getMemberOperatorResultType (
		Value* value,
		const sl::StringRef& name
		)
	{
		return getMemberOperatorResultType (*value, name, value);
	}

	bool
	memberOperator (
		const Value& opValue,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	memberOperator (
		Value* value,
		const sl::StringRef& name
		)
	{
		return memberOperator (*value, name, value);
	}

	// call operators

	void
	callTraceFunction (
		const sl::StringRef& functionName,
		const sl::StringRef& string
		);

	Type*
	getCallOperatorResultType (
		const Value& opValue,
		sl::BoxList <Value>* argValueList
		);

	bool
	getCallOperatorResultType (
		const Value& opValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	getCallOperatorResultType (
		Value* value,
		sl::BoxList <Value>* argValueList
		)
	{
		return getCallOperatorResultType (*value, argValueList, value);
	}

	bool
	callOperator (
		const Value& opValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue = NULL
		);

	bool
	callOperator (
		Value* value,
		sl::BoxList <Value>* argValueList
		)
	{
		return callOperator (*value, argValueList, value);
	}

	bool
	callOperator (
		const Value& opValue,
		Value* resultValue = NULL
		)
	{
		sl::BoxList <Value> argValueList;
		return callOperator (opValue, &argValueList, resultValue);
	}

	bool
	callOperator (
		const Value& opValue,
		const Value& argValue,
		Value* resultValue = NULL
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue);
		return callOperator (opValue, &argValueList, resultValue);
	}

	bool
	callOperator (
		const Value& opValue,
		const Value& argValue1,
		const Value& argValue2,
		Value* resultValue = NULL
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue1);
		argValueList.insertTail (argValue2);
		return callOperator (opValue, &argValueList, resultValue);
	}

	bool
	callOperator (
		const Value& opValue,
		const Value& argValue1,
		const Value& argValue2,
		const Value& argValue3,
		Value* resultValue = NULL
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue1);
		argValueList.insertTail (argValue2);
		argValueList.insertTail (argValue3);
		return callOperator (opValue, &argValueList, resultValue);
	}

	bool
	callOperator (
		const Value& opValue,
		const Value& argValue1,
		const Value& argValue2,
		const Value& argValue3,
		const Value& argValue4,
		Value* resultValue = NULL
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue1);
		argValueList.insertTail (argValue2);
		argValueList.insertTail (argValue3);
		argValueList.insertTail (argValue4);
		return callOperator (opValue, &argValueList, resultValue);
	}

	void
	gcSafePoint ();

	void
	checkStackOverflow ();

	void
	checkDivByZero (const Value& value);

	// closure operators

	Type*
	getClosureOperatorResultType (
		const Value& opValue,
		sl::BoxList <Value>* argValueList
		);

	bool
	getClosureOperatorResultType (
		const Value& opValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	getClosureOperatorResultType (
		Value* value,
		sl::BoxList <Value>* argValueList
		)
	{
		return getClosureOperatorResultType (*value,  argValueList, value);
	}

	bool
	closureOperator (
		const Value& opValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	closureOperator (
		Value* value,
		sl::BoxList <Value>* argValueList
		)
	{
		return closureOperator (*value,  argValueList, value);
	}

	bool
	closureOperator (
		const Value& opValue,
		Value* resultValue
		)
	{
		sl::BoxList <Value> argValueList;
		return closureOperator (opValue, &argValueList, resultValue);
	}

	bool
	closureOperator (
		const Value& opValue,
		const Value& argValue,
		Value* resultValue
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue);
		return closureOperator (opValue, &argValueList, resultValue);
	}

	bool
	closureOperator2 (
		const Value& opValue,
		const Value& argValue1,
		const Value& argValue2,
		Value* resultValue
		)
	{
		sl::BoxList <Value> argValueList;
		argValueList.insertTail (argValue1);
		argValueList.insertTail (argValue2);
		return closureOperator (opValue, &argValueList, resultValue);
	}

	Type*
	getFunctionType (
		const Value& opValue,
		FunctionType* functionType
		);

	// property getter

	Type*
	getPropertyGetterType (const Value& opValue);

	bool
	getPropertyGetterType (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyGetterType (Value* value)
	{
		return getPropertyGetterType (*value, value);
	}

	bool
	getPropertyGetter (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyGetter (Value* value)
	{
		return getPropertyGetter (*value, value);
	}

	// property setter

	Type*
	getPropertySetterType (
		const Value& opValue,
		const Value& argValue
		);

	bool
	getPropertySetterType (
		const Value& opValue,
		const Value& argValue,
		Value* resultValue
		);

	bool
	getPropertySetterType (
		Value* value,
		const Value& argValue
		)
	{
		return getPropertySetterType (*value, argValue, value);
	}

	bool
	getPropertySetter (
		const Value& opValue,
		const Value& argValue,
		Value* resultValue
		);

	bool
	getPropertySetter (
		Value* value,
		const Value& argValue
		)
	{
		return getPropertySetter (*value, argValue, value);
	}

	Type*
	getPropertySetterType (const Value& opValue)
	{
		return getPropertySetterType (opValue, Value ());
	}

	bool
	getPropertySetterType (
		const Value& opValue,
		Value* resultValue
		)
	{
		return getPropertySetterType (opValue, Value (), resultValue);
	}

	bool
	getPropertySetterType (Value* value)
	{
		return getPropertySetterType (*value, Value (), value);
	}

	bool
	getPropertySetter (
		const Value& opValue,
		Value* resultValue
		)
	{
		return getPropertySetter (opValue, Value (), resultValue);
	}

	bool
	getPropertySetter (Value* value)
	{
		return getPropertySetter (*value, Value (), value);
	}

	// property binder

	Type*
	getPropertyBinderType (const Value& opValue);

	bool
	getPropertyBinderType (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyBinderType (Value* value)
	{
		return getPropertyBinderType (*value, value);
	}

	bool
	getPropertyBinder (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyBinder (Value* value)
	{
		return getPropertyBinder (*value, value);
	}

	// autoget & onchanged

	Type*
	getPropertyAutoGetValueType (const Value& opValue);

	bool
	getPropertyAutoGetValueType (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyAutoGetValueType (Value* value)
	{
		return getPropertyAutoGetValueType (*value, value);
	}

	bool
	getPropertyAutoGetValue (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyAutoGetValue (Value* value)
	{
		return getPropertyAutoGetValue (*value, value);
	}

	Type*
	getPropertyOnChangedType (const Value& opValue);

	bool
	getPropertyOnChangedType (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyOnChangedType (Value* value)
	{
		return getPropertyOnChangedType (*value, value);
	}

	bool
	getPropertyOnChanged (
		const Value& opValue,
		Value* resultValue
		);

	bool
	getPropertyOnChanged (Value* value)
	{
		return getPropertyOnChanged (*value, value);
	}

	// misc property functions

	bool
	getProperty (
		const Value& opValue,
		Value* resultValue
		);

	bool
	setProperty (
		const Value& opValue,
		const Value& srcValue
		);

	bool
	getPropertyThinPtr (
		Property* prop,
		Closure* closure,
		PropertyPtrType* ptrType,
		Value* resultValue
		);

	bool
	getPropertyThinPtr (
		Property* prop,
		Closure* closure,
		Value* resultValue
		)
	{
		return getPropertyThinPtr (
			prop,
			closure,
			prop->getType ()->getPropertyPtrType (PropertyPtrTypeKind_Thin),
			resultValue
			);
	}

	bool
	getPropertyVTable (
		Property* prop,
		Closure* closure,
		Value* resultValue
		);

	bool
	getPropertyVTable (
		const Value& opValue,
		Value* resultValue
		);

	// load & store operators

	bool
	loadDataRef (
		const Value& opValue,
		Value* resultValue
		);

	bool
	loadDataRef (Value* value)
	{
		return loadDataRef (*value, value);
	}

	bool
	storeDataRef (
		const Value& dstValue,
		const Value& srcValue
		);

	// fields

	bool
	getField (
		const Value& opValue,
		StructField* member,
		MemberCoord* coord,
		Value* resultValue
		);

	bool
	getField (
		const Value& opValue,
		StructField* member,
		Value* resultValue
		)
	{
		return getField (opValue, member, NULL, resultValue);
	}

	bool
	getField (
		Value* value,
		StructField* member
		)
	{
		return getField (*value, member, NULL, value);
	}

	// impl

	bool
	getFieldPtrImpl (
		const Value& opValue,
		MemberCoord* coord,
		Type* resultType,
		Value* resultValue
		);

	bool
	getStructField (
		const Value& opValue,
		StructField* field,
		MemberCoord* coord,
		Value* resultValue
		);

	bool
	getDynamicStructField (
		const Value& opValue,
		DerivableType* type,
		StructField* field,
		Value* resultValue
		);

	bool
	getUnionField (
		const Value& opValue,
		StructField* field,
		Value* resultValue
		);

	bool
	getClassField (
		const Value& opValue,
		StructField* field,
		MemberCoord* coord,
		Value* resultValue
		);

	bool
	getPropertyField (
		const Value& opValue,
		ModuleItem* member,
		Value* resultValue
		);

	// misc

	bool
	getVirtualMethod (
		Function* function,
		Closure* closure,
		Value* resultValue
		);

	bool
	getVirtualProperty (
		Property* prop,
		Closure* closure,
		Value* resultValue
		);

	// closures

	bool
	createClosureObject (
		const Value& opValue, // thin function or property ptr with closure
		Type* thunkType, // function or property type
		bool isWeak,
		Value* resultValue
		);

	bool
	createDataClosureObject (
		const Value& opValue, // data ptr
		PropertyType* thunkType, // function or property type
		Value* resultValue
		);

	// pointer checks

	void
	checkPtr (
		StdFunc stdCheckFunction,
		StdFunc stdTryCheckFunction,
		const Value* argValueArray,
		size_t argCount
		);

	bool
	checkDataPtrRange (const Value& value);

	void
	checkNullPtr (const Value& value);

	// access check

	bool
	checkAccess (ModuleItemDecl* decl);

protected:
	// overloaded operators

	Function*
	getOverloadedUnaryOperator (
		UnOpKind opKind,
		const Value& opValue
		);

	Function*
	getOverloadedBinaryOperator (
		BinOpKind opKind,
		const Value& opValue1
		);

	// bit fields

	bool
	extractBitField (
		const Value& value,
		BitFieldType* bitFieldType,
		Value* resultValue
		);

	bool
	mergeBitField (
		const Value& value,
		const Value& shadowValue,
		BitFieldType* bitFieldType,
		Value* resultValue
		);

	// member operators

	bool
	getNamespaceMemberType (
		Namespace* nspace,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getNamespaceMember (
		Namespace* nspace,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getNamedTypeMemberType (
		const Value& opValue,
		NamedType* namedType,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getNamedTypeMember (
		const Value& opValue,
		NamedType* namedType,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getEnumTypeMemberType (
		const Value& opValue,
		EnumType* enumType,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getEnumTypeMember (
		const Value& opValue,
		EnumType* enumType,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getVariantMember (
		const Value& opValue,
		size_t index,
		Value* resultValue
		);

	bool
	getVariantMember (
		const Value& opValue,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	getLibraryMember (
		DynamicLibNamespace* library,
		Closure* closure,
		const sl::StringRef& name,
		Value* resultValue
		);

	bool
	finalizeMemberOperator (
		const Value& opValue,
		ModuleItemDecl* decl,
		Value* resultValue
		);

	bool
	getClassVTable (
		const Value& opValue,
		ClassType* classType,
		Value* resultValue
		);

	bool
	callImpl (
		const Value& pfnValue,
		FunctionType* functionType,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	callClosureFunctionPtr (
		const Value& opValue,
		sl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	bool
	castArgValueList (
		FunctionType* functionType,
		Closure* closure,
		sl::BoxList <Value>* argValueList
		);

	Type*
	getCdeclVarArgType (Type* type);

	bool
	callOperatorVararg (
		Function* operatorVararg,
		DerivableType* type,
		const Value& value,
		Value* resultValue
		);

	bool
	callOperatorVararg (
		Function* operatorVararg,
		DerivableType* type,
		Value* value
		)
	{
		return callOperatorVararg (operatorVararg, type, *value, value);
	}

	bool
	deleteDataPtr (const Value& opValue);

	bool
	deleteClassPtr (const Value& opValue);

	bool
	dynamicCastDataPtr (
		const Value& opValue,
		DataPtrType* type,
		Value* resultValue
		);

	bool
	dynamicCastClassPtr (
		const Value& opValue,
		ClassPtrType* type,
		Value* resultValue
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
