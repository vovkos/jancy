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

#include "pch.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

OperatorMgr::OperatorMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	// operator tables

	memset(m_unaryOperatorTable, 0, sizeof(m_unaryOperatorTable));
	memset(m_binaryOperatorTable, 0, sizeof(m_binaryOperatorTable));
	memset(m_castOperatorTable, 0, sizeof(m_castOperatorTable));

	// unary arithmetics

	m_unaryOperatorTable[UnOpKind_Plus]     = &m_unOp_Plus;
	m_unaryOperatorTable[UnOpKind_Minus]    = &m_unOp_Minus;
	m_unaryOperatorTable[UnOpKind_BwNot]    = &m_unOp_BwNot;
	m_unaryOperatorTable[UnOpKind_LogNot]   = &m_unOp_LogNot;

	// pointer operators

	m_unaryOperatorTable[UnOpKind_Addr]     = &m_unOp_Addr;
	m_unaryOperatorTable[UnOpKind_Indir]    = &m_unOp_Indir;
	m_unaryOperatorTable[UnOpKind_Ptr]      = &m_unOp_Ptr;

	// increment operators

	m_unOp_PreInc.m_opKind  = UnOpKind_PreInc;
	m_unOp_PreDec.m_opKind  = UnOpKind_PreDec;
	m_unOp_PostInc.m_opKind = UnOpKind_PostInc;
	m_unOp_PostDec.m_opKind = UnOpKind_PostDec;

	m_unaryOperatorTable[UnOpKind_PreInc]   = &m_unOp_PreInc;
	m_unaryOperatorTable[UnOpKind_PreDec]   = &m_unOp_PreDec;
	m_unaryOperatorTable[UnOpKind_PostInc]  = &m_unOp_PostInc;
	m_unaryOperatorTable[UnOpKind_PostDec]  = &m_unOp_PostDec;

	// binary arithmetics

	m_binaryOperatorTable[BinOpKind_Add]    = &m_binOp_Add;
	m_binaryOperatorTable[BinOpKind_Sub]    = &m_binOp_Sub;
	m_binaryOperatorTable[BinOpKind_Mul]    = &m_binOp_Mul;
	m_binaryOperatorTable[BinOpKind_Div]    = &m_binOp_Div;
	m_binaryOperatorTable[BinOpKind_Mod]    = &m_binOp_Mod;
	m_binaryOperatorTable[BinOpKind_Shl]    = &m_binOp_Shl;
	m_binaryOperatorTable[BinOpKind_Shr]    = &m_binOp_Shr;
	m_binaryOperatorTable[BinOpKind_BwAnd]  = &m_binOp_BwAnd;
	m_binaryOperatorTable[BinOpKind_BwXor]  = &m_binOp_BwXor;
	m_binaryOperatorTable[BinOpKind_BwOr]   = &m_binOp_BwOr;
	m_binaryOperatorTable[BinOpKind_At]     = &m_binOp_At;

	// binary logic operators

	m_binaryOperatorTable[BinOpKind_LogAnd] = &m_binOp_LogAnd;
	m_binaryOperatorTable[BinOpKind_LogOr]  = &m_binOp_LogOr;

	// comparison operators

	m_binaryOperatorTable[BinOpKind_Eq]     = &m_binOp_Eq;
	m_binaryOperatorTable[BinOpKind_Ne]     = &m_binOp_Ne;
	m_binaryOperatorTable[BinOpKind_Lt]     = &m_binOp_Lt;
	m_binaryOperatorTable[BinOpKind_Le]     = &m_binOp_Le;
	m_binaryOperatorTable[BinOpKind_Gt]     = &m_binOp_Gt;
	m_binaryOperatorTable[BinOpKind_Ge]     = &m_binOp_Ge;

	// indexing operator

	m_binaryOperatorTable[BinOpKind_Idx]    = &m_binOp_Idx;

	// assignment operators

	m_binOp_AddAssign.m_opKind = BinOpKind_AddAssign;
	m_binOp_SubAssign.m_opKind = BinOpKind_SubAssign;
	m_binOp_MulAssign.m_opKind = BinOpKind_MulAssign;
	m_binOp_DivAssign.m_opKind = BinOpKind_DivAssign;
	m_binOp_ModAssign.m_opKind = BinOpKind_ModAssign;
	m_binOp_ShlAssign.m_opKind = BinOpKind_ShlAssign;
	m_binOp_ShrAssign.m_opKind = BinOpKind_ShrAssign;
	m_binOp_AndAssign.m_opKind = BinOpKind_AndAssign;
	m_binOp_XorAssign.m_opKind = BinOpKind_XorAssign;
	m_binOp_OrAssign.m_opKind  = BinOpKind_OrAssign;
	m_binOp_AtAssign.m_opKind  = BinOpKind_AtAssign;

	m_binaryOperatorTable[BinOpKind_Assign]    = &m_binOp_Assign;
	m_binaryOperatorTable[BinOpKind_RefAssign] = &m_binOp_RefAssign;
	m_binaryOperatorTable[BinOpKind_AddAssign] = &m_binOp_AddAssign;
	m_binaryOperatorTable[BinOpKind_SubAssign] = &m_binOp_SubAssign;
	m_binaryOperatorTable[BinOpKind_MulAssign] = &m_binOp_MulAssign;
	m_binaryOperatorTable[BinOpKind_DivAssign] = &m_binOp_DivAssign;
	m_binaryOperatorTable[BinOpKind_ModAssign] = &m_binOp_ModAssign;
	m_binaryOperatorTable[BinOpKind_ShlAssign] = &m_binOp_ShlAssign;
	m_binaryOperatorTable[BinOpKind_ShrAssign] = &m_binOp_ShrAssign;
	m_binaryOperatorTable[BinOpKind_AndAssign] = &m_binOp_AndAssign;
	m_binaryOperatorTable[BinOpKind_XorAssign] = &m_binOp_XorAssign;
	m_binaryOperatorTable[BinOpKind_OrAssign]  = &m_binOp_OrAssign;
	m_binaryOperatorTable[BinOpKind_AtAssign]  = &m_binOp_AtAssign;

	// cast operators

	m_stdCastOperatorTable[StdCast_Copy] = &m_cast_Copy;
	m_stdCastOperatorTable[StdCast_SwapByteOrder] = &m_cast_SwapByteOrder;
	m_stdCastOperatorTable[StdCast_PtrFromInt] = &m_cast_PtrFromInt;
	m_stdCastOperatorTable[StdCast_Int] = &m_cast_Int;
	m_stdCastOperatorTable[StdCast_BeInt] = &m_cast_BeInt;
	m_stdCastOperatorTable[StdCast_Fp] = &m_cast_Fp;
	m_stdCastOperatorTable[StdCast_FromVariant] = &m_cast_FromVariant;

	for (size_t i = 0; i < TypeKind__Count; i++)
		m_castOperatorTable[i] = &m_cast_Default;

	m_castOperatorTable[TypeKind_Bool] = &m_cast_Bool;

	for (size_t i = TypeKind_Int8; i <= TypeKind_Int64_u; i++)
		m_castOperatorTable[i] = &m_cast_Int;

	for (size_t i = TypeKind_Int16_be; i <= TypeKind_Int64_beu; i++)
		m_castOperatorTable[i] = &m_cast_BeInt;

	m_castOperatorTable[TypeKind_Void]        = &m_cast_Void;
	m_castOperatorTable[TypeKind_Float]       = &m_cast_Fp;
	m_castOperatorTable[TypeKind_Double]      = &m_cast_Fp;
	m_castOperatorTable[TypeKind_Variant]     = &m_cast_Variant;
	m_castOperatorTable[TypeKind_Array]       = &m_cast_Array;
	m_castOperatorTable[TypeKind_Enum]        = &m_cast_Enum;
	m_castOperatorTable[TypeKind_Struct]      = &m_cast_Struct;
	m_castOperatorTable[TypeKind_DataPtr]     = &m_cast_DataPtr;
	m_castOperatorTable[TypeKind_DataRef]     = &m_cast_DataRef;
	m_castOperatorTable[TypeKind_ClassPtr]    = &m_cast_ClassPtr;
	m_castOperatorTable[TypeKind_FunctionPtr] = &m_cast_FunctionPtr;
	m_castOperatorTable[TypeKind_FunctionRef] = &m_cast_FunctionRef;
	m_castOperatorTable[TypeKind_PropertyPtr] = &m_cast_PropertyPtr;
	m_castOperatorTable[TypeKind_PropertyRef] = &m_cast_PropertyRef;
	m_castOperatorTable[TypeKind_TypedefShadow] = &m_cast_Typedef;

	m_unsafeEnterCount = 0;
}

void
OperatorMgr::clear()
{
	m_unsafeEnterCount = 0;
}

OverloadableFunction
OperatorMgr::getOverloadedUnaryOperator(
	UnOpKind opKind,
	const Value& opValue
	)
{
	Value opTypeValue;
	bool result = prepareOperandType(opValue, &opTypeValue);
	if (!result)
		return OverloadableFunction();

	Type* opType = opTypeValue.getType();
	if (opType->getTypeKind() == TypeKind_ClassPtr)
	{
		ClassPtrType* ptrType = (ClassPtrType*)opType;
		return ptrType->getTargetType()->getUnaryOperator(opKind);
	}
	else if (opType->getTypeKindFlags() & TypeKindFlag_Derivable)
	{
		DerivableType* derivableType = (DerivableType*)opType;
		return derivableType->getUnaryOperator(opKind);
	}

	return OverloadableFunction();
}

bool
OperatorMgr::unaryOperator(
	UnOpKind opKind,
	const Value& rawOpValue,
	Value* resultValue
	)
{
	ASSERT((size_t)opKind < UnOpKind__Count);

	OverloadableFunction function = getOverloadedUnaryOperator(opKind, rawOpValue);
	if (function)
	{
		sl::BoxList<Value> argList;
		argList.insertTail(rawOpValue);
		return callOperator(function, &argList, resultValue);
	}

	Value opValue;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	UnaryOperator* op = m_unaryOperatorTable[opKind];
	ASSERT(op);

	bool result = prepareOperand(rawOpValue, &opValue, op->getOpFlags());
	if (!result)
		return false;

	if (opValue.getType()->getTypeKind() == TypeKind_Variant && opKind <= UnOpKind_Indir)
	{
		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_VariantUnaryOperator);
		Value opKindValue(opKind, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int));
		return callOperator(function, opKindValue, opValue, resultValue);
	}

	return op->op(opValue, resultValue);
}

OverloadableFunction
OperatorMgr::getOverloadedBinaryOperator(
	BinOpKind opKind,
	const Value& opValue
	)
{
	Value opTypeValue;
	bool result = prepareOperandType(opValue, &opTypeValue);
	if (!result)
		return OverloadableFunction();

	Type* opType = opTypeValue.getType();
	if (opType->getTypeKind() == TypeKind_ClassPtr)
	{
		ClassPtrType* ptrType = (ClassPtrType*)opType;
		return ptrType->getTargetType()->getBinaryOperator(opKind);
	}
	else if (opType->getTypeKindFlags() & TypeKindFlag_Derivable)
	{
		DerivableType* derivableType = (DerivableType*)opType;
		return derivableType->getBinaryOperator(opKind);
	}

	return OverloadableFunction();
}

bool
OperatorMgr::binaryOperator(
	BinOpKind opKind,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	ASSERT((size_t)opKind < BinOpKind__Count);

	bool result;

	OverloadableFunction function = getOverloadedBinaryOperator(opKind, rawOpValue1);
	if (function)
	{
		if (function->getFlags() & MulticastMethodFlag_InaccessibleViaEventPtr)
		{
			Value opValue1;
			result = prepareOperandType(rawOpValue1, &opValue1);
			if (!result)
				return false;

			if (opValue1.getType()->getTypeKind() == TypeKind_ClassPtr &&
				(opValue1.getType()->getFlags() & PtrTypeFlag_Event))
			{
				err::setError("operator is inaccessible via 'event' pointer");
				return false;
			}
		}

		sl::BoxList<Value> argList;
		argList.insertTail(rawOpValue1);
		argList.insertTail(rawOpValue2);
		return callOperator(function, &argList, resultValue);
	}

	Value opValue1;
	Value opValue2;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	BinaryOperator* op = m_binaryOperatorTable[opKind];
	ASSERT(op);

	result =
		prepareOperand(rawOpValue1, &opValue1, op->getOpFlags1()) &&
		prepareOperand(rawOpValue2, &opValue2, op->getOpFlags2());

	if (!result)
		return false;

	if (opKind <= BinOpKind_Ge &&
		(opValue1.getType()->getTypeKind() == TypeKind_Variant ||
		opValue2.getType()->getTypeKind() == TypeKind_Variant))
	{
		StdFunc stdFunc = opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge ?
			StdFunc_VariantRelationalOperator :
			StdFunc_VariantBinaryOperator;

		Function* function = m_module->m_functionMgr.getStdFunction(stdFunc);
		Value opKindValue(opKind, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int));
		return callOperator(function, opKindValue, opValue1, opValue2, resultValue);
	}

	return op->op(opValue1, opValue2, resultValue);
}

Type*
getConditionalNumericOperatorResultType(
	const Value& trueValue,
	Type* trueType,
	const Value& falseValue,
	Type* falseType
	)
{
	if (trueType->getTypeKind() == TypeKind_Enum &&
		(trueType->getFlags() & EnumTypeFlag_BitFlag) &&
		falseValue.isZero())
		return trueType;

	if (falseType->getTypeKind() == TypeKind_Enum &&
		(falseType->getFlags() & EnumTypeFlag_BitFlag) &&
		trueValue.isZero())
		return falseType;

	return getArithmeticOperatorResultType(trueType, falseType);
}

Type*
OperatorMgr::getConditionalOperatorResultType(
	const Value& trueValue,
	const Value& falseValue
	)
{
	bool result;

	Type* resultType;
	Type* trueType = trueValue.getClosureAwareType();
	Type* falseType = falseValue.getClosureAwareType();

	if (trueType->getTypeKind() == TypeKind_Array)
		trueType = ((ArrayType*)trueType)->getElementType()->getDataPtrType(
			DataPtrTypeKind_Normal,
			trueValue.getValueKind() == ValueKind_Const ? PtrTypeFlag_Const : 0
			);

	if (falseType->getTypeKind() == TypeKind_Array)
		falseType = ((ArrayType*)falseType)->getElementType()->getDataPtrType(
			DataPtrTypeKind_Normal,
			falseValue.getValueKind() == ValueKind_Const ? PtrTypeFlag_Const : 0
			);

	if (trueType->cmp(falseType) == 0)
	{
		resultType = trueType;
	}
	else
	{
		uint_t trueFlags = OpFlag_KeepBool | OpFlag_KeepEnum;
		uint_t falseFlags = OpFlag_KeepBool | OpFlag_KeepEnum;

		if (isArrayRefType(trueType))
			trueFlags |= OpFlag_ArrayRefToPtr;

		if (isArrayRefType(falseType))
			falseFlags |= OpFlag_ArrayRefToPtr;

		Value trueTypeValue;
		Value falseTypeValue;

		result =
			prepareOperandType(trueType, &trueTypeValue, trueFlags) &&
			prepareOperandType(falseType, &falseTypeValue, falseFlags);

		if (!result)
			return NULL;

		trueType = trueTypeValue.getType();
		falseType = falseTypeValue.getType();

		resultType =
			trueType->cmp(falseType) == 0 ? trueType :
			(trueType->getTypeKindFlags() & falseType->getTypeKindFlags() & TypeKindFlag_Numeric) ?
				getConditionalNumericOperatorResultType(trueValue, trueType, falseValue, falseType) :
				(trueType->getTypeKindFlags() & falseType->getTypeKindFlags() & (TypeKindFlag_DataPtr | TypeKindFlag_ClassPtr)) &&
				!(trueType->getFlags() & PtrTypeFlag_Const) &&
				(falseType->getFlags() & PtrTypeFlag_Const) ?
					falseType :
					trueType;
	}

	// if it's a lean data pointer, fatten it

	if ((resultType->getTypeKindFlags() & TypeKindFlag_DataPtr) &&
		((DataPtrType*)resultType)->getPtrTypeKind() == DataPtrTypeKind_Lean)
	{
		resultType = ((DataPtrType*)resultType)->getTargetType()->getDataPtrType(
			resultType->getTypeKind(),
			DataPtrTypeKind_Normal,
			resultType->getFlags()
			);
	}

	result =
		checkCastKind(trueValue, resultType) &&
		checkCastKind(falseValue, resultType);

	return result ? resultType : NULL;
}

bool
OperatorMgr::conditionalOperator(
	const Value& rawTrueValue,
	const Value& rawFalseValue,
	BasicBlock* thenBlock,
	BasicBlock* phiBlock,
	Value* resultValue
	)
{
	bool result;

	Value trueValue;
	Value falseValue;

	Type* resultType = getConditionalOperatorResultType(rawTrueValue, rawFalseValue);
	if (!resultType)
		return false;

	if (resultType->getTypeKind() != TypeKind_Void)
	{
		result = castOperator(rawFalseValue, resultType, &falseValue);
		if (!result)
			return false;
	}

	BasicBlock* elseBlock = m_module->m_controlFlowMgr.getCurrentBlock(); // might have changed

	m_module->m_controlFlowMgr.jump(phiBlock, thenBlock);

	if (resultType->getTypeKind() != TypeKind_Void)
	{
		result = castOperator(rawTrueValue, resultType, &trueValue);
		if (!result)
			return false;
	}

	thenBlock = m_module->m_controlFlowMgr.getCurrentBlock(); // might have changed

	m_module->m_controlFlowMgr.follow(phiBlock);

	if (resultType->getTypeKind() == TypeKind_Void)
		resultValue->setVoid(m_module);
	else if (!m_module->hasCodeGen())
		resultValue->setType(resultType);
	else
		m_module->m_llvmIrBuilder.createPhi(trueValue, thenBlock, falseValue, elseBlock, resultValue);

	return true;
}

void
OperatorMgr::forceCast(
	const Value& value,
	Type* dstType,
	Value* resultValue
	)
{
	Type* srcType = value.getType();

	if (srcType->getSize() >= dstType->getSize())
	{
		Value tmpValue;
		m_module->m_llvmIrBuilder.createAlloca(srcType, "tmp", NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createStore(value, tmpValue);
		m_module->m_llvmIrBuilder.createBitCast(tmpValue, dstType->getDataPtrType_c(), &tmpValue);
		m_module->m_llvmIrBuilder.createLoad(tmpValue, dstType, resultValue);
	}
	else
	{
		Value tmpValue, tmpValue2;
		m_module->m_llvmIrBuilder.createAlloca(dstType, "tmp", NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createBitCast(tmpValue, srcType->getDataPtrType_c(), &tmpValue2);
		m_module->m_llvmIrBuilder.createStore(value, tmpValue2);
		m_module->m_llvmIrBuilder.createLoad(tmpValue, dstType, resultValue);
	}
}

bool
OperatorMgr::castOperator(
	OperatorDynamism dynamism,
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	bool result = type->ensureLayout();
	if (!result)
		return false;

	if (!m_module->hasCodeGen() && rawOpValue.getValueKind() != ValueKind_Const)
	{
		resultValue->setType(type);
		return true;
	}

	if (rawOpValue.getValueKind() == ValueKind_Null)
	{
		if ((type->getTypeKindFlags() & TypeKindFlag_Ptr) && (type->getFlags() & PtrTypeFlag_Safe))
		{
			setCastError(rawOpValue, type);
			return false;
		}

		if (type->getTypeKind() == TypeKind_Void)
			resultValue->setNull(m_module);
		else
			*resultValue = type->getZeroValue();

		return true;
	}

	TypeKind typeKind = type->getTypeKind();
	ASSERT((size_t)typeKind < TypeKind__Count);

	CastOperator* op = m_castOperatorTable[typeKind];
	ASSERT(op); // there is always a default

	Value opValue;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	result = prepareOperand(rawOpValue, &opValue, op->getOpFlags());
	if (!result)
		return false;

	Type* opType = opValue.getType();
	if (opType->cmp(type) == 0) // identity, try to shortcut
	{
		if (opValue.hasLlvmValue())
		{
			*resultValue = opValue;
			return true;
		}

		if (opValue.getValueKind() == ValueKind_Property)
		{
			ASSERT(type->getTypeKind() == TypeKind_PropertyPtr);
			return getPropertyThinPtr(opValue.getProperty(), opValue.getClosure(), (PropertyPtrType*)type, resultValue);
		}

		// nope, need to go through full cast
	}

	if (opType->getTypeKind() == TypeKind_Variant)
		return m_stdCastOperatorTable[StdCast_FromVariant]->cast(opValue, type, resultValue);

	if (dynamism != OperatorDynamism_Dynamic)
		return op->cast(opValue, type, resultValue);

	typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		return dynamicCastDataPtr(opValue, (DataPtrType*)type, resultValue);

	case TypeKind_ClassPtr:
		return dynamicCastClassPtr(opValue, (ClassPtrType*)type, resultValue);

	default:
		err::setFormatStringError("cannot dynamically cast to '%s'", type->getTypeString().sz());
		return false;
	}
}

bool
OperatorMgr::castOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	TypeKind typeKind,
	Value* resultValue
	)
{
	Type* type = m_module->m_typeMgr.getPrimitiveType(typeKind);
	return castOperator(dynamism, opValue, type, resultValue);
}

bool
OperatorMgr::dynamicCastDataPtr(
	const Value& opValue,
	DataPtrType* type,
	Value* resultValue
	)
{
	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr))
	{
		err::setFormatStringError(
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType()->getTypeString().sz(),
			type->getTypeString().sz()
			);
		return false;
	}

	if ((opValue.getType()->getFlags() & PtrTypeFlag_Const) &&
		!(type->getFlags() & PtrTypeFlag_Const))
	{
		setCastError(opValue, type);
		return false;
	}

	Value ptrValue;
	bool result = castOperator(
		opValue,
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Void)->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const),
		&ptrValue
		);

	if (!result)
		return false;

	Type* targetType = type->getTargetType();
	Value typeValue(&targetType, m_module->m_typeMgr.getStdType(StdType_BytePtr));

	Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicCastDataPtr);

	result = callOperator(function, ptrValue, typeValue, resultValue);
	if (!result)
		return false;

	resultValue->overrideType(type);
	return true;
}

bool
OperatorMgr::dynamicCastClassPtr(
	const Value& opValue,
	ClassPtrType* type,
	Value* resultValue
	)
{
	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr))
	{
		err::setFormatStringError(
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType()->getTypeString().sz(),
			type->getTypeString().sz()
			);
		return false;
	}

	if ((opValue.getType()->getFlags() & PtrTypeFlag_Const) &&
		!(type->getFlags() & PtrTypeFlag_Const))
	{
		setCastError(opValue, type);
		return false;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast(opValue, m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr), &ptrValue);

	Type* targetType = type->getTargetType();
	Value typeValue(&targetType, m_module->m_typeMgr.getStdType(StdType_BytePtr));

	Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicCastClassPtr);
	m_module->m_llvmIrBuilder.createCall2(
		function,
		function->getType(),
		ptrValue,
		typeValue,
		&ptrValue
		);

	m_module->m_llvmIrBuilder.createBitCast(ptrValue, type, resultValue);
	return true;
}

CastKind
OperatorMgr::getCastKind(
	const Value& rawOpValue,
	Type* type
	)
{
	if (rawOpValue.getValueKind() == ValueKind_Null)
		return (type->getTypeKindFlags() & TypeKindFlag_Nullable) ? CastKind_Implicit : CastKind_None;

	TypeKind typeKind = type->getTypeKind();
	ASSERT((size_t)typeKind < TypeKind__Count);

	CastOperator* op = m_castOperatorTable[typeKind];
	ASSERT(op); // there is always a default

	Value opValue;
	bool result = prepareOperandType(
		rawOpValue,
		&opValue,
		op->getOpFlags()
		);

	if (!result)
		return CastKind_None;

	Type* opType = opValue.getType();
	return
		opType->cmp(type) == 0 ? CastKind_Identitiy :
		opType->getTypeKind() == TypeKind_Variant ? CastKind_ImplicitCrossFamily :
		op->getCastKind(opValue, type);
}

CastKind
OperatorMgr::getArgCastKind(
	Closure* closure,
	FunctionType* functionType,
	FunctionArg* const* actualArgArray,
	size_t actualArgCount
	)
{
	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();

	if (closure)
	{
		bool result = closure->getArgTypeArray(m_module, &formalArgArray);
		if (!result)
			return CastKind_None;
	}

	size_t formalArgCount = formalArgArray.getCount();

	if (actualArgCount > formalArgCount && !(functionType->getFlags() & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray[argCount - 1]->getInitializer().isEmpty())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	for (size_t i = 0; i < argCount; i++)
	{
		Type* formalArgType = formalArgArray[i]->getType();
		Type* actualArgType = actualArgArray[i]->getType();

		CastKind castKind = getCastKind(actualArgType, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getArgCastKind(
	FunctionType* functionType,
	const Value* argValueArray,
	size_t actualArgCount
	)
{
	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();
	size_t formalArgCount = formalArgArray.getCount();

	if (actualArgCount > formalArgCount && !(functionType->getFlags() & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray[argCount - 1]->getInitializer().isEmpty())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	for (size_t i = 0; i < argCount; i++)
	{
		Type* formalArgType = formalArgArray[i]->getType();
		Type* actualArgType = argValueArray[i].getType();

		if (!actualArgType)
			return formalArgArray[i]->getInitializer().isEmpty() ? CastKind_None : CastKind_Identitiy;

		CastKind castKind = getCastKind(actualArgType, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getArgCastKind(
	FunctionType* functionType,
	const sl::ConstBoxList<Value>& argList
	)
{
	size_t actualArgCount = argList.getCount();

	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();
	size_t formalArgCount = formalArgArray.getCount();

	if (actualArgCount > formalArgCount && !(functionType->getFlags() & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray[argCount - 1]->getInitializer().isEmpty())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	sl::ConstBoxIterator<Value> arg = argList.getHead();
	for (size_t i = 0; i < argCount; i++, arg++)
	{
		Type* formalArgType = formalArgArray[i]->getType();

		if (arg->isEmpty())
			return formalArgArray[i]->getInitializer().isEmpty() ? CastKind_None : CastKind_Identitiy;

		CastKind castKind = getCastKind(*arg, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getFunctionCastKind(
	FunctionType* srcType,
	FunctionType* dstType
	)
{
	CastKind argCastKind = getArgCastKind(srcType, dstType->getArgArray());
	if (!argCastKind)
		return CastKind_None;

	Type* srcReturnType = srcType->getReturnType();
	Type* dstReturnType = dstType->getReturnType();

	if (dstReturnType->getTypeKind() == TypeKind_Void)
		return argCastKind;

	CastKind returnCastKind = getCastKind(srcReturnType, dstReturnType);
	return AXL_MIN(argCastKind, returnCastKind);
}

CastKind
OperatorMgr::getPropertyCastKind(
	PropertyType* srcType,
	PropertyType* dstType
	)
{
	CastKind castKind = getFunctionCastKind(srcType->getGetterType(), dstType->getGetterType());
	if (!castKind)
		return CastKind_None;

	FunctionTypeOverload* srcSetterType = srcType->getSetterType();
	FunctionTypeOverload* dstSetterType = dstType->getSetterType();

	CastKind worstCastKind = castKind;

	size_t count = dstSetterType->getOverloadCount();
	for (size_t i = 0; i < count; i++)
	{
		FunctionType* dstOverload = dstSetterType->getOverload(i);

		size_t j = srcSetterType->chooseOverload(dstOverload->getArgArray(), &castKind);
		if (j == -1)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

bool
OperatorMgr::checkCastKind(
	const Value& opValue,
	Type* type
	)
{
	CastKind castKind = getCastKind(opValue, type);
	if (castKind <= CastKind_Explicit)
	{
		setCastError(opValue, type, castKind);
		return false;
	}

	return true;
}

bool
OperatorMgr::sizeofOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
	)
{
	Value typeValue;
	bool result = prepareOperandType(opValue, &typeValue, OpFlag_LoadArrayRef);
	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic)
	{
		if (type->getFlags() & TypeFlag_Dynamic)
		{
			DynamicFieldValueInfo* fieldInfo = opValue.getDynamicFieldInfo();
			if (fieldInfo)
			{
				Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicFieldSizeOf);
				Value typeValue(&fieldInfo->m_parentType, m_module->m_typeMgr.getStdType(StdType_BytePtr));
				Value fieldValue(&fieldInfo->m_field, m_module->m_typeMgr.getStdType(StdType_BytePtr));
				return callOperator(function, fieldInfo->m_parentValue, typeValue, fieldValue, resultValue);
			}
			else
			{
				Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicTypeSizeOf);
				Value typeValue(&type, m_module->m_typeMgr.getStdType(StdType_BytePtr));
				return callOperator(function, opValue, typeValue, resultValue);
			}
		}

		type = typeValue.getType();
		if (type->getTypeKind() != TypeKind_DataPtr)
		{
			err::setFormatStringError("'dynamic sizeof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicSizeOf);
		return callOperator(function, opValue, resultValue);
	}

	if (type->getFlags() & TypeFlag_Dynamic)
	{
		err::setError("use 'dynamic sizeof' to get size of a dynamic type");
		return false;
	}

	resultValue->setConstSizeT(type->getSize(), m_module);
	return true;
}

bool
OperatorMgr::countofOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
	)
{
	Value typeValue;
	bool result = prepareOperandType(opValue, &typeValue, OpFlag_LoadArrayRef);
	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic)
	{
		if (type->getFlags() & TypeFlag_Dynamic)
		{
			DynamicFieldValueInfo* fieldInfo = opValue.getDynamicFieldInfo();
			if (!fieldInfo)
			{
				err::setError("invalid 'dynamic countof' operator");
				return false;
			}

			Type* fieldType = fieldInfo->m_field->getType();
			if (fieldType->getTypeKind() != TypeKind_Array)
			{
				err::setFormatStringError("'dynamic countof' operator is only applicable to arrays, not to '%s'", type->getTypeString().sz());
				return false;
			}

			Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicFieldCountOf);
			Value typeValue(&fieldInfo->m_parentType, m_module->m_typeMgr.getStdType(StdType_BytePtr));
			Value fieldValue(&fieldInfo->m_field, m_module->m_typeMgr.getStdType(StdType_BytePtr));
			return callOperator(function, fieldInfo->m_parentValue, typeValue, fieldValue, resultValue);
		}

		type = typeValue.getType();
		if (type->getTypeKind() != TypeKind_DataPtr)
		{
			err::setFormatStringError("'dynamic countof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

		type = ((DataPtrType*)type)->getTargetType();
		typeValue.createConst(&type, m_module->m_typeMgr.getStdType(StdType_BytePtr));
		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicCountOf);
		return callOperator(function, opValue, typeValue, resultValue);
	}

	if (type->getTypeKind() != TypeKind_Array)
	{
		err::setFormatStringError("'countof' operator is only applicable to arrays, not to '%s'", type->getTypeString().sz());
		return false;
	}

	if (type->getFlags() & TypeFlag_Dynamic)
	{
		err::setError("use 'dynamic countof' to get element count of a dynamic array");
		return false;
	}

	resultValue->setConstSizeT(((ArrayType*)type)->getElementCount(), m_module);
	return true;
}

bool
OperatorMgr::typeofOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
	)
{
	Value typeValue;

	bool result =
		prepareOperandType(opValue, &typeValue, OpFlag_KeepBool | OpFlag_KeepEnum | OpFlag_LoadArrayRef) &&
		m_module->ensureIntrospectionLibRequired();

	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic)
	{
		if (type->getTypeKind() != TypeKind_DataPtr)
		{
			err::setFormatStringError("'dynamic typeof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

/*		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicTypeOf);
		bool result = callOperator(function, opValue, resultValue);
		if (!result)
			return false; */

		err::setError("'dynamic typeof' operator is not yet implemented");
		return false;
	}

	resultValue->setVariable(type->getTypeVariable());
	return prepareOperand(resultValue); // turn it into a class pointer
}

bool
OperatorMgr::offsetofOperator(
	const Value& value,
	Value* resultValue
	)
{
	if (value.getValueKind() != ValueKind_Field)
	{
		err::setFormatStringError("'offsetof' can only be applied to fields");
		return false;
	}

	resultValue->setConstSizeT(value.getFieldOffset(), m_module);
	return true;
}

bool
OperatorMgr::prepareOperandType(
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
	)
{
	bool result;

	switch (opValue.getValueKind())
	{
	case ValueKind_Void:
		resultValue->setVoid(m_module); // ensure non-null type
		return true;

	case ValueKind_FunctionOverload:
	case ValueKind_FunctionTypeOverload:
		*resultValue = opValue;
		return true;

	case ValueKind_Field:
		resultValue->overrideType(opValue.getField()->getType());
		return true;
	}

	Value value = opValue;

	for (;;)
	{
		Type* type = value.getType();
		result = type->ensureLayout();
		if (!result)
			return false;

		TypeKind typeKind = type->getTypeKind();
		switch (typeKind)
		{
		case TypeKind_TypedefShadow:
			value.overrideType(((TypedefShadowType*)type)->getTypedef()->getType());
			break;

		case TypeKind_NamedImport:
		case TypeKind_ImportIntMod:
		case TypeKind_ImportPtr:
			value.overrideType(((ImportType*)type)->getActualType());
			break;

		case TypeKind_DataPtr:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((DataPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			break;

		case TypeKind_DataRef:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((DataPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			if (!(opFlags & OpFlag_KeepDataRef))
			{
				DataPtrType* ptrType = (DataPtrType*)type;
				Type* targetType = ptrType->getTargetType();
				TypeKind targetTypeKind = targetType->getTypeKind();

				if (targetTypeKind == TypeKind_BitField)
				{
					BitFieldType* bitFieldType = (BitFieldType*)targetType;
					value = bitFieldType->getBaseType();
				}
				else if (targetTypeKind != TypeKind_Array)
				{
					bool b1 = (targetType->getTypeKindFlags() & TypeKindFlag_Derivable) && (opFlags & OpFlag_KeepDerivableRef);
					bool b2 = targetTypeKind == TypeKind_Variant && (opFlags & OpFlag_KeepVariantRef);

					if (!b1 && !b2)
						value = ((DataPtrType*)type)->getTargetType();
				}
				else if (opFlags & OpFlag_LoadArrayRef)
				{
					value = ((DataPtrType*)type)->getTargetType();
				}
				else if (opFlags & OpFlag_ArrayRefToPtr)
				{
					ArrayType* arrayType = (ArrayType*)targetType;
					value = arrayType->getElementType()->getDataPtrType(
						TypeKind_DataPtr,
						ptrType->getPtrTypeKind(),
						ptrType->getFlags()
						);
				}
			}

			break;

		case TypeKind_ClassPtr:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((ClassPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			break;

		case TypeKind_ClassRef:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((ClassPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			if (!(opFlags & OpFlag_KeepClassRef))
			{
				ClassPtrType* ptrType = (ClassPtrType*)type;
				ClassType* targetType = ptrType->getTargetType();
				value.overrideType(targetType->getClassPtrType(
					TypeKind_ClassPtr,
					ptrType->getPtrTypeKind(),
					ptrType->getFlags()
					));
			}

			break;

		case TypeKind_FunctionRef:
			if (!(opFlags & OpFlag_KeepFunctionRef))
			{
				FunctionPtrType* ptrType = (FunctionPtrType*)value.getClosureAwareType(); // important: take closure into account!
				if (!ptrType)
					return false;

				FunctionType* targetType = ptrType->getTargetType();
				value = targetType->getFunctionPtrType(ptrType->getPtrTypeKind(), ptrType->getFlags());
			}

			break;

		case TypeKind_PropertyRef:
			if (!(opFlags & OpFlag_KeepPropertyRef))
			{
				PropertyPtrType* ptrType = (PropertyPtrType*)value.getClosureAwareType();
				if (!ptrType)
					return false;

				PropertyType* targetType = ptrType->getTargetType();
				if (!targetType->isIndexed())
					value = targetType->getReturnType();
			}

			break;

		case TypeKind_Bool:
			if (!(opFlags & OpFlag_KeepBool))
				value = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8);

			break;

		case TypeKind_Enum:
			if (!(opFlags & OpFlag_KeepEnum))
				value.overrideType(((EnumType*)type)->getRootType());

			break;
		}

		if (value.getType() == type)
			break;
	}

	*resultValue = value;
	return true;
}

void
OperatorMgr::prepareArrayRef(
	const Value& value,
	Value* resultValue
	)
{
	ASSERT(isArrayRefType(value.getType()));
	DataPtrType* ptrType = (DataPtrType*)value.getType();
	DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

	ArrayType* arrayType = (ArrayType*)ptrType->getTargetType();
	Type* elementType = arrayType->getElementType();
	DataPtrType* resultType = elementType->getDataPtrType(
		TypeKind_DataPtr,
		ptrTypeKind,
		ptrType->getFlags()
		);

	if (value.getValueKind() == ValueKind_Const || ptrTypeKind == DataPtrTypeKind_Normal)
	{
		resultValue->overrideType(value, resultType);
	}
	else if (ptrTypeKind != DataPtrTypeKind_Lean)
	{
		m_module->m_llvmIrBuilder.createGep2(value, 0, resultType, resultValue);
	}
	else
	{
		// get validator first (resultValue can point to value)
		LeanDataPtrValidator* validator = value.getLeanDataPtrValidator();
		m_module->m_llvmIrBuilder.createGep2(value, 0, resultType, resultValue);
		resultValue->setLeanDataPtrValidator(validator);
	}
}

bool
OperatorMgr::prepareOperand(
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
	)
{
	bool result;

	if (!m_module->hasCodeGen())
		return prepareOperandType(opValue, resultValue, opFlags);

	switch (opValue.getValueKind())
	{
	case ValueKind_Void:
		resultValue->setVoid(m_module); // ensure non-null type
		return true;

	case ValueKind_FunctionOverload:
	case ValueKind_FunctionTypeOverload:
		*resultValue = opValue;
		return true;
	}

	Value value = opValue;
	for (;;)
	{
		Type* type = value.getType();
		result = type->ensureLayout();
		if (!result)
			return false;

		TypeKind typeKind = type->getTypeKind();
		switch (typeKind)
		{
		case TypeKind_NamedImport:
		case TypeKind_ImportIntMod:
		case TypeKind_ImportPtr:
			value.overrideType(((ImportType*)type)->getActualType());
			break;

		case TypeKind_DataPtr:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((DataPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			break;

		case TypeKind_DataRef:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((DataPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			if (!(opFlags & OpFlag_KeepDataRef))
			{
				DataPtrType* ptrType = (DataPtrType*)type;
				Type* targetType = ptrType->getTargetType();
				TypeKind targetTypeKind = targetType->getTypeKind();

				if (targetTypeKind != TypeKind_Array)
				{
					bool b1 = (targetType->getTypeKindFlags() & TypeKindFlag_Derivable) && (opFlags & OpFlag_KeepDerivableRef);
					bool b2 = targetTypeKind == TypeKind_Variant && (opFlags & OpFlag_KeepVariantRef);

					if (!b1 && !b2)
					{
						result = loadDataRef(&value);
						if (!result)
							return false;
					}
				}
				else if (opFlags & OpFlag_LoadArrayRef)
				{
					result = loadDataRef(&value);
					if (!result)
						return false;
				}
				else if (opFlags & OpFlag_ArrayRefToPtr)
				{
					prepareArrayRef(&value);
				}
			}

			break;

		case TypeKind_ClassPtr:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((ClassPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			break;

		case TypeKind_ClassRef:
			if (opFlags & OpFlag_EnsurePtrTargetLayout)
			{
				result = ((ClassPtrType*)type)->getTargetType()->ensureLayout();
				if (!result)
					return false;
			}

			if (!(opFlags & OpFlag_KeepClassRef))
			{
				ClassPtrType* ptrType = (ClassPtrType*)type;
				ClassType* targetType = ptrType->getTargetType();
				value.overrideType(targetType->getClassPtrType(
					TypeKind_ClassPtr,
					ptrType->getPtrTypeKind(),
					ptrType->getFlags())
					);
			}

			break;

		case TypeKind_FunctionRef:
			if (!(opFlags & OpFlag_KeepFunctionRef))
			{
				FunctionPtrType* ptrType = (FunctionPtrType*)type;
				FunctionType* targetType = ptrType->getTargetType();
				value.overrideType(targetType->getFunctionPtrType(ptrType->getPtrTypeKind(), ptrType->getFlags()));
			}

			break;

		case TypeKind_PropertyRef:
			if (!(opFlags & OpFlag_KeepPropertyRef))
			{
				PropertyPtrType* ptrType = (PropertyPtrType*)value.getClosureAwareType();
				if (!ptrType)
					return false;

				PropertyType* targetType = ptrType->getTargetType();
				if (!targetType->isIndexed())
				{
					result = getProperty(value, &value);
					if (!result)
						return false;
				}
			}

			break;

		case TypeKind_Bool:
			if (!(opFlags & OpFlag_KeepBool))
			{
				result = m_castIntFromBool.cast(value, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8), &value);
				if (!result)
					return false;
			}

			break;

		case TypeKind_Enum:
			if (!(opFlags & OpFlag_KeepEnum))
				value.overrideType(((EnumType*)type)->getRootType());

			break;
		}

		if (value.getType() == type)
			break;
	}

	*resultValue = value;
	return true;
}

bool
OperatorMgr::awaitOperator(const Value& value)
{
	bool result;

	Value opPromiseValue;
	result = castOperator(value, m_module->m_typeMgr.getStdType(StdType_PromisePtr), &opPromiseValue);
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function->getFunctionKind() == FunctionKind_AsyncSequencer);

	Value thisPromiseValue = m_module->m_functionMgr.getPromiseValue();
	ASSERT(thisPromiseValue);

	// modify Promise (adjust state, save pending promise)

	Value stateFieldValue;
	Value stateIdValue;
	Value pendingPromiseFieldValue;
	Value waitValue;

	size_t stateId = m_module->m_controlFlowMgr.getAsyncBlockArray().getCount();
	stateIdValue.setConstSizeT(stateId, m_module);

	result =
		memberOperator(opPromiseValue, "wait", &waitValue) &&
		getPromiseField(thisPromiseValue, "m_state", &stateFieldValue) &&
		storeDataRef(stateFieldValue, stateIdValue) &&
		getPromiseField(thisPromiseValue, "m_pendingPromise", &pendingPromiseFieldValue) &&
		storeDataRef(pendingPromiseFieldValue, opPromiseValue);

	if (!result)
		return false;

	// create and call resume function (either scheduled, or non-scheduled)

	Value resumeFuncValue;
	Value schedulerValue;

	BasicBlock* schedulerBlock = m_module->m_controlFlowMgr.createBlock("scheduler_block");
	BasicBlock* noSchedulerBlock = m_module->m_controlFlowMgr.createBlock("no_scheduler_block");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock("follow_block");

	result =
		m_module->m_operatorMgr.getPromiseField(thisPromiseValue, "m_scheduler", &schedulerValue) &&
		m_module->m_operatorMgr.loadDataRef(&schedulerValue) &&
		m_module->m_controlFlowMgr.conditionalJump(schedulerValue, schedulerBlock, noSchedulerBlock, schedulerBlock) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_At, function, schedulerValue, &resumeFuncValue) &&
		closureOperator(resumeFuncValue, thisPromiseValue, &resumeFuncValue) &&
		callOperator(waitValue, resumeFuncValue);

	ASSERT(result);

	m_module->m_controlFlowMgr.jump(followBlock, noSchedulerBlock);

	result =
		closureOperator(function, thisPromiseValue, &resumeFuncValue) &&
		callOperator(waitValue, resumeFuncValue);

	m_module->m_controlFlowMgr.follow(followBlock);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	BasicBlock* block = m_module->m_controlFlowMgr.createAsyncBlock(scope);
	m_module->m_controlFlowMgr.asyncRet(block);
	return true;
}

bool
OperatorMgr::awaitOperator(
	const Value& value,
	Value* resultValue
	)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	if (function->getFunctionKind() != FunctionKind_AsyncSequencer)
	{
		err::setError("await can only be used in async functions");
		return false;
	}

	Value thisPromiseValue = m_module->m_functionMgr.getPromiseValue();
	ASSERT(thisPromiseValue);

	Value pendingPromiseFieldValue;
	Value opPromiseValue;
	Value waitValue;

	bool result = awaitOperator(value);
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	return
		getPromiseField(thisPromiseValue, "m_pendingPromise", &pendingPromiseFieldValue) &&
		loadDataRef(pendingPromiseFieldValue, &opPromiseValue) &&
		memberOperator(opPromiseValue, "blockingWait", &waitValue) &&
		callOperator(waitValue, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
