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
#include "jnc_ct_LeanDataPtrValidator.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace rtl {

bool
tryCheckDataPtrRangeIndirect(
	const void* p,
	size_t size,
	DataPtrValidator* validator
);

} // namespace rtl

namespace ct {

//..............................................................................

bool
OperatorMgr::prepareDataPtr(
	const Value& value,
	Value* resultValue
) {
	ASSERT(value.getType()->getTypeKind() == TypeKind_DataPtr || value.getType()->getTypeKind() == TypeKind_DataRef);

	bool result = checkDataPtrRange(value);
	if (!result)
		return false;

	DataPtrType* type = (DataPtrType*)value.getType();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind();
	DataPtrType* resultType = type->getTargetType()->getDataPtrType_c();

	switch (ptrTypeKind) {
	case DataPtrTypeKind_Thin:
	case DataPtrTypeKind_Lean:
		resultValue->overrideType(value, resultType);
		break;

	case DataPtrTypeKind_Normal:
		if (value.getValueKind() == ValueKind_Const) {
			void* p = ((DataPtr*)value.getConstData())->m_p;
			resultValue->createConst(&p, resultType);
		} else {
			m_module->m_llvmIrBuilder.createExtractValue(value, 0, NULL, resultValue);
			m_module->m_llvmIrBuilder.createBitCast(*resultValue, resultType, resultValue);
		}
		break;

	default:
		ASSERT(false);
	}

	return true;
}

bool
OperatorMgr::loadDataRef(
	const Value& opValue,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_DataRef);

	bool result;

	DataPtrType* type = (DataPtrType*)opValue.getType();
	Type* targetType = type->getTargetType();
	uint_t typeFlags = type->getFlags();

	if (targetType->getFlags() & TypeFlag_Dynamic) {
		err::setFormatStringError("invalid usage of dynamic type '%s'", targetType->getTypeString().sz());
		return false;
	}

	if (opValue.getValueKind() != ValueKind_Const) {
		Value ptrValue;
		result = prepareDataPtr(opValue, &ptrValue);
		if (!result)
			return false;

		m_module->m_llvmIrBuilder.createLoad(
			ptrValue,
			targetType,
			resultValue,
			(typeFlags & PtrTypeFlag_Volatile) != 0
		);
	} else {
		const void* p;

		DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind();
		if (ptrTypeKind != DataPtrTypeKind_Normal) {
			p = *(void**) opValue.getConstData();
		} else {
			DataPtr* ptr = (DataPtr*)opValue.getConstData();
			result = rtl::tryCheckDataPtrRangeIndirect(ptr->m_p, targetType->getSize(), ptr->m_validator);
			if (!result)
				return false;

			p = ptr->m_p;
		}

		resultValue->createConst(p, targetType);
	}

	if ((typeFlags & PtrTypeFlag_BigEndian) && targetType->getSize() >= 2)
		swapByteOrder(resultValue);

	if (typeFlags & PtrTypeFlag_BitField) {
		result = extractBitField(
			*resultValue,
			targetType,
			type->getBitOffset(),
			type->getBitCount(),
			resultValue
		);

		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::storeDataRef(
	const Value& dstValue,
	const Value& rawSrcValue
) {
	ASSERT(dstValue.getType()->getTypeKind() == TypeKind_DataRef);

	bool result;

	DataPtrType* dstType = (DataPtrType*)dstValue.getType();
	uint_t dstTypeFlags = dstType->getFlags();

	if (dstType->getFlags() & PtrTypeFlag_Const) {
		err::setError("cannot store into const location");
		return false;
	}

	Type* targetType = dstType->getTargetType();
	if (targetType->getFlags() & TypeFlag_Dynamic) {
		err::setFormatStringError("invalid usage of dynamic type '%s'", targetType->getTypeString().sz());
		return false;
	}

	Value srcValue;
	Value bfShadowValue;

	result =
		checkCastKind(rawSrcValue, targetType) &&
		castOperator(rawSrcValue, targetType, &srcValue);

	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	bool isSwapByteOrderNeeded = (dstTypeFlags & PtrTypeFlag_BigEndian) && targetType->getSize() >= 2;

	if (srcValue.getValueKind() != ValueKind_Const ||
		dstValue.getValueKind() != ValueKind_Const) {
		Value ptrValue;
		result = prepareDataPtr(dstValue, &ptrValue);
		if (!result)
			return false;

		if (dstTypeFlags & PtrTypeFlag_BitField) {
			m_module->m_llvmIrBuilder.createLoad(
				ptrValue,
				targetType,
				&bfShadowValue,
				(dstType->getFlags() & PtrTypeFlag_Volatile) != 0
			);

			if (isSwapByteOrderNeeded)
				swapByteOrder(&bfShadowValue);

			result = mergeBitField(
				srcValue,
				bfShadowValue,
				targetType,
				dstType->getBitOffset(),
				dstType->getBitCount(),
				&srcValue
			);

			if (!result)
				return false;
		}

		if (isSwapByteOrderNeeded)
			swapByteOrder(&srcValue);

		m_module->m_llvmIrBuilder.createStore(
			srcValue,
			ptrValue,
			(dstType->getFlags() & PtrTypeFlag_Volatile) != 0
		);
	} else {
		void* p;

		DataPtrTypeKind ptrTypeKind = dstType->getPtrTypeKind();
		if (ptrTypeKind != DataPtrTypeKind_Normal) {
			p = *(void**) dstValue.getConstData();
		} else {
			DataPtr* ptr = (DataPtr*)dstValue.getConstData();
			result = rtl::tryCheckDataPtrRangeIndirect(ptr->m_p, targetType->getSize(), ptr->m_validator);
			if (!result)
				return false;

			p = ptr->m_p;
		}

		if (dstTypeFlags & PtrTypeFlag_BitField) {
			bfShadowValue.createConst(p, targetType);
			if (isSwapByteOrderNeeded)
				swapByteOrder(&bfShadowValue);

			result = mergeBitField(
				srcValue,
				bfShadowValue,
				targetType,
				dstType->getBitOffset(),
				dstType->getBitCount(),
				&srcValue
			);

			if (!result)
				return false;
		}

		if (isSwapByteOrderNeeded)
			swapByteOrder(&srcValue);

		memcpy(p, srcValue.getConstData(), targetType->getSize());
	}

	return true;
}

bool
OperatorMgr::extractBitField(
	const Value& rawValue,
	Type* baseType,
	uint_t bitOffset,
	uint_t bitCount,
	Value* resultValue
) {
	bool result;

	TypeKind typeKind = baseType->getSize() <= 4 ? TypeKind_Int32_u : TypeKind_Int64_u;
	Type* type = m_module->m_typeMgr.getPrimitiveType(typeKind);
	int64_t mask = ((int64_t) 1 << bitCount) - 1;

	Value value(rawValue, baseType);
	Value maskValue(mask, type);
	Value offsetValue(bitOffset, type);

	result =
		binaryOperator(BinOpKind_Shr, &value, offsetValue) &&
		binaryOperator(BinOpKind_BwAnd, &value, maskValue);

	if (!result)
		return false;

	if (!(baseType->getTypeKindFlags() & TypeKindFlag_Unsigned)) { // extend with sign bit
		int64_t signBit = (int64_t) 1 << (bitCount - 1);

		Value signBitValue(signBit, type);
		Value oneValue(1, type);

		Value signExtValue;
		result =
			binaryOperator(BinOpKind_BwAnd, &signBitValue, value) &&
			binaryOperator(BinOpKind_Sub, signBitValue, oneValue, &signExtValue) &&
			unaryOperator(UnOpKind_BwNot, &signExtValue) &&
			binaryOperator(BinOpKind_BwOr, &value, signExtValue);

		if (!result)
			return false;
	}

	return castOperator(value, baseType, resultValue);
}

bool
OperatorMgr::mergeBitField(
	const Value& rawValue,
	const Value& rawShadowValue,
	Type* baseType,
	uint_t bitOffset,
	uint_t bitCount,
	Value* resultValue
) {
	bool result;

	TypeKind typeKind = baseType->getSize() <= 4 ? TypeKind_Int32_u : TypeKind_Int64_u;
	Type* type = m_module->m_typeMgr.getPrimitiveType(typeKind);
	int64_t mask = (((int64_t) 1 << bitCount) - 1) << bitOffset;

	Value value(rawValue, baseType);
	Value shadowValue(rawShadowValue, baseType);
	Value maskValue(mask, type);
	Value offsetValue(bitOffset, type);

	result =
		binaryOperator(BinOpKind_Shl, &value, offsetValue) &&
		binaryOperator(BinOpKind_BwAnd, value, maskValue, resultValue);

	if (!result)
		return false;

	mask = ~((((uint64_t) 1 << bitCount) - 1) << bitOffset);
	maskValue.setConstInt64(mask, type);

	return
		binaryOperator(BinOpKind_BwAnd, &shadowValue, maskValue) &&
		binaryOperator(BinOpKind_BwOr, &value, shadowValue) &&
		castOperator(value, baseType, resultValue);
}

void
OperatorMgr::makeLeanDataPtr(
	const Value& value,
	Value* resultValue
) {
	ASSERT(
		value.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr &&
		((DataPtrType*)value.getType())->getPtrTypeKind() == DataPtrTypeKind_Normal);

	DataPtrType* ptrType = ((DataPtrType*)value.getType());
	ptrType = ptrType->getTargetType()->getDataPtrType(
		DataPtrTypeKind_Lean,
		ptrType->getFlags() & PtrTypeFlag__All
	);

	Type* validatorType = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);

	Value ptrValue;
	Value validatorValue;
	m_module->m_llvmIrBuilder.createExtractValue(value, 0, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createExtractValue(value, 1, validatorType, &validatorValue);
	m_module->m_llvmIrBuilder.createBitCast(ptrValue, ptrType, &ptrValue);
	resultValue->setLeanDataPtr(ptrValue.getLlvmValue(), ptrType, validatorValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
