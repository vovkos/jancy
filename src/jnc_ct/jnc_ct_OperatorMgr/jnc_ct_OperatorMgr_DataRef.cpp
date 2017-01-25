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
namespace ct {

//..............................................................................

bool
OperatorMgr::prepareDataPtr (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr || value.getType ()->getTypeKind () == TypeKind_DataRef);

	bool result = checkDataPtrRange (value);
	if (!result)
		return false;

	DataPtrType* type = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();
	DataPtrType* resultType = type->getTargetType ()->getDataPtrType_c ();

	switch (ptrTypeKind)
	{
	case DataPtrTypeKind_Thin:
	case DataPtrTypeKind_Lean:
		resultValue->overrideType (value, resultType);
		break;

	case DataPtrTypeKind_Normal:
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, NULL, resultValue);
		m_module->m_llvmIrBuilder.createBitCast (*resultValue, resultType, resultValue);
		break;

	default:
		ASSERT (false);
	}

	return true;
}

bool
OperatorMgr::loadDataRef (
	const Value& opValue,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_DataRef);

	bool result;

	DataPtrType* type = (DataPtrType*) opValue.getType ();
	Type* targetType = type->getTargetType ();

	Value ptrValue;
	result = prepareDataPtr (opValue, &ptrValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createLoad (
		ptrValue,
		targetType,
		resultValue,
		(type->getFlags () & PtrTypeFlag_Volatile) != 0
		);

	if (targetType->getTypeKind () == TypeKind_BitField)
	{
		result = extractBitField (
			*resultValue,
			(BitFieldType*) targetType,
			resultValue
			);

		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::storeDataRef (
	const Value& dstValue,
	const Value& rawSrcValue
	)
{
	ASSERT (dstValue.getType ()->getTypeKind () == TypeKind_DataRef);

	bool result;

	DataPtrType* dstType = (DataPtrType*) dstValue.getType ();
	if (dstType->isConstPtrType ())
	{
		err::setError ("cannot store into const location");
		return false;
	}

	Type* targetType = dstType->getTargetType ();
	TypeKind targetTypeKind = targetType->getTypeKind ();

	Type* castType = (targetTypeKind == TypeKind_BitField) ?
		((BitFieldType*) targetType)->getBaseType () :
		targetType;

	Value ptrValue;
	Value srcValue;
	Value bfShadowValue;

	result =
		checkCastKind (rawSrcValue, castType) &&
		castOperator (rawSrcValue, castType, &srcValue) &&
		prepareDataPtr (dstValue, &ptrValue);

	if (!result)
		return false;

	if (targetTypeKind == TypeKind_BitField)
	{
		m_module->m_llvmIrBuilder.createLoad (
			ptrValue,
			castType,
			&bfShadowValue,
			(dstType->getFlags () & PtrTypeFlag_Volatile) != 0
			);

		result = mergeBitField (
			srcValue,
			bfShadowValue,
			(BitFieldType*) targetType,
			&srcValue
			);

		if (!result)
			return false;
	}

	m_module->m_llvmIrBuilder.createStore (
		srcValue,
		ptrValue,
		(dstType->getFlags () & PtrTypeFlag_Volatile) != 0
		);

	return true;
}

bool
OperatorMgr::extractBitField (
	const Value& rawValue,
	BitFieldType* bitFieldType,
	Value* resultValue
	)
{
	bool result;

	Type* baseType = bitFieldType->getBaseType ();
	size_t bitOffset = bitFieldType->getBitOffset ();
	size_t bitCount = bitFieldType->getBitCount ();

	TypeKind typeKind = baseType->getSize () <= 4 ? TypeKind_Int32_u : TypeKind_Int64_u;
	Type* type = m_module->m_typeMgr.getPrimitiveType (typeKind);
	int64_t mask = ((int64_t) 1 << bitCount) - 1;

	Value value (rawValue, baseType);
	Value maskValue (mask, type);
	Value offsetValue (bitOffset, type);

	result =
		binaryOperator (BinOpKind_Shr, &value, offsetValue) &&
		binaryOperator (BinOpKind_BwAnd, &value, maskValue);

	if (!result)
		return false;

	if (!(baseType->getTypeKindFlags () & TypeKindFlag_Unsigned)) // extend with sign bit
	{
		int64_t signBit = (int64_t) 1 << (bitCount - 1);

		Value signBitValue (signBit, type);
		Value oneValue (1, type);

		Value signExtValue;
		result =
			binaryOperator (BinOpKind_BwAnd, &signBitValue, value) &&
			binaryOperator (BinOpKind_Sub, signBitValue, oneValue, &signExtValue) &&
			unaryOperator (UnOpKind_BwNot, &signExtValue) &&
			binaryOperator (BinOpKind_BwOr, &value, signExtValue);

		if (!result)
			return false;
	}

	return castOperator (value, baseType, resultValue);
}

bool
OperatorMgr::mergeBitField (
	const Value& rawValue,
	const Value& rawShadowValue,
	BitFieldType* bitFieldType,
	Value* resultValue
	)
{
	bool result;

	Type* baseType = bitFieldType->getBaseType ();
	size_t bitOffset = bitFieldType->getBitOffset ();
	size_t bitCount = bitFieldType->getBitCount ();

	TypeKind typeKind = baseType->getSize () <= 4 ? TypeKind_Int32_u : TypeKind_Int64_u;
	Type* type = m_module->m_typeMgr.getPrimitiveType (typeKind);
	int64_t mask = (((int64_t) 1 << bitCount) - 1) << bitOffset;

	Value value (rawValue, baseType);
	Value shadowValue (rawShadowValue, baseType);
	Value maskValue (mask, type);
	Value offsetValue (bitOffset, type);

	result =
		binaryOperator (BinOpKind_Shl, &value, offsetValue) &&
		binaryOperator (BinOpKind_BwAnd, value, maskValue, resultValue);

	if (!result)
		return false;

	mask = ~((((uint64_t) 1 << bitCount) - 1) << bitOffset);
	maskValue.setConstInt64 (mask, type);

	return
		binaryOperator (BinOpKind_BwAnd, &shadowValue, maskValue) &&
		binaryOperator (BinOpKind_BwOr, &value, shadowValue) &&
		castOperator (value, baseType, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
