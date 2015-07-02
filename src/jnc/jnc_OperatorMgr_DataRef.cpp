#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::getLeanDataPtrBox (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);

	ValueKind valueKind = value.getValueKind ();
	if (valueKind == ValueKind_Variable)
	{	
		*resultValue = value.getVariable ()->getBox ();
		return;
	}

	ASSERT (value.getLeanDataPtrValidator ());
	Value scopeValidatorValue = value.getLeanDataPtrValidator ()->getScopeValidator ();

	if (scopeValidatorValue.getValueKind () == ValueKind_Variable)
	{
		*resultValue = scopeValidatorValue.getVariable ()->getBox ();
		return;
	}
	
	Type* scopeValidatorType = scopeValidatorValue.getType ();
	Type* resultType = m_module->m_typeMgr.getStdType (StdType_BoxPtr);
	if (scopeValidatorType->cmp (resultType) == 0)
	{
		*resultValue = scopeValidatorValue;
	}
	else if (scopeValidatorType->getTypeKind () == TypeKind_ClassPtr)
	{
		Type* ifaceHdrPtrType = m_module->m_typeMgr.getStdType (StdType_SimpleIfaceHdrPtr);

		Value objHdrValue;
		m_module->m_llvmIrBuilder.createBitCast (scopeValidatorValue, ifaceHdrPtrType, &objHdrValue);
		m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 0, NULL, &objHdrValue); // root
		m_module->m_llvmIrBuilder.createLoad (objHdrValue, resultType, resultValue);
	}
	else
	{
		ASSERT (scopeValidatorType->getTypeKindFlags () & TypeKindFlag_DataPtr);
		ASSERT (((DataPtrType*) scopeValidatorType)->getPtrTypeKind () == DataPtrTypeKind_Normal);
		m_module->m_llvmIrBuilder.createExtractValue (scopeValidatorValue, 3, resultType, resultValue);
	}
}

void
OperatorMgr::getLeanDataPtrRange (
	const Value& value,
	Value* rangeBeginValue,
	Value* rangeEndValue
	)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);

	Type* bytePtrType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "calc lean data pointer range");

	ValueKind valueKind = value.getValueKind ();
	if (valueKind == ValueKind_Variable)
	{	
		size_t size =  value.getVariable ()->getType ()->getSize ();
		m_module->m_llvmIrBuilder.createBitCast (value, bytePtrType, rangeBeginValue);
		m_module->m_llvmIrBuilder.createGep (*rangeBeginValue, size, bytePtrType, rangeEndValue);
		return;
	}

	LeanDataPtrValidator* validator = value.getLeanDataPtrValidator ();
	ASSERT (validator);

	if (validator->getValidatorKind () == LeanDataPtrValidatorKind_Complex)
	{
		m_module->m_llvmIrBuilder.createBitCast (validator->getRangeBegin (), bytePtrType, rangeBeginValue);
		m_module->m_llvmIrBuilder.createGep (*rangeBeginValue, validator->getSizeValue (), bytePtrType, rangeEndValue);
		return;
	}

	ASSERT (validator->getValidatorKind () == LeanDataPtrValidatorKind_Simple);
	Value validatorValue = validator->getScopeValidator ();

	if (validatorValue.getValueKind () == ValueKind_Variable)
	{
		size_t size = validatorValue.getVariable ()->getType ()->getSize ();
		m_module->m_llvmIrBuilder.createBitCast (validatorValue, bytePtrType, rangeBeginValue);
		m_module->m_llvmIrBuilder.createGep (*rangeBeginValue, size, bytePtrType, rangeEndValue);
		return;
	}

	ASSERT (
		(validatorValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr) &&
		((DataPtrType*) validatorValue.getType ())->getPtrTypeKind () == DataPtrTypeKind_Normal);

	m_module->m_llvmIrBuilder.createExtractValue (validatorValue, 1, bytePtrType, rangeBeginValue);
	m_module->m_llvmIrBuilder.createExtractValue (validatorValue, 2, bytePtrType, rangeEndValue);		
}

void
OperatorMgr::prepareDataPtr (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr || value.getType ()->getTypeKind () == TypeKind_DataRef);

	checkDataPtrRange (value);

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
}

bool
OperatorMgr::loadDataRef (
	const Value& opValue,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_DataRef);
	
	DataPtrType* type = (DataPtrType*) opValue.getType ();

	Type* targetType = type->getTargetType ();

	Value ptrValue;
	prepareDataPtr (opValue, &ptrValue);

	m_module->m_llvmIrBuilder.createLoad (
		ptrValue, 
		targetType, 
		resultValue, 
		(type->getFlags () & PtrTypeFlag_Volatile) != 0
		);

	if (targetType->getTypeKind () == TypeKind_BitField)
	{
		bool result = extractBitField (
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
		err::setFormatStringError ("cannot store into const location");
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
		castOperator (rawSrcValue, castType, &srcValue);

	if (!result)
		return false;

	prepareDataPtr (dstValue, &ptrValue);

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

//.............................................................................

} // namespace jnc {
