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
#include "jnc_ct_BinOp_Arithmetic.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
stringIncrementOperator(
	Module* module,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	ASSERT(opValue1.getType()->getTypeKind() == TypeKind_String);
	Function* func = module->m_functionMgr.getStdFunction(StdFunc_StringIncrement);
	return module->m_operatorMgr.callOperator(func, opValue1, opValue2, resultValue);
}

bool
stringConcatenateOperator(
	Module* module,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Function* func = module->m_functionMgr.getStdFunction(StdFunc_StringConcatenate);
	return module->m_operatorMgr.callOperator(func, opValue1, opValue2, resultValue);
}

bool
dataPtrIncrementOperator(
	Module* module,
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	ASSERT(opValue1.getType()->getTypeKind() == TypeKind_DataPtr);

	bool result;

	DataPtrType* opType = (DataPtrType*)opValue1.getType();
	DataPtrType* resultType = opType->getUnCheckedPtrType();
	Type* targetType = opType->getTargetType();
	if (targetType->getStdType() == StdType_AbstractData) {
		err::setError("pointer arithmetic is not applicable to 'anydata' pointers");
		return false;
	}

	if (!module->hasCodeGen()) {
		resultValue->setType(resultType);
		return true;
	}

	DataPtrTypeKind ptrTypeKind = opType->getPtrTypeKind();

	if (opValue1.getValueKind() == ValueKind_Const &&
		opValue2.getValueKind() == ValueKind_Const) {
		Value deltaValue;
		result = module->m_operatorMgr.castOperator(opValue2, TypeKind_IntPtr, &deltaValue);
		if (!result)
			return false;

		intptr_t delta = deltaValue.getIntPtr() * targetType->getSize();

		if (ptrTypeKind == DataPtrTypeKind_Thin) {
			char* p = *(char**)opValue1.getConstData() + delta;
			resultValue->createConst(&p, opValue1.getType());
		} else {
			ASSERT(ptrTypeKind == DataPtrTypeKind_Normal); // lean is compile-time-only

			DataPtr ptr = *(DataPtr*)opValue1.getConstData();
			ptr.m_p = (char*)ptr.m_p + delta;
			resultValue->createConst(&ptr, opValue1.getType());
		}
	} else if (ptrTypeKind == DataPtrTypeKind_Thin) {
		module->m_llvmIrBuilder.createGep(opValue1, targetType, opValue2, resultType, resultValue);
	} else if (ptrTypeKind == DataPtrTypeKind_Lean) {
		module->m_llvmIrBuilder.createGep(opValue1, targetType, opValue2, resultType, resultValue);
		resultValue->setLeanDataPtr(resultValue->getLlvmValue(), resultType, opValue1);
	} else { // DataPtrTypeKind_Normal
		size_t size = targetType->getSize();
		Value sizeValue(size ? size : 1, module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));

		Value incValue;
		module->m_operatorMgr.binaryOperator(BinOpKind_Mul, opValue2, sizeValue, &incValue);

		Type* byteType = module->m_typeMgr.getPrimitiveType(TypeKind_Byte);
		Value ptrValue;
		module->m_llvmIrBuilder.createExtractValue(opValue1, 0, NULL, &ptrValue);
		module->m_llvmIrBuilder.createGep(ptrValue, byteType, incValue, NULL, &ptrValue);
		module->m_llvmIrBuilder.createInsertValue(opValue1, ptrValue, 0, resultType, resultValue);
	}

	return true;
}

bool
dataPtrDifferenceOperator(
	Module* module,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	ASSERT(rawOpValue1.getType()->getTypeKind() == TypeKind_DataPtr);
	ASSERT(rawOpValue2.getType()->getTypeKind() == TypeKind_DataPtr);

	Type* targetType1 = ((DataPtrType*)rawOpValue1.getType())->getTargetType();
	Type* targetType2 = ((DataPtrType*)rawOpValue2.getType())->getTargetType();

	if (targetType1->cmp(targetType2) != 0) {
		err::setError("pointer difference target types mismatch");
		return false;
	} else if (targetType1->getStdType() == StdType_AbstractData) {
		err::setError("pointer arithmetic is not applicable to 'anydata' pointers");
		return false;
	}

	Type* bytePtrType = module->m_typeMgr.getStdType(StdType_CharConstThinPtr);
	Value opValue1;
	Value opValue2;

	bool result =
		module->m_operatorMgr.castOperator(rawOpValue1, bytePtrType, &opValue1) &&
		module->m_operatorMgr.castOperator(rawOpValue2, bytePtrType, &opValue2);

	if (!result)
		return false;

	size_t size = targetType1->getSize();
	if (!size)
		size = 1;

	Type* type = module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr);

	if (opValue1.getValueKind() == ValueKind_Const &&
		opValue2.getValueKind() == ValueKind_Const) {
		char* p1 = *(char**)opValue1.getConstData();
		char* p2 = *(char**)opValue2.getConstData();
		intptr_t diff = (p1 - p2) / size;

		resultValue->setConstSizeT(diff, type);
	} else if (!module->hasCodeGen()) {
		resultValue->setType(type);
	} else {
		Value diffValue;
		Value sizeValue;
		sizeValue.setConstSizeT(size, module);

		module->m_llvmIrBuilder.createPtrToInt(opValue1, type, &opValue1);
		module->m_llvmIrBuilder.createPtrToInt(opValue2, type, &opValue2);
		module->m_llvmIrBuilder.createSub_i(opValue1, opValue2, type, &diffValue);
		module->m_llvmIrBuilder.createDiv_i(diffValue, sizeValue, type, resultValue);
	}

	return true;
}
//..............................................................................

bool
BinOp_Add::op(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	Type* opType1 = opValue1.getType();
	Type* opType2 = opValue2.getType();

	if (opType2->getTypeKindFlags() & TypeKindFlag_Integer) {
		if (opType1->getTypeKind() == TypeKind_String)
			return stringIncrementOperator(m_module, opValue1, opValue2, resultValue);
		if (opType1->getTypeKind() == TypeKind_DataPtr)
			return dataPtrIncrementOperator(m_module, opValue1, opValue2, resultValue);
	} else if (opType1->getTypeKindFlags() & TypeKindFlag_Integer) {
		if (opType2->getTypeKind() == TypeKind_String)
			return stringIncrementOperator(m_module, opValue2, opValue1, resultValue);
		if (opType2->getTypeKind() == TypeKind_DataPtr)
			return dataPtrIncrementOperator(m_module, opValue2, opValue1, resultValue);
	} else if (opType1->getTypeKind() == TypeKind_String || opType2->getTypeKind() == TypeKind_String)
		return stringConcatenateOperator(m_module, opValue1, opValue2, resultValue);

	return BinOp_Arithmetic<BinOp_Add>::op(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Add::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createAdd_i(opValue1, opValue2, resultType, resultValue);
}

llvm::Value*
BinOp_Add::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createAdd_f(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

bool
BinOp_Sub::op(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	if (opValue1.getType()->getTypeKind() == TypeKind_DataPtr &&
		(opValue2.getType()->getTypeKindFlags() & TypeKindFlag_Integer)) {
		Value negOpValue2;
		return
			m_module->m_operatorMgr.unaryOperator(UnOpKind_Minus, opValue2, &negOpValue2) &&
			dataPtrIncrementOperator(m_module, opValue1, negOpValue2, resultValue);
	}

	if (opValue1.getType()->getTypeKind() == TypeKind_DataPtr && opValue2.getType()->getTypeKind() == TypeKind_DataPtr)
		return dataPtrDifferenceOperator(m_module, opValue1, opValue2, resultValue);
	else
		return BinOp_Arithmetic<BinOp_Sub>::op(opValue1, opValue2, resultValue);
}

llvm::Value*
BinOp_Sub::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createSub_i(opValue1, opValue2, resultType, resultValue);
}


llvm::Value*
BinOp_Sub::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createSub_f(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Mul::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createMul_i(opValue1, opValue2, resultType, resultValue);
}

llvm::Value*
BinOp_Mul::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createMul_f(opValue1, opValue2, resultType, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
BinOp_Div::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createDiv_u(opValue1, opValue2, resultType, resultValue) :
		m_module->m_llvmIrBuilder.createDiv_i(opValue1, opValue2, resultType, resultValue);
}


llvm::Value*
BinOp_Div::llvmOpFp(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createDiv_f(opValue1, opValue2, resultType, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
BinOp_Mod::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return isUnsigned ?
		m_module->m_llvmIrBuilder.createMod_u(opValue1, opValue2, resultType, resultValue) :
		m_module->m_llvmIrBuilder.createMod_i(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

llvm::Value*
BinOp_Shl::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createShl(opValue1, opValue2, resultType, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
BinOp_Shr::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createShr(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

EnumType*
getBitFlagEnumBwAndResultType(
	const Value& opValue1,
	const Value& opValue2
) {
	EnumType* opType1 = (EnumType*)opValue1.getType();
	EnumType* opType2 = (EnumType*)opValue2.getType();

	if (!isBitFlagEnumType(opType1))
		return isBitFlagEnumType(opType2) ? opType2 : NULL;

	return isBitFlagEnumType(opType2) && opType2->isBaseType(opType1) ?
		opType2 :
		opType1;
}

EnumType*
getBitFlagEnumBwOrXorResultType(
	const Value& opValue1,
	const Value& opValue2
) {
	EnumType* opType1 = (EnumType*)opValue1.getType();
	EnumType* opType2 = (EnumType*)opValue2.getType();

	if (!isBitFlagEnumType(opType1) || !isBitFlagEnumType(opType2))
		return NULL;

	return
		opType1->cmp(opType2) == 0 ? opType1 :
		opType2->isBaseType(opType1) ? opType2 :
		opType1->isBaseType(opType2) ? opType1 :
		NULL;
}

//..............................................................................

bool
BinOp_BwAnd::op(
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	Value opValue1;
	Value opValue2;
	Value tmpValue;

	// prepare before calling Base::Op cause we have OpFlag_KeepEnum

	EnumType* enumType = getBitFlagEnumBwAndResultType(rawOpValue1, rawOpValue2);
	if (!enumType)
		return
			m_module->m_operatorMgr.prepareOperand(rawOpValue1, &opValue1) &&
			m_module->m_operatorMgr.prepareOperand(rawOpValue2, &opValue2) &&
			BinOp_IntegerOnly<BinOp_BwAnd>::op(opValue1, opValue2, resultValue);

	return
		m_module->m_operatorMgr.prepareOperand(rawOpValue1, &opValue1) &&
		m_module->m_operatorMgr.prepareOperand(rawOpValue2, &opValue2) &&
		BinOp_IntegerOnly<BinOp_BwAnd>::op(opValue1, opValue2, &tmpValue) &&
		m_module->m_operatorMgr.castOperator(tmpValue, enumType, resultValue);
}

llvm::Value*
BinOp_BwAnd::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createAnd(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

bool
BinOp_BwOr::op(
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	Value opValue1;
	Value opValue2;

	// prepare before calling Base::Op cause we have OpFlag_KeepEnum

	EnumType* enumType = getBitFlagEnumBwOrXorResultType(rawOpValue1, rawOpValue2);
	if (!enumType)
		return
			m_module->m_operatorMgr.prepareOperand(rawOpValue1, &opValue1) &&
			m_module->m_operatorMgr.prepareOperand(rawOpValue2, &opValue2) &&
			BinOp_IntegerOnly<BinOp_BwOr>::op(opValue1, opValue2, resultValue);

	opValue1.overrideType(rawOpValue1, enumType->getRootType());
	opValue2.overrideType(rawOpValue2, enumType->getRootType());

	return
		BinOp_IntegerOnly<BinOp_BwOr>::op(opValue1, opValue2, resultValue) &&
		m_module->m_operatorMgr.castOperator(resultValue, enumType);
}

llvm::Value*
BinOp_BwOr::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createOr(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

bool
BinOp_BwXor::op(
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	Value opValue1;
	Value opValue2;

	// prepare before calling Base::Op cause we have OpFlag_KeepEnum

	EnumType* enumType = getBitFlagEnumBwOrXorResultType(rawOpValue1, rawOpValue2);
	if (!enumType)
		return
			m_module->m_operatorMgr.prepareOperand(rawOpValue1, &opValue1) &&
			m_module->m_operatorMgr.prepareOperand(rawOpValue2, &opValue2) &&
			BinOp_IntegerOnly<BinOp_BwXor>::op(opValue1, opValue2, resultValue);

	opValue1.overrideType(rawOpValue1, enumType->getRootType());
	opValue2.overrideType(rawOpValue2, enumType->getRootType());

	return
		BinOp_IntegerOnly<BinOp_BwXor>::op(opValue1, opValue2, resultValue) &&
		m_module->m_operatorMgr.castOperator(resultValue, enumType);
}

llvm::Value*
BinOp_BwXor::llvmOpInt(
	const Value& opValue1,
	const Value& opValue2,
	Type* resultType,
	Value* resultValue,
	bool isUnsigned
) {
	return m_module->m_llvmIrBuilder.createXor(opValue1, opValue2, resultType, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
