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
#include "jnc_ct_LeanDataPtrValidator.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Value
LeanDataPtrValidator::getValidatorValue() {
	if (m_validatorValue)
		return m_validatorValue;

	createValidator();
	ASSERT(m_validatorValue);
	return m_validatorValue;
}

void
LeanDataPtrValidator::createValidator() {
	ASSERT(m_originValue);
	if (m_originValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr) {
		createClassFieldValidator();
		return;
	}

	ASSERT(m_originValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	Module* module = m_originValue.getType()->getModule();
	DataPtrType* originType = (DataPtrType*)m_originValue.getType();

	Value originValidatorValue;

	if (m_originValue.getValueKind() == ValueKind_Variable) {
		Variable* variable = m_originValue.getVariable();
		LeanDataPtrValidator* originValidator = variable->getLeanDataPtrValidator();

		if (!originValidator->m_validatorValue) {
			StorageKind storageKind = variable->getStorageKind();
			switch (storageKind) {
			case StorageKind_Static:
				module->m_variableMgr.createStaticDataPtrValidator(variable);
				break;

			case StorageKind_Stack:
				module->m_variableMgr.liftStackVariable(variable);
				break;

			default:
				ASSERT(false); // heap variables should already have validator value set
			}
		}

		originValidatorValue = originValidator->m_validatorValue;
		ASSERT(originValidatorValue);
	} else {
		if (originType->getTargetType()->getStdType() == StdType_DataPtrValidator) {
			originValidatorValue = m_originValue;
		} else if (originType->getPtrTypeKind() == DataPtrTypeKind_Lean) {
			originValidatorValue = m_originValue.getLeanDataPtrValidator()->getValidatorValue();
		} else {
			ASSERT(originType->getPtrTypeKind() == DataPtrTypeKind_Normal);
			module->m_llvmIrBuilder.createExtractValue(
				m_originValue,
				1,
				module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr),
				&originValidatorValue
			);
		}
	}

	if (!m_rangeBeginValue || m_rangeBeginValue.getLlvmValue() == m_originValue.getLlvmValue()) {
		m_validatorValue = originValidatorValue;
	} else {
		Value boxValue;
		module->m_llvmIrBuilder.createGep2(originValidatorValue, module->m_typeMgr.getStdType(StdType_DataPtrValidator), 1, NULL, &boxValue);
		module->m_llvmIrBuilder.createLoad(boxValue, module->m_typeMgr.getStdType(StdType_BoxPtr), &boxValue);
		createValidator(boxValue);
	}
}

void
LeanDataPtrValidator::createValidator(const Value& boxValue) {
	ASSERT(m_originValue && m_rangeBeginValue && m_rangeLength);

	Module* module = m_originValue.getType()->getModule();
	Function* createDataPtrValidator = module->m_functionMgr.getStdFunction(StdFunc_CreateDataPtrValidator);

	Value argValueArray[3];
	argValueArray[0] = boxValue;
	module->m_llvmIrBuilder.createBitCast(m_rangeBeginValue, module->m_typeMgr.getStdType(StdType_ByteThinPtr), &argValueArray[1]);
	argValueArray[2].setConstSizeT(m_rangeLength, module);

	module->m_llvmIrBuilder.createCall(
		createDataPtrValidator,
		createDataPtrValidator->getType(),
		argValueArray,
		3,
		createDataPtrValidator->getType()->getReturnType(),
		&m_validatorValue
	);

	module->m_gcShadowStackMgr.markGcRoot(
		m_validatorValue,
		module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr)
	);
}

void
LeanDataPtrValidator::createClassFieldValidator() {
	ASSERT(m_originValue);

	Module* module = m_originValue.getType()->getModule();

	Value boxValue;
	if (m_originValue.getValueKind() == ValueKind_Variable) {
		Value tmpValue;
		module->m_llvmIrBuilder.createBitCast(m_originValue, module->m_typeMgr.getStdType(StdType_BoxPtr), &tmpValue);
		module->m_llvmIrBuilder.createGep(tmpValue, module->m_typeMgr.getStdType(StdType_Box), -1, module->m_typeMgr.getStdType(StdType_BoxPtr), &boxValue);
	} else {
		Value tmpValue;
		module->m_llvmIrBuilder.createBitCast(m_originValue, module->m_typeMgr.getStdType(StdType_IfaceHdrPtr), &tmpValue);
		module->m_llvmIrBuilder.createGep2(tmpValue, module->m_typeMgr.getStdType(StdType_IfaceHdr), 1, NULL, &tmpValue);
		module->m_llvmIrBuilder.createLoad(tmpValue, module->m_typeMgr.getStdType(StdType_BoxPtr), &boxValue);
	}

	createValidator(boxValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
