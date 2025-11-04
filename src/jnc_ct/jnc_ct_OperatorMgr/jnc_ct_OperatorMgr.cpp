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
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_LeanDataPtrValidator.h"
#include "jnc_rtl_DynamicLayout.h"

namespace jnc {
namespace ct {

//..............................................................................

OperatorMgr::OperatorMgr() {
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

	m_unOp_Inc.m_opKind = UnOpKind_Inc;
	m_unOp_Dec.m_opKind = UnOpKind_Dec;
	m_unOp_PostInc.m_opKind = UnOpKind_PostInc;
	m_unOp_PostDec.m_opKind = UnOpKind_PostDec;

	m_unaryOperatorTable[UnOpKind_Inc]   = &m_unOp_Inc;
	m_unaryOperatorTable[UnOpKind_Dec]   = &m_unOp_Dec;
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

	m_binaryOperatorTable[BinOpKind_Eq]       = &m_binOp_Eq;
	m_binaryOperatorTable[BinOpKind_Ne]       = &m_binOp_Ne;
	m_binaryOperatorTable[BinOpKind_Lt]       = &m_binOp_Lt;
	m_binaryOperatorTable[BinOpKind_Le]       = &m_binOp_Le;
	m_binaryOperatorTable[BinOpKind_Gt]       = &m_binOp_Gt;
	m_binaryOperatorTable[BinOpKind_Ge]       = &m_binOp_Ge;
	m_binaryOperatorTable[BinOpKind_Match]    = &m_binOp_Match;
	m_binaryOperatorTable[BinOpKind_NotMatch] = &m_binOp_NotMatch;

	// indexing operator

	m_binaryOperatorTable[BinOpKind_Idx] = &m_binOp_Idx;

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
	m_stdCastOperatorTable[StdCast_PtrFromInt] = &m_cast_PtrFromInt;
	m_stdCastOperatorTable[StdCast_Int] = &m_cast_Int;
	m_stdCastOperatorTable[StdCast_Fp] = &m_cast_Fp;
	m_stdCastOperatorTable[StdCast_FromVariant] = &m_cast_FromVariant;

	for (size_t i = 0; i < TypeKind__Count; i++)
		m_castOperatorTable[i] = &m_cast_Default;

	m_castOperatorTable[TypeKind_Bool] = &m_cast_Bool;

	for (size_t i = TypeKind_Int8; i <= TypeKind_Int64_u; i++)
		m_castOperatorTable[i] = &m_cast_Int;

	m_castOperatorTable[TypeKind_Void]          = &m_cast_Void;
	m_castOperatorTable[TypeKind_Float]         = &m_cast_Fp;
	m_castOperatorTable[TypeKind_Double]        = &m_cast_Fp;
	m_castOperatorTable[TypeKind_Variant]       = &m_cast_Variant;
	m_castOperatorTable[TypeKind_String]        = &m_cast_String;
	m_castOperatorTable[TypeKind_Array]         = &m_cast_Array;
	m_castOperatorTable[TypeKind_Enum]          = &m_cast_Enum;
	m_castOperatorTable[TypeKind_Struct]        = &m_cast_Struct;
	m_castOperatorTable[TypeKind_DataPtr]       = &m_cast_DataPtr;
	m_castOperatorTable[TypeKind_DataRef]       = &m_cast_DataRef;
	m_castOperatorTable[TypeKind_ClassPtr]      = &m_cast_ClassPtr;
	m_castOperatorTable[TypeKind_ClassRef]      = &m_cast_ClassRef;
	m_castOperatorTable[TypeKind_FunctionPtr]   = &m_cast_FunctionPtr;
	m_castOperatorTable[TypeKind_FunctionRef]   = &m_cast_FunctionRef;
	m_castOperatorTable[TypeKind_PropertyPtr]   = &m_cast_PropertyPtr;
	m_castOperatorTable[TypeKind_PropertyRef]   = &m_cast_PropertyRef;
	m_castOperatorTable[TypeKind_TypedefShadow] = &m_cast_Typedef;

	m_unsafeEnterCount = 0;
#if (_JNC_DYLAYOUT_FINALIZE_STRUCT_SECTIONS_ON_CALLS)
	m_callCount = 0;
#endif
}

void
OperatorMgr::clear() {
	m_unsafeEnterCount = 0;
#if (_JNC_DYLAYOUT_FINALIZE_STRUCT_SECTIONS_ON_CALLS)
	m_callCount = 0;
#endif
}

OverloadableFunction
OperatorMgr::getOverloadedUnaryOperator(
	UnOpKind opKind,
	const Value& opValue
) {
	Value opTypeValue;
	bool result = prepareOperandType(opValue, &opTypeValue);
	if (!result)
		return OverloadableFunction();

	Type* opType = opTypeValue.getType();
	if (opType->getTypeKind() == TypeKind_ClassPtr) {
		ClassPtrType* ptrType = (ClassPtrType*)opType;
		return ptrType->getTargetType()->getUnaryOperator(opKind);
	} else if (opType->getTypeKindFlags() & TypeKindFlag_Derivable) {
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
) {
	ASSERT((size_t)opKind < UnOpKind__Count);

	OverloadableFunction function = getOverloadedUnaryOperator(opKind, rawOpValue);
	if (function) {
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

	if (opValue.getType()->getTypeKind() == TypeKind_Variant && opKind <= UnOpKind_Indir) {
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
) {
	Value opTypeValue;
	bool result = prepareOperandType(opValue, &opTypeValue);
	if (!result)
		return OverloadableFunction();

	Type* opType = opTypeValue.getType();
	if (opType->getTypeKind() == TypeKind_ClassPtr) {
		ClassPtrType* ptrType = (ClassPtrType*)opType;
		return ptrType->getTargetType()->getBinaryOperator(opKind);
	} else if (opType->getTypeKindFlags() & TypeKindFlag_Derivable) {
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
) {
	ASSERT((size_t)opKind < BinOpKind__Count);

	bool result;

	OverloadableFunction function = getOverloadedBinaryOperator(opKind, rawOpValue1);
	if (function) {
		if (function->getFlags() & MulticastMethodFlag_InaccessibleViaEventPtr) {
			Value opValue1;
			result = prepareOperandType(rawOpValue1, &opValue1);
			if (!result)
				return false;

			if (opValue1.getType()->getTypeKind() == TypeKind_ClassPtr &&
				(opValue1.getType()->getFlags() & PtrTypeFlag_Event)) {
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
		opValue2.getType()->getTypeKind() == TypeKind_Variant)) {
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
) {
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
getConditionalOperandType(const Value& value) {
	Type* type = value.getType();
	Closure* closure = value.getClosure();
	if (closure) {
		type = closure->getClosureType(type);
		if (type->getTypeKindFlags() & TypeKindFlag_FunctionPtr)
			type = ((FunctionPtrType*)type)->getTargetType()->getFunctionPtrType(
				type->getTypeKind(),
				FunctionPtrTypeKind_Normal
			);
		else {
			ASSERT(type->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
			type = ((PropertyPtrType*)type)->getTargetType()->getPropertyPtrType(
				type->getTypeKind(),
				PropertyPtrTypeKind_Normal
			);
		}
	} else {
		switch (type->getTypeKind()) {
		case TypeKind_Array:
			type = ((ArrayType*)type)->getElementType()->getDataPtrType(
				DataPtrTypeKind_Normal,
				value.getValueKind() == ValueKind_Const ? PtrTypeFlag_Const : 0
			);
			break;
		case TypeKind_FunctionRef: {
			FunctionPtrType* ptrType = (FunctionPtrType*)type;
			type = ptrType->getTargetType()->getFunctionPtrType(
				TypeKind_FunctionPtr,
				ptrType->getPtrTypeKind(),
				ptrType->getFlags() & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)
			);
			break;
			}
		}
	}

	return type;
}

Type*
OperatorMgr::getConditionalOperatorResultType(
	const Value& trueValue,
	const Value& falseValue
) {
	bool result;

	Type* resultType;
	Type* trueType = getConditionalOperandType(trueValue);
	Type* falseType = getConditionalOperandType(falseValue);

	if (trueType->cmp(falseType) == 0)
		resultType = trueType;
	else {
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
				getCastKind(falseValue, trueType) ? // prefer true-type unless can't cast to true-type
					trueType :
					falseType;
	}

	// for pointers, remove `safe` flag
	// if it's a lean data pointer, turn it into a normal one

	if ((resultType->getTypeKindFlags() & TypeKindFlag_DataPtr) &&
		((DataPtrType*)resultType)->getPtrTypeKind() == DataPtrTypeKind_Lean
	)
		resultType = ((DataPtrType*)resultType)->getTargetType()->getDataPtrType(
			resultType->getTypeKind(),
			DataPtrTypeKind_Normal,
			resultType->getFlags() & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)
		);
	else if (
		(resultType->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		(resultType->getFlags() & PtrTypeFlag_Safe)
	)
		resultType = ((ClassPtrType*)resultType)->getTargetType()->getClassPtrType(
			resultType->getTypeKind(),
			ClassPtrTypeKind_Normal,
			resultType->getFlags() & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)
		);

	return resultType;
}

bool
OperatorMgr::conditionalOperator(
	const Value& rawTrueValue,
	const Value& rawFalseValue,
	BasicBlock* thenBlock,
	BasicBlock* phiBlock,
	Value* resultValue
) {
	bool result;

	Value trueValue;
	Value falseValue;

	Type* resultType = getConditionalOperatorResultType(rawTrueValue, rawFalseValue);
	if (!resultType)
		return false;

	if (resultType->getTypeKind() != TypeKind_Void) {
		result = castOperator(rawFalseValue, resultType, &falseValue);
		if (!result)
			return false;
	}

	BasicBlock* elseBlock = m_module->m_controlFlowMgr.getCurrentBlock(); // might have changed

	m_module->m_controlFlowMgr.jump(phiBlock, thenBlock);

	if (resultType->getTypeKind() != TypeKind_Void) {
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
) {
	Type* srcType = value.getType();

	if (srcType->getSize() >= dstType->getSize()) {
		Value tmpValue;
		m_module->m_llvmIrBuilder.createAlloca(srcType, NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createStore(value, tmpValue);
		m_module->m_llvmIrBuilder.createBitCast(tmpValue, dstType->getDataPtrType_c(), &tmpValue);
		m_module->m_llvmIrBuilder.createLoad(tmpValue, dstType, resultValue);
	} else {
		Value tmpValue, tmpValue2;
		m_module->m_llvmIrBuilder.createAlloca(dstType, NULL, &tmpValue);
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
) {
	bool result = type->ensureLayout();
	if (!result)
		return false;

	if (rawOpValue.getValueKind() == ValueKind_Null) {
		if ((type->getTypeKindFlags() & TypeKindFlag_Ptr) && (type->getFlags() & PtrTypeFlag_Safe)) {
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

	if (!m_module->hasCodeGen() && opValue.getValueKind() != ValueKind_Const) {
		resultValue->setType(type);
		return true;
	}

	Type* opType = opValue.getType();
	if (opType->cmp(type) == 0) { // identity, try to shortcut
		if (opValue.hasLlvmValue()) {
			*resultValue = opValue;
			return true;
		}

		if (opValue.getValueKind() == ValueKind_Property) {
			ASSERT(type->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
			return getPropertyThinPtr(opValue.getProperty(), opValue.getClosure(), (PropertyPtrType*)type, resultValue);
		}

		// nope, need to go through full cast
	}

	if (opType->getTypeKind() == TypeKind_Variant)
		return m_stdCastOperatorTable[StdCast_FromVariant]->cast(opValue, type, resultValue);

	if (dynamism != OperatorDynamism_Dynamic)
		return op->cast(opValue, type, resultValue);

	typeKind = type->getTypeKind();
	switch (typeKind) {
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
) {
	Type* type = m_module->m_typeMgr.getPrimitiveType(typeKind);
	return castOperator(dynamism, opValue, type, resultValue);
}

bool
OperatorMgr::dynamicCastDataPtr(
	const Value& opValue,
	DataPtrType* type,
	Value* resultValue
) {
	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr)) {
		err::setFormatStringError(
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType()->getTypeString().sz(),
			type->getTypeString().sz()
		);
		return false;
	}

	if ((opValue.getType()->getFlags() & PtrTypeFlag_Const) &&
		!(type->getFlags() & PtrTypeFlag_Const)) {
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
	Value typeValue(&targetType, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));

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
) {
	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr)) {
		err::setFormatStringError(
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType()->getTypeString().sz(),
			type->getTypeString().sz()
		);
		return false;
	}

	if ((opValue.getType()->getFlags() & PtrTypeFlag_Const) &&
		!(type->getFlags() & PtrTypeFlag_Const)) {
		setCastError(opValue, type);
		return false;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast(opValue, m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr), &ptrValue);

	Type* targetType = type->getTargetType();
	Value typeValue(&targetType, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));

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
) {
	if (rawOpValue.getValueKind() == ValueKind_Null)
		return (type->getTypeKindFlags() & TypeKindFlag_Nullable) ? CastKind_Identity : CastKind_None;

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
		opType->cmp(type) == 0 ? CastKind_Identity :
		opType->getTypeKind() == TypeKind_Variant ? CastKind_ImplicitCrossFamily :
		op->getCastKind(opValue, type);
}

CastKind
OperatorMgr::getArgCastKind(
	Closure* closure,
	FunctionType* functionType,
	FunctionArg* const* actualArgArray,
	size_t actualArgCount
) {
	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();
	if (closure) {
		bool result = closure->getArgTypeArray(m_module, &formalArgArray);
		if (!result)
			return CastKind_None;
	}

	CastKind worstCastKind = CastKind_Identity;
	size_t formalArgCount = formalArgArray.getCount();
	if (actualArgCount > formalArgCount)
		worstCastKind = CastKind_ImplicitLossyFuntionCall;
	else while (actualArgCount < formalArgCount) {
		formalArgCount--;
		if (!formalArgArray[formalArgCount]->hasInitializer())
			return CastKind_None;
	}

	for (size_t i = 0; i < formalArgCount; i++) {
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
) {
	CastKind worstCastKind = CastKind_Identity;
	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();
	size_t formalArgCount = formalArgArray.getCount();
	if (actualArgCount > formalArgCount)
		worstCastKind = CastKind_ImplicitLossyFuntionCall;
	else while (actualArgCount < formalArgCount) {
		formalArgCount--;
		if (!formalArgArray[formalArgCount]->hasInitializer())
			return CastKind_None;
	}

	const Value* arg = argValueArray;
	for (size_t i = 0; i < formalArgCount; i++, arg++) {
		FunctionArg* formalArg = formalArgArray[i];

		CastKind castKind =
			!arg->isEmpty() ? getCastKind(*arg, formalArg->getType()) :
			formalArg->hasInitializer() ? CastKind_Identity :
			CastKind_None;

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
) {
	CastKind worstCastKind = CastKind_Identity;
	size_t actualArgCount = argList.getCount();
	sl::Array<FunctionArg*> formalArgArray = functionType->getArgArray();
	size_t formalArgCount = formalArgArray.getCount();
	if (actualArgCount > formalArgCount)
		worstCastKind = CastKind_ImplicitLossyFuntionCall;
	else while (actualArgCount < formalArgCount) {
		formalArgCount--;
		if (!formalArgArray[formalArgCount]->hasInitializer())
			return CastKind_None;
	}

	sl::ConstBoxIterator<Value> arg = argList.getHead();
	for (size_t i = 0; i < formalArgCount; i++, arg++) {
		FunctionArg* formalArg = formalArgArray[i];

		CastKind castKind =
			!arg->isEmpty() ? getCastKind(*arg, formalArg->getType()) :
			formalArg->hasInitializer() ? CastKind_Identity :
			CastKind_None;

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
) {
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
) {
	CastKind castKind = getFunctionCastKind(srcType->getGetterType(), dstType->getGetterType());
	if (!castKind)
		return CastKind_None;

	FunctionTypeOverload* srcSetterType = srcType->getSetterType();
	FunctionTypeOverload* dstSetterType = dstType->getSetterType();

	CastKind worstCastKind = castKind;

	size_t count = dstSetterType->getOverloadCount();
	for (size_t i = 0; i < count; i++) {
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
) {
	CastKind castKind = getCastKind(opValue, type);
	if (castKind <= CastKind_Explicit) {
		setCastError(opValue, type, castKind);
		return false;
	}

	return true;
}

void
OperatorMgr::swapByteOrder(
	const Value& opValue,
	Value* resultValue
) {
	Type* type = opValue.getType();
	ASSERT((type->getTypeKindFlags() & TypeKindFlag_Integer) && type->getSize() >= 2);

	if (opValue.getValueKind() == ValueKind_Const) {
		int64_t buffer;
		sl::swapByteOrder(&buffer, opValue.getConstData(), type->getSize());
		resultValue->createConst(&buffer, type);
	} else {
		llvm::Function* llvmSwapFunc = llvm::Intrinsic::getDeclaration(
			m_module->getLlvmModule(),
			llvm::Intrinsic::bswap,
			llvm::ArrayRef<llvm::Type*>(type->getLlvmType())
		);

		m_module->m_llvmIrBuilder.createCall(
			llvmSwapFunc,
			m_module->m_typeMgr.getFunctionType(type, &type, 1),
			&opValue,
			1,
			type,
			resultValue
		);
	}
}

bool
OperatorMgr::sizeofOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
) {
	Value typeValue;
	bool result = prepareOperandType(opValue, &typeValue, OpFlag_LoadArrayRef);
	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic) {
		if (type->getTypeKind() != TypeKind_DataPtr) {
			err::setFormatStringError("'dynamic sizeof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicSizeOf);
		return callOperator(function, opValue, resultValue);
	}

	resultValue->setConstSizeT(type->getSize(), m_module);
	return true;
}

bool
OperatorMgr::countofOperator(
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
) {
	Value typeValue;
	bool result = prepareOperandType(opValue, &typeValue, OpFlag_LoadArrayRef);
	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic) {
		if (type->getTypeKind() != TypeKind_DataPtr) {
			err::setFormatStringError("'dynamic countof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

		type = ((DataPtrType*)type)->getTargetType();
		typeValue.createConst(&type, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicCountOf);
		return callOperator(function, opValue, typeValue, resultValue);
	}

	if (type->getTypeKind() != TypeKind_Array) {
		err::setFormatStringError("'countof' operator is only applicable to arrays, not to '%s'", type->getTypeString().sz());
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
) {
	Value typeValue;

	bool result =
		prepareOperandType(opValue, &typeValue, OpFlag_KeepBool | OpFlag_KeepEnum | OpFlag_LoadArrayRef) &&
		m_module->ensureIntrospectionLibRequired();

	if (!result)
		return false;

	Type* type = typeValue.getType();
	if (dynamism == OperatorDynamism_Dynamic) {
		if (!(type->getTypeKindFlags() & (TypeKindFlag_DataPtr | TypeKindFlag_ClassPtr))) {
			err::setFormatStringError("'dynamic typeof' operator is only applicable to data and class pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

/*		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicTypeOf);
		bool result = callOperator(function, opValue, resultValue);
		if (!result)
			return false; */

		err::setError("'dynamic typeof' operator is not yet implemented");
		return false;
	}

	if (opValue.getValueKind() == ValueKind_Type && type->getTypeKind() == TypeKind_Class) {
		ClassType* classType = (ClassType*)type;
		if (!(type->getFlags() & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable))) {
			result = classType->require();
			if (!result)
				return false;
		}
	}

	resultValue->setVariable(type->getTypeVariable());
	return prepareOperand(resultValue); // turn it into a class pointer
}

bool
OperatorMgr::declofOperator(
	const Value& opValue,
	Value* resultValue
) {
	ModuleItem* item = opValue.getModuleItem();
	ModuleItemDecl* itemDecl = NULL;
	Variable* variable = NULL;
	if (!item) {
		if (opValue.getValueKind() == ValueKind_Type) {
			Type* type = opValue.getType();
			if (type->getTypeKindFlags() & TypeKindFlag_Named) {
				itemDecl = (NamedType*)type;
				variable = type->getTypeVariable();
			}
		}
	} else {
		ModuleItemKind itemKind = item->getItemKind();
		switch (itemKind) {
		case ModuleItemKind_Variable:
			itemDecl = (Variable*)item;
			variable = ((Variable*)item)->getDeclVariable();
			break;

		case ModuleItemKind_Function:
			itemDecl = (Function*)item;
			variable = ((Function*)item)->getDeclVariable();
			break;

		case ModuleItemKind_Property:
			itemDecl = (Property*)item;
			variable = ((Property*)item)->getDeclVariable();
			break;

		case ModuleItemKind_EnumConst:
			itemDecl = (EnumConst*)item;
			variable = ((EnumConst*)item)->getDeclVariable();
			break;
		}
	}

	if (!variable) {
		err::setError("'declof' is only applicable to user items");
		return false;
	}

	bool result = itemDecl->ensureAttributeValuesReady();
	if (!result)
		return false;

	resultValue->setVariable(variable);
	return true;
}

bool
OperatorMgr::offsetofOperator(
	OperatorDynamism dynamism,
	const Value& value,
	Value* resultValue
) {
	if (dynamism == OperatorDynamism_Dynamic) {
		Value typeValue;
		bool result = prepareOperandType(value, &typeValue, OpFlag_LoadArrayRef);
		if (!result)
			return false;

		Type* type = typeValue.getType();
		if (type->getTypeKind() != TypeKind_DataPtr) {
			err::setFormatStringError("'dynamic sizeof' operator is only applicable to data pointers, not to '%s'", type->getTypeString().sz());
			return false;
		}

		Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicOffsetOf);
		return callOperator(function, value, resultValue);
	}

	if (value.getValueKind() != ValueKind_Field) {
		err::setError("'offsetof' can only be applied to fields");
		return false;
	}

	resultValue->setConstSizeT(value.getFieldOffset(), m_module);
	return true;
}

ModuleItem*
OperatorMgr::templateInstantiateOperator(
	ModuleItem* item,
	const sl::ArrayRef<Type*>& argArray
) {
	if (item->getItemKind() != ModuleItemKind_Template) {
		err::setFormatStringError("'%s' is not a template", getModuleItemKindString(item->getItemKind()));
		return NULL;
	}

	size_t argCount = argArray.getCount();
	sl::BoxList<Value> valueList;
	for (size_t i = 0; i < argCount; i++)
		valueList.insertTail(argArray[i]);

	return ((Template*)item)->instantiate(valueList);
}

bool
OperatorMgr::templateInstantiateOperator(
	const Value& opValue,
	const sl::ArrayRef<Type*>& argArray,
	Value* resultValue
) {
	if (opValue.getValueKind() != ValueKind_Template) {
		err::setFormatStringError("'%s' is not a template", getValueKindString(opValue.getValueKind()));
		return false;
	}

	size_t argCount = argArray.getCount();
	if (argCount > opValue.getTemplate()->getArgArray().getCount()) {
		err::setError("too many template arguments");
		return false;
	}

	sl::BoxList<Value> valueList;
	for (size_t i = 0; i < argCount; i++)
		valueList.insertTail(argArray[i]);

	Closure* closure = resultValue->getClosure();
	if (!closure)
		closure = resultValue->createClosure();

	sl::takeOver(closure->getArgValueList(), &valueList);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
OperatorMgr::prepareOperand_nop(
	Value* value,
	uint_t opFlags
) {
	return true;
}

bool
OperatorMgr::prepareOperandType_typedef(
	Value* value,
	uint_t opFlags
) {
	value->overrideType(((TypedefShadowType*)value->getType())->getTypedef()->getType());
	return true;
}

bool
OperatorMgr::prepareOperand_import(
	Value* value,
	uint_t opFlags
) {
	value->overrideType(((ImportType*)value->getType())->getActualType());
	return true;
}

bool
OperatorMgr::prepareOperand_dataPtr(
	Value* value,
	uint_t opFlags
) {
	return (opFlags & OpFlag_EnsurePtrTargetLayout) ?
		((DataPtrType*)value->getType())->getTargetType()->ensureLayout() :
		true;
}

bool
OperatorMgr::prepareOperandType_dataRef_derivable(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepDerivableRef))
		*value = ((DataPtrType*)value->getType())->getTargetType();
	return true;
}

bool
OperatorMgr::prepareOperand_dataRef_derivable(
	Value* value,
	uint_t opFlags
) {
	return (!(opFlags & OpFlag_KeepDerivableRef)) ?
		loadDataRef(value) :
		true;
}

bool
OperatorMgr::prepareOperandType_dataRef_variant(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepVariantRef))
		*value = ((DataPtrType*)value->getType())->getTargetType();
	return true;
}

bool
OperatorMgr::prepareOperand_dataRef_variant(
	Value* value,
	uint_t opFlags
) {
	return (!(opFlags & OpFlag_KeepVariantRef)) ?
		loadDataRef(value) :
		true;
}

bool
OperatorMgr::prepareOperandType_dataRef_string(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepStringRef))
		*value = ((DataPtrType*)value->getType())->getTargetType();
	return true;
}

bool
OperatorMgr::prepareOperand_dataRef_string(
	Value* value,
	uint_t opFlags
) {
	return (!(opFlags & OpFlag_KeepStringRef)) ?
		loadDataRef(value) :
		true;
}

bool
OperatorMgr::prepareOperandType_dataRef_array(
	Value* value,
	uint_t opFlags
) {
	DataPtrType* ptrType = (DataPtrType*)value->getType();

	if (opFlags & OpFlag_LoadArrayRef)
		*value = ptrType->getTargetType();
	else if (opFlags & OpFlag_ArrayRefToPtr) {
		ArrayType* arrayType = (ArrayType*)ptrType->getTargetType();
		*value = arrayType->getElementType()->getDataPtrType(
			TypeKind_DataPtr,
			ptrType->getPtrTypeKind(),
			ptrType->getFlags() & PtrTypeFlag__All
		);
	}

	return true;
}

bool
OperatorMgr::prepareOperand_dataRef_array(
	Value* value,
	uint_t opFlags
) {
	if (opFlags & OpFlag_LoadArrayRef)
		return loadDataRef(value);

	if (opFlags & OpFlag_ArrayRefToPtr)
		prepareArrayRef(value);

	return true;
}

bool
OperatorMgr::prepareOperandType_dataRef_default(
	Value* value,
	uint_t opFlags
) {
	*value = ((DataPtrType*)value->getType())->getTargetType();
	return true;
}

bool
OperatorMgr::prepareOperand_dataRef_default(
	Value* value,
	uint_t opFlags
) {
	return loadDataRef(value);
}

bool
OperatorMgr::prepareOperandType_dataRef(
	Value* value,
	uint_t opFlags
) {
	if (opFlags & OpFlag_EnsurePtrTargetLayout) {
		bool result = ((DataPtrType*)value->getType())->getTargetType()->ensureLayout();
		if (!result)
			return false;
	}

	if (!(opFlags & OpFlag_KeepDataRef)) {
		Type* targetType = ((DataPtrType*)value->getType())->getTargetType();
		TypeKind typeKind = targetType->getTypeKind();
		ASSERT(typeKind < TypeKind__Count);

		bool result = (this->*m_prepareOperandTypeFuncTable_dataRef[typeKind])(value, opFlags);
		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::prepareOperand_dataRef(
	Value* value,
	uint_t opFlags
) {
	bool result;
	Type* type = value->getType();

	if (opFlags & OpFlag_EnsurePtrTargetLayout) {
		result = ((DataPtrType*)type)->getTargetType()->ensureLayout();
		if (!result)
			return false;
	}

	if (!(opFlags & OpFlag_KeepDataRef)) {
		Type* targetType = ((DataPtrType*)value->getType())->getTargetType();
		TypeKind typeKind = targetType->getTypeKind();
		ASSERT(typeKind < TypeKind__Count);

		bool result = (this->*m_prepareOperandFuncTable_dataRef[typeKind])(value, opFlags);
		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::prepareOperand_classPtr(
	Value* value,
	uint_t opFlags
) {
	return (opFlags & OpFlag_EnsurePtrTargetLayout) ?
		((ClassPtrType*)value->getType())->getTargetType()->ensureLayout() :
		true;
}

bool
OperatorMgr::prepareOperand_classRef(
	Value* value,
	uint_t opFlags
) {
	Type* type = value->getType();
	if (opFlags & OpFlag_EnsurePtrTargetLayout) {
		bool result = ((ClassPtrType*)type)->getTargetType()->ensureLayout();
		if (!result)
			return false;
	}

	if (!(opFlags & OpFlag_KeepClassRef)) {
		ClassPtrType* ptrType = (ClassPtrType*)type;
		ClassType* targetType = ptrType->getTargetType();
		value->overrideType(
			targetType->getClassPtrType(
				TypeKind_ClassPtr,
				ptrType->getPtrTypeKind(),
				ptrType->getFlags() & PtrTypeFlag__All
			)
		);
	}

	return true;
}

bool
OperatorMgr::prepareOperand_functionRef(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepFunctionRef)) {
		FunctionPtrType* ptrType = (FunctionPtrType*)value->getType();
		FunctionType* targetType = ptrType->getTargetType();
		value->overrideType(
			targetType->getFunctionPtrType(
				ptrType->getPtrTypeKind(),
				ptrType->getFlags() & PtrTypeFlag__All
			)
		);
	}

	return true;
}

bool
OperatorMgr::prepareOperandType_propertyRef(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepPropertyRef)) {
		PropertyPtrType* ptrType = (PropertyPtrType*)value->getClosureAwareType();
		if (!ptrType)
			return false;

		PropertyType* targetType = ptrType->getTargetType();
		if (!targetType->isIndexed())
			*value = targetType->getReturnType();
	}

	return true;
}

bool
OperatorMgr::prepareOperand_propertyRef(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepPropertyRef)) {
		PropertyPtrType* ptrType = (PropertyPtrType*)value->getClosureAwareType();
		if (!ptrType)
			return false;

		PropertyType* targetType = ptrType->getTargetType();
		if (!targetType->isIndexed()) {
			if ((targetType->getFlags() & PropertyTypeFlag_Bindable) && m_module->m_controlFlowMgr.isReactiveExpression()) {
				bool result = addReactorBinding(*value);
				if (!result)
					return false;
			}

			bool result = getProperty(value);
			if (!result)
				return false;
		}
	}

	return true;
}

bool
OperatorMgr::prepareOperandType_bool(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepBool))
		*value = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8);

	return true;
}

bool
OperatorMgr::prepareOperand_bool(
	Value* value,
	uint_t opFlags
) {
	return (!(opFlags & OpFlag_KeepBool)) ?
		m_castIntFromBool.cast(
			*value,
			m_module->m_typeMgr.getPrimitiveType(TypeKind_Int8),
			value
		) :
		true;
}

bool
OperatorMgr::prepareOperand_enum(
	Value* value,
	uint_t opFlags
) {
	if (!(opFlags & OpFlag_KeepEnum))
		value->overrideType(((EnumType*)value->getType())->getRootType());

	return true;
}

OperatorMgr::PrepareOperandFunc OperatorMgr::m_prepareOperandTypeFuncTable[TypeKind__Count] = {
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Void
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Variant
	&OperatorMgr::prepareOperand_nop,             // TypeKind_String
	&OperatorMgr::prepareOperandType_bool,        // TypeKind_Bool
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int8
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int8_u
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int16
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int16_u
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int32
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int32_u
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int64
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Int64_u
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Float
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Double
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Array
	&OperatorMgr::prepareOperand_enum,            // TypeKind_Enum
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Struct
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Union
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Class
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Function
	&OperatorMgr::prepareOperand_nop,             // TypeKind_Property
	&OperatorMgr::prepareOperand_dataPtr,         // TypeKind_DataPtr
	&OperatorMgr::prepareOperandType_dataRef,     // TypeKind_DataRef
	&OperatorMgr::prepareOperand_classPtr,        // TypeKind_ClassPtr
	&OperatorMgr::prepareOperand_classRef,        // TypeKind_ClassRef
	&OperatorMgr::prepareOperand_nop,             // TypeKind_FunctionPtr
	&OperatorMgr::prepareOperand_nop,             // TypeKind_FunctionRef
	&OperatorMgr::prepareOperand_nop,             // TypeKind_PropertyPtr
	&OperatorMgr::prepareOperandType_propertyRef, // TypeKind_PropertyRef
	&OperatorMgr::prepareOperand_import,          // TypeKind_NamedImport
	&OperatorMgr::prepareOperand_import,          // TypeKind_ImportPtr
	&OperatorMgr::prepareOperand_import,          // TypeKind_ImportIntMod
	&OperatorMgr::prepareOperandType_typedef,     // TypeKind_TypedefShadow
	&OperatorMgr::prepareOperand_nop,             // TypeKind_TemplateArg
	&OperatorMgr::prepareOperand_nop,             // TypeKind_TemplateInstance
};

OperatorMgr::PrepareOperandFunc OperatorMgr::m_prepareOperandFuncTable[TypeKind__Count] = {
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Void
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Variant
	&OperatorMgr::prepareOperand_nop,         // TypeKind_String
	&OperatorMgr::prepareOperand_bool,        // TypeKind_Bool
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int8
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int8_u
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int16
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int16_u
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int32
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int32_u
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int64
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Int64_u
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Float
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Double
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Array
	&OperatorMgr::prepareOperand_enum,        // TypeKind_Enum
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Struct
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Union
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Class
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Function
	&OperatorMgr::prepareOperand_nop,         // TypeKind_Property
	&OperatorMgr::prepareOperand_dataPtr,     // TypeKind_DataPtr
	&OperatorMgr::prepareOperand_dataRef,     // TypeKind_DataRef
	&OperatorMgr::prepareOperand_classPtr,    // TypeKind_ClassPtr
	&OperatorMgr::prepareOperand_classRef,    // TypeKind_ClassRef
	&OperatorMgr::prepareOperand_nop,         // TypeKind_FunctionPtr
	&OperatorMgr::prepareOperand_functionRef, // TypeKind_FunctionRef
	&OperatorMgr::prepareOperand_nop,         // TypeKind_PropertyPtr
	&OperatorMgr::prepareOperand_propertyRef, // TypeKind_PropertyRef
	&OperatorMgr::prepareOperand_import,      // TypeKind_NamedImport
	&OperatorMgr::prepareOperand_import,      // TypeKind_ImportPtr
	&OperatorMgr::prepareOperand_import,      // TypeKind_ImportIntMod
	&OperatorMgr::prepareOperand_nop,         // TypeKind_TypedefShadow
	&OperatorMgr::prepareOperand_nop,         // TypeKind_TemplateArg
	&OperatorMgr::prepareOperand_nop,         // TypeKind_TemplateInstance
};

OperatorMgr::PrepareOperandFunc OperatorMgr::m_prepareOperandTypeFuncTable_dataRef[TypeKind__Count] = {
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Void
	&OperatorMgr::prepareOperandType_dataRef_variant,   // TypeKind_Variant
	&OperatorMgr::prepareOperandType_dataRef_string,    // TypeKind_String
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Bool
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int8
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int8_u
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int16
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int16_u
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int32
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int32_u
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int64
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Int64_u
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Float
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Double
	&OperatorMgr::prepareOperandType_dataRef_array,     // TypeKind_Array
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Enum
	&OperatorMgr::prepareOperandType_dataRef_derivable, // TypeKind_Struct
	&OperatorMgr::prepareOperandType_dataRef_derivable, // TypeKind_Union
	&OperatorMgr::prepareOperandType_dataRef_derivable, // TypeKind_Class
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Function
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_Property
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_DataPtr
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_DataRef
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_ClassPtr
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_ClassRef
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_FunctionPtr
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_FunctionRef
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_PropertyPtr
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_PropertyRef
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_NamedImport
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_ImportPtr
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_ImportIntMod
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_TypedefShadow
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_TemplateArg
	&OperatorMgr::prepareOperandType_dataRef_default,   // TypeKind_TemplateInstance
};

OperatorMgr::PrepareOperandFunc OperatorMgr::m_prepareOperandFuncTable_dataRef[TypeKind__Count] = {
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Void
	&OperatorMgr::prepareOperand_dataRef_variant,   // TypeKind_Variant
	&OperatorMgr::prepareOperand_dataRef_string,    // TypeKind_String
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Bool
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int8
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int8_u
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int16
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int16_u
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int32
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int32_u
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int64
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Int64_u
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Float
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Double
	&OperatorMgr::prepareOperand_dataRef_array,     // TypeKind_Array
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Enum
	&OperatorMgr::prepareOperand_dataRef_derivable, // TypeKind_Struct
	&OperatorMgr::prepareOperand_dataRef_derivable, // TypeKind_Union
	&OperatorMgr::prepareOperand_dataRef_derivable, // TypeKind_Class
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Function
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_Property
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_DataPtr
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_DataRef
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_ClassPtr
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_ClassRef
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_FunctionPtr
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_FunctionRef
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_PropertyPtr
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_PropertyRef
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_NamedImport
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_ImportPtr
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_ImportIntMod
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_TypedefShadow
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_TemplateArg
	&OperatorMgr::prepareOperand_dataRef_default,   // TypeKind_TemplateInstance
};

bool
OperatorMgr::prepareOperandType(
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
) {
	bool result;

	switch (opValue.getValueKind()) {
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

	for (;;) {
		Type* type = value.getType();
		TypeKind typeKind = type->getTypeKind();
		ASSERT((size_t)typeKind < TypeKind__Count);

		result =
			type->ensureLayout() &&
			(this->*m_prepareOperandTypeFuncTable[typeKind])(&value, opFlags);

		if (!result)
			return false;

		if (value.getType() == type)
			break;
	}

	*resultValue = value;
	return true;
}

bool
OperatorMgr::prepareOperand(
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
) {
	bool result;

	if (!m_module->hasCodeGen())
		return prepareOperandType(opValue, resultValue, opFlags);

	switch (opValue.getValueKind()) {
	case ValueKind_Void:
		resultValue->setVoid(m_module); // ensure non-null type
		return true;

	case ValueKind_FunctionOverload:
	case ValueKind_FunctionTypeOverload:
		*resultValue = opValue;
		return true;
	}

	Value value = opValue;

	for (;;) {
		Type* type = value.getType();
		TypeKind typeKind = type->getTypeKind();
		ASSERT((size_t)typeKind < TypeKind__Count);

		result =
			type->ensureLayout() &&
			(this->*m_prepareOperandFuncTable[typeKind])(&value, opFlags);

		if (!result)
			return false;

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
) {
	ASSERT(isArrayRefType(value.getType()));
	DataPtrType* ptrType = (DataPtrType*)value.getType();
	DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

	ArrayType* arrayType = (ArrayType*)ptrType->getTargetType();
	Type* elementType = arrayType->getElementType();
	DataPtrType* resultType = elementType->getDataPtrType(
		TypeKind_DataPtr,
		ptrTypeKind,
		ptrType->getFlags() & PtrTypeFlag__All
	);

	if (value.getValueKind() == ValueKind_Const || ptrTypeKind == DataPtrTypeKind_Normal) {
		resultValue->overrideType(value, resultType);
	} else if (ptrTypeKind != DataPtrTypeKind_Lean) {
		m_module->m_llvmIrBuilder.createGep2(value, arrayType, 0, resultType, resultValue);
	} else {
		// get validator first (resultValue can point to value)
		LeanDataPtrValidator* validator = value.getLeanDataPtrValidator();
		m_module->m_llvmIrBuilder.createGep2(value, arrayType, 0, resultType, resultValue);
		resultValue->setLeanDataPtrValidator(validator);
	}
}

bool
OperatorMgr::awaitOperator(const Value& value) {
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

	if (!result)
		return false;

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
) {
	if (!m_module->hasCodeGen()) {
		resultValue->setType(m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant));
		return true;
	}

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	if (function->getFunctionKind() != FunctionKind_AsyncSequencer) {
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

	return
		getPromiseField(thisPromiseValue, "m_pendingPromise", &pendingPromiseFieldValue) &&
		loadDataRef(pendingPromiseFieldValue, &opPromiseValue) &&
		memberOperator(opPromiseValue, "blockingWait", &waitValue) &&
		callOperator(waitValue, resultValue);
}

bool
OperatorMgr::awaitDynamicLayout(const Value& opValue) {
	ASSERT(m_module->m_functionMgr.getCurrentFunction()->getFunctionKind() == FunctionKind_AsyncSequencer);

	Value shouldAwaitValue;
	Value promiseValue;

	BasicBlock* awaitBlock = m_module->m_controlFlowMgr.createBlock("await_block");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock("follow_block");

	bool result =
		memberOperator(opValue, "m_shouldAwait", &shouldAwaitValue) &&
		m_module->m_controlFlowMgr.conditionalJump(shouldAwaitValue, awaitBlock, followBlock) &&
		memberOperator(opValue, "m_promise", &promiseValue) &&
		awaitOperator(promiseValue);

	if (!result)
		return false;

	m_module->m_controlFlowMgr.follow(followBlock);
	return true;
}

bool
OperatorMgr::getRegexGroup(
	size_t index,
	Value* resultValue
) {
	Scope* scope = m_module->m_namespaceMgr.findRegexScope();
	if (!scope) {
		err::setError("no regex groups are visible from here");
		return false;
	}

	if (!index) { // $0 is the match itself
		*resultValue = scope->m_regexMatchVariable;
		return true;
	}

	Value groupArrayValue;
	Value indexValue(index, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
	Value groupValue;

	BasicBlock* thenBlock = m_module->m_controlFlowMgr.createBlock("is_match");
	BasicBlock* elseBlock = m_module->m_controlFlowMgr.createBlock("no_match");
	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock("match_phi");

	bool result =
		m_module->m_controlFlowMgr.conditionalJump(scope->m_regexMatchVariable, thenBlock, elseBlock) &&
		memberOperator(scope->m_regexMatchVariable, "m_groupArray", &groupArrayValue) &&
		binaryOperator(BinOpKind_Idx, groupArrayValue, indexValue, &groupValue) &&
		prepareOperand(&groupValue);

	thenBlock = m_module->m_controlFlowMgr.setCurrentBlock(elseBlock);

	return m_module->m_operatorMgr.conditionalOperator(
		groupValue,
		groupValue.getType()->getZeroValue(),
		thenBlock,
		phiBlock,
		resultValue
	);
}

//..............................................................................

} // namespace ct
} // namespace jnc
