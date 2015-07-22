#include "pch.h"
#include "jnc_LeanDataPtrValidator.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Value 
LeanDataPtrValidator::getValidatorValue ()
{
	if (m_validatorValue)
		return m_validatorValue;

	ASSERT (m_originValue);
	Module* module = m_originValue.getType ()->getModule ();

	if (m_originValue.getValueKind () == ValueKind_Variable)
	{
		Variable* variable = m_originValue.getVariable ();
		StorageKind storageKind = variable->getStorageKind ();
		switch (storageKind)
		{
		case StorageKind_Static:
			m_validatorValue = module->m_variableMgr.createStaticDataPtrValidatorVariable (variable);
			break;

		case StorageKind_Stack:
			module->m_variableMgr.liftStackVariable (variable);
			break;

		case StorageKind_Heap:
			ASSERT (false); // not yet
			break;

		default:
			ASSERT (false);
		}
	}
	else if (m_originValue.getType ()->getTypeKindFlags () & TypeKindFlag_ClassPtr)
	{
		ASSERT (m_rangeBeginValue && m_rangeLength);

		Function* createDataPtrValidator = module->m_functionMgr.getStdFunction (StdFunction_CreateDataPtrValidator);

		Value argValueArray [3];
		module->m_llvmIrBuilder.createBitCast (m_originValue, module->m_typeMgr.getStdType (StdType_BoxPtr), &argValueArray [0]);
		module->m_llvmIrBuilder.createBitCast (m_rangeBeginValue, module->m_typeMgr.getStdType (StdType_BytePtr), &argValueArray [1]);
		argValueArray [2].setConstSizeT (m_rangeLength, module);

		module->m_llvmIrBuilder.createCall (
			createDataPtrValidator,
			createDataPtrValidator->getType (),
			argValueArray,
			3,
			&m_validatorValue
			);		
	}
	else if (m_originValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr)
	{
		DataPtrType* type = (DataPtrType*) m_originValue.getType ();
		if (type->getTargetType ()->getStdType () == StdType_DataPtrValidator)
		{
			m_validatorValue = m_originValue;
		}
		else
		{
			ASSERT (type->getPtrTypeKind () == DataPtrTypeKind_Normal);
			module->m_llvmIrBuilder.createExtractValue (
				m_originValue, 
				1, 
				module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr), 
				&m_validatorValue
				);
		}
	}
	else
	{
		ASSERT (false);
	}

	ASSERT (m_validatorValue);
	return m_validatorValue;
}

//.............................................................................

} // namespace jnc {

/*

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

 */