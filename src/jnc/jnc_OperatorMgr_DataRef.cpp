#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::getLeanDataPtrObjHdr (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);

	ValueKind valueKind = value.getValueKind ();
	if (valueKind == ValueKind_Variable)
	{	
		*resultValue = value.getVariable ()->getObjHdr ();
		return;
	}

	ASSERT (value.getLeanDataPtrValidator ());
	Value scopeValidatorValue = value.getLeanDataPtrValidator ()->getScopeValidator ();

	if (scopeValidatorValue.getValueKind () == ValueKind_Variable)
	{
		*resultValue = scopeValidatorValue.getVariable ()->getObjHdr ();
		return;
	}
	
	Type* scopeValidatorType = scopeValidatorValue.getType ();
	Type* resultType = m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr);
	if (scopeValidatorType->cmp (resultType) == 0)
	{
		*resultValue = scopeValidatorValue;
	}
	else if (scopeValidatorType->getTypeKind () == TypeKind_ClassPtr)
	{
		static int llvmIndexArray [] = { 0, 0, 1 }; // Iface*, IfaceHdr**, ObjHdr**

		Value objHdrValue;
		m_module->m_llvmIrBuilder.createGep (scopeValidatorValue, llvmIndexArray, 3, NULL, &objHdrValue);
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

	Type* bytePtrType = m_module->getSimpleType (StdType_BytePtr);

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

bool
OperatorMgr::prepareDataPtr (
	const Value& value,
	Value* resultValue
	)
{
	ASSERT (value.getType ()->getTypeKind () == TypeKind_DataPtr || value.getType ()->getTypeKind () == TypeKind_DataRef);
	DataPtrType* type = (DataPtrType*) value.getType ();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind ();

	DataPtrType* resultType = type->getTargetType ()->getDataPtrType_c ();

	Value ptrValue;
	Value rangeBeginValue;	
	Value rangeEndValue;	

	if (ptrTypeKind == DataPtrTypeKind_Thin)
	{
		resultValue->overrideType (value, resultType);
		return true;
	}
	else if (ptrTypeKind == DataPtrTypeKind_Lean)
	{
		if (type->getFlags () & PtrTypeFlag_Safe)
		{
			resultValue->overrideType (value, resultType);
			return true;
		}

		ptrValue.overrideType (value, resultType);
		getLeanDataPtrRange (value, &rangeBeginValue, &rangeEndValue);
	}
	else // EDataPtrType_Normal
	{
		m_module->m_llvmIrBuilder.createExtractValue (value, 0, resultType, &ptrValue);

		if (type->getFlags () & PtrTypeFlag_Safe)
		{
			*resultValue = ptrValue;
			return true;
		}

		m_module->m_llvmIrBuilder.createExtractValue (value, 1, NULL, &rangeBeginValue);
		m_module->m_llvmIrBuilder.createExtractValue (value, 2, NULL, &rangeEndValue);
	}

	checkDataPtrRange (ptrValue, type->getTargetType ()->getSize (), rangeBeginValue, rangeEndValue);
	*resultValue = ptrValue;
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
		castOperator (rawSrcValue, castType, &srcValue) &&
		prepareDataPtr (dstValue, &ptrValue);

	if (!result)
		return false;

	switch (targetTypeKind)
	{
	case TypeKind_DataPtr:
		result = checkDataPtrScopeLevel (srcValue, dstValue);
		if (!result)
			return false;

		break;

	case TypeKind_ClassPtr:
		checkClassPtrScopeLevel (srcValue, dstValue);
		break;

	case TypeKind_FunctionPtr:
		checkFunctionPtrScopeLevel (srcValue, dstValue);
		break;

	case TypeKind_PropertyPtr:
		checkPropertyPtrScopeLevel (srcValue, dstValue);
		break;

	case TypeKind_BitField:
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
	int64_t mask = ((int64_t) 1 << bitCount) - 1;

	Value value (rawValue, baseType);
	Value maskValue (mask, typeKind);
	Value offsetValue (bitOffset, typeKind);

	result = 
		binaryOperator (BinOpKind_Shr, &value, offsetValue) &&
		binaryOperator (BinOpKind_BwAnd, &value, maskValue);

	if (!result)
		return false;

	if (!(baseType->getTypeKindFlags () & TypeKindFlag_Unsigned)) // extend with sign bit
	{
		int64_t signBit = (int64_t) 1 << (bitCount - 1);

		Value signBitValue (signBit, typeKind);
		Value oneValue (1, typeKind);

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
	int64_t mask = (((int64_t) 1 << bitCount) - 1) << bitOffset;

	Value value (rawValue, baseType);
	Value shadowValue (rawShadowValue, baseType);
	Value maskValue (mask, typeKind);
	Value offsetValue (bitOffset, typeKind);

	result = 
		binaryOperator (BinOpKind_Shl, &value, offsetValue) &&
		binaryOperator (BinOpKind_BwAnd, value, maskValue, resultValue);

	if (!result)
		return false;

	mask = ~((((uint64_t) 1 << bitCount) - 1) << bitOffset);	
	maskValue.setConstInt64 (mask, typeKind);

	return 
		binaryOperator (BinOpKind_BwAnd, &shadowValue, maskValue) &&
		binaryOperator (BinOpKind_BwOr, &value, shadowValue) &&
		castOperator (value, baseType, resultValue);
}

//.............................................................................

} // namespace jnc {
