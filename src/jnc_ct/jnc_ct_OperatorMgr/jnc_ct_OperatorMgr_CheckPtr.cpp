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
#include "jnc_ct_LeanDataPtrValidator.h"

namespace jnc {
namespace ct {

//..............................................................................

void
OperatorMgr::checkPtr(
	StdFunc stdCheckFunction,
	StdFunc stdTryCheckFunction,
	const Value* argValueArray,
	size_t argCount
) {
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	ASSERT(scope);

	if (!scope->canStaticThrow()) {
		Function* checkFunction = m_module->m_functionMgr.getStdFunction(stdCheckFunction);

		m_module->m_llvmIrBuilder.createCall(
			checkFunction,
			checkFunction->getType(),
			argValueArray,
			argCount,
			m_module->m_typeMgr.getPrimitiveType(TypeKind_Void),
			NULL
		);
	} else {
		Function* checkFunction = m_module->m_functionMgr.getStdFunction(stdTryCheckFunction);
		FunctionType* checkFunctionType = checkFunction->getType();

		Value returnValue;
		m_module->m_llvmIrBuilder.createCall(
			checkFunction,
			checkFunctionType,
			argValueArray,
			argCount,
			checkFunctionType->getReturnType(),
			&returnValue
		);

		m_module->m_controlFlowMgr.checkErrorCode(returnValue, checkFunctionType->getReturnType());
	}
}

bool
OperatorMgr::checkDataPtrRange(const Value& value) {
	ASSERT(value.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);

	DataPtrType* type = (DataPtrType*)value.getType();
	DataPtrTypeKind ptrTypeKind = type->getPtrTypeKind();

	if (m_module->m_operatorMgr.isUnsafeRgn() ||
		(type->getFlags() & PtrTypeFlag_Safe) ||
		ptrTypeKind == DataPtrTypeKind_Thin)
		return true;

	size_t targetSize = type->getTargetType()->getSize();

	Value ptrValue;
	Value validatorValue;

	if (ptrTypeKind == DataPtrTypeKind_Normal) {
		m_module->m_llvmIrBuilder.createExtractValue(value, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createExtractValue(value, 1, NULL, &validatorValue);
	} else {
		ASSERT(ptrTypeKind == DataPtrTypeKind_Lean);

		m_module->m_llvmIrBuilder.createBitCast(value, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &ptrValue);

		LeanDataPtrValidator* validator = value.getLeanDataPtrValidator();
		if (validator->isDynamicRange() || validator->hasValidatorValue())
			validatorValue = validator->getValidatorValue();
		else {
			size_t rangeLength = validator->getRangeLength();
			if (rangeLength < targetSize) {
				err::setFormatStringError("'%s' fails range check", type->getTypeString().sz());
				return false;
			}

			rangeLength -= targetSize;

			Value rangeBeginValue = validator->getRangeBeginValue();
			m_module->m_llvmIrBuilder.createBitCast(rangeBeginValue, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &rangeBeginValue);

			Value argValueArray[] = {
				ptrValue,
				rangeBeginValue,
				Value(rangeLength, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
			};

			checkPtr(
				StdFunc_CheckDataPtrRangeDirect,
				StdFunc_TryCheckDataPtrRangeDirect,
				argValueArray,
				countof(argValueArray)
			);
			return true;
		}
	}

	Value argValueArray[] = {
		ptrValue,
		Value(targetSize, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
		validatorValue,
	};

	checkPtr(
		StdFunc_CheckDataPtrRangeIndirect,
		StdFunc_TryCheckDataPtrRangeIndirect,
		argValueArray,
		countof(argValueArray)
	);

	return true;
}

void
OperatorMgr::checkNullPtr(const Value& value) {
	Type* type = value.getType();

	if (m_module->m_operatorMgr.isUnsafeRgn() || (type->getFlags() & PtrTypeFlag_Safe))
		return;

	TypeKind typeKind = type->getTypeKind();
	ASSERT(typeKind == TypeKind_ClassPtr || typeKind == TypeKind_ClassRef);

	// use a static sink to avoid load being optimized out
	Variable* nullPtrCheckSink = m_module->m_variableMgr.getStdVariable(StdVariable_NullPtrCheckSink);

	Value tmpValue;
	m_module->m_llvmIrBuilder.createBitCast(value, nullPtrCheckSink->getType()->getDataPtrType_c(), &tmpValue);
	m_module->m_llvmIrBuilder.createLoad(tmpValue, nullPtrCheckSink->getType(), &tmpValue);
	m_module->m_llvmIrBuilder.createStore(tmpValue, nullPtrCheckSink);
}

//..............................................................................

} // namespace ct
} // namespace jnc
