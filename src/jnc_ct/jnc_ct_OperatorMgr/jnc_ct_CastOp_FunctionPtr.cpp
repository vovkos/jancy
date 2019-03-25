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
#include "jnc_ct_CastOp_FunctionPtr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_ClosureClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_FunctionPtr_FromMulticast::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(isClassPtrType(opValue.getType(), ClassTypeKind_Multicast));
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	if (opValue.getType()->getFlags() & PtrTypeFlag_Event)
		return CastKind_None;

	MulticastClassType* mcType = (MulticastClassType*)((ClassPtrType*)opValue.getType())->getTargetType();
	return m_module->m_operatorMgr.getCastKind(mcType->getTargetType(), type);
}

bool
Cast_FunctionPtr_FromMulticast::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(isClassPtrType(opValue.getType(), ClassTypeKind_Multicast));
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	Value callValue;

	return
		m_module->m_operatorMgr.memberOperator(opValue, "call", &callValue) &&
		m_module->m_operatorMgr.castOperator(callValue, type, resultValue);
}

//..............................................................................

CastKind
Cast_FunctionPtr_FromDataPtr::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	FunctionPtrType* dstType = (FunctionPtrType*)type;
	DataPtrType* srcType = (DataPtrType*)opValue.getType();

	return
		srcType->getPtrTypeKind() != DataPtrTypeKind_Thin ? CastKind_None :
		dstType->getPtrTypeKind() != FunctionPtrTypeKind_Thin ? CastKind_None :
		dstType->getTargetType()->getTypeKind() == TypeKind_Void ? CastKind_ImplicitCrossFamily :
		CastKind_Explicit;
}

bool
Cast_FunctionPtr_FromDataPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	FunctionPtrType* dstType = (FunctionPtrType*)type;
	DataPtrType* srcType = (DataPtrType*)opValue.getType();

	if (srcType->getPtrTypeKind() != DataPtrTypeKind_Thin ||
		dstType->getPtrTypeKind() != FunctionPtrTypeKind_Thin)
	{
		setCastError(opValue, type);
		return false;
	}

	if (!m_module->m_operatorMgr.isUnsafeRgn())
	{
		setUnsafeCastError(srcType, dstType);
		return false;
	}

	m_module->m_llvmIrBuilder.createBitCast(opValue, type, resultValue);
	return true;
}

//..............................................................................

CastKind
Cast_FunctionPtr_Base::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	if (!opValue.getType())
	{
		ASSERT(opValue.getValueKind() == ValueKind_Function && opValue.getFunction()->isOverloaded());
		return CastKind_None; // choosing overload is not yet implemented
	}

	FunctionPtrType* srcPtrType = (FunctionPtrType*)opValue.getClosureAwareType();
	FunctionPtrType* dstPtrType = (FunctionPtrType*)type;

	if (!srcPtrType)
		return CastKind_None;

	CastKind castKind = m_module->m_operatorMgr.getFunctionCastKind(
		srcPtrType->getTargetType(),
		dstPtrType->getTargetType()
		);

	if (castKind != CastKind_None)
		return castKind;

	// second attempt -- use non-closure-aware type

	srcPtrType = (FunctionPtrType*)opValue.getType();
	return m_module->m_operatorMgr.getFunctionCastKind(
		srcPtrType->getTargetType(),
		dstPtrType->getTargetType()
		);
}

//..............................................................................

bool
Cast_FunctionPtr_FromFat::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	FunctionPtrType* srcPtrType = (FunctionPtrType*)opValue.getType();
	FunctionType* srcFunctionType = srcPtrType->getTargetType();

	FunctionPtrType* thinPtrType = srcFunctionType->getStdObjectMemberMethodType()->getFunctionPtrType(FunctionPtrTypeKind_Thin);

	Value pfnValue;
	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, thinPtrType, &pfnValue);
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 1, m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr), &closureValue);

	Closure* closure = opValue.getClosure();
	if (closure)
		pfnValue.setClosure(closure);
	else
		closure = pfnValue.createClosure();

	closure->insertThisArgValue(closureValue);

	return m_module->m_operatorMgr.castOperator(pfnValue, type, resultValue);
}

//..............................................................................

bool
Cast_FunctionPtr_Weak2Normal::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr && ((FunctionPtrType*)type)->getPtrTypeKind() == FunctionPtrTypeKind_Normal);

	BasicBlock* initialBlock = m_module->m_controlFlowMgr.getCurrentBlock();
	BasicBlock* strengthenBlock = m_module->m_controlFlowMgr.createBlock("strengthen");
	BasicBlock* aliveBlock = m_module->m_controlFlowMgr.createBlock("alive");
	BasicBlock* deadBlock = m_module->m_controlFlowMgr.createBlock("dead");
	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock("phi");

	Type* closureType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);
	Value nullClosureValue = closureType->getZeroValue();

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 1, closureType, &closureValue);

	Value cmpValue;
	m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, closureValue, nullClosureValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump(cmpValue, strengthenBlock, phiBlock);

	Function* strengthenFunction = m_module->m_functionMgr.getStdFunction(StdFunc_StrengthenClassPtr);

	Value strengthenedClosureValue;
	m_module->m_llvmIrBuilder.createCall(
		strengthenFunction,
		strengthenFunction->getType(),
		closureValue,
		&strengthenedClosureValue
		);

	m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, strengthenedClosureValue, nullClosureValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump(cmpValue, aliveBlock, deadBlock);
	m_module->m_controlFlowMgr.follow(phiBlock);

	m_module->m_controlFlowMgr.setCurrentBlock(deadBlock);
	m_module->m_controlFlowMgr.follow(phiBlock);

	Value valueArray[3] =
	{
		opValue,
		opValue,
		opValue.getType()->getZeroValue()
	};

	BasicBlock* blockArray[3] =
	{
		initialBlock,
		aliveBlock,
		deadBlock
	};

	Value intermediateValue;
	m_module->m_llvmIrBuilder.createPhi(valueArray, blockArray, 3, &intermediateValue);

	FunctionPtrType* intermediateType = ((FunctionPtrType*)opValue.getType())->getUnWeakPtrType();
	intermediateValue.overrideType(intermediateType);
	return m_module->m_operatorMgr.castOperator(intermediateValue, type, resultValue);
}

//..............................................................................

bool
Cast_FunctionPtr_Thin2Fat::llvmCast(
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(rawOpValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	Value opValue = rawOpValue;

	FunctionPtrType* srcPtrType = (FunctionPtrType*)opValue.getType();
	FunctionPtrType* dstPtrType = (FunctionPtrType*)type;

	FunctionType* srcFunctionType = srcPtrType->getTargetType();
	FunctionType* dstFunctionType = dstPtrType->getTargetType();

	Closure* closure = opValue.getClosure();

	Value simpleClosureValue;

	bool isSimpleClosure = closure && closure->isSimpleClosure();
	if (isSimpleClosure)
		simpleClosureValue = *closure->getArgValueList()->getHead();

	if (opValue.getValueKind() == ValueKind_Function && opValue.getFunction()->isVirtual())
	{
		bool result = m_module->m_operatorMgr.getVirtualMethod(opValue.getFunction(), closure, &opValue);
		if (!result)
			return false;
	}

	// case 1: no conversion required, no closure object needs to be created

	if (isSimpleClosure &&
		srcFunctionType->isMemberMethodType() &&
		srcFunctionType->getShortType()->cmp(dstFunctionType) == 0)
	{
		return llvmCast_NoThunkSimpleClosure(
			opValue,
			simpleClosureValue,
			srcFunctionType,
			dstPtrType,
			resultValue
			);
	}

	if (opValue.getValueKind() == ValueKind_Function)
	{
		Function* function = opValue.getFunction();
		ASSERT(!function->isVirtual());

		// case 2.1: conversion is required, but no closure object needs to be created (closure arg is null)

		if (!closure)
			return llvmCast_DirectThunkNoClosure(
				function,
				dstPtrType,
				resultValue
				);

		// case 2.2: same as above, but simple closure is passed as closure arg

		if (isSimpleClosure && function->getType()->isMemberMethodType())
			return llvmCast_DirectThunkSimpleClosure(
				function,
				simpleClosureValue,
				dstPtrType,
				resultValue
				);
	}

	// case 3: closure object needs to be created (so conversion is required even if function signatures match)

	return llvmCast_FullClosure(
		opValue,
		srcFunctionType,
		dstPtrType,
		resultValue
		);
}

bool
Cast_FunctionPtr_Thin2Fat::llvmCast_NoThunkSimpleClosure(
	const Value& opValue,
	const Value& simpleClosureObjValue,
	FunctionType* srcFunctionType,
	FunctionPtrType* dstPtrType,
	Value* resultValue
	)
{
	Type* thisArgType = srcFunctionType->getThisArgType();

	Value thisArgValue;
	bool result = m_module->m_operatorMgr.castOperator(simpleClosureObjValue, thisArgType, &thisArgValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createClosureFunctionPtr(opValue, thisArgValue, dstPtrType, resultValue);
	return true;
}

bool
Cast_FunctionPtr_Thin2Fat::llvmCast_DirectThunkNoClosure(
	Function* function,
	FunctionPtrType* dstPtrType,
	Value* resultValue
	)
{
	Function* thunkFunction = m_module->m_functionMgr.getDirectThunkFunction(
		function,
		((FunctionPtrType*)dstPtrType)->getTargetType(),
		true
		);

	Value nullValue = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr)->getZeroValue();
	m_module->m_llvmIrBuilder.createClosureFunctionPtr(thunkFunction, nullValue, dstPtrType, resultValue);
	return true;
}

bool
Cast_FunctionPtr_Thin2Fat::llvmCast_DirectThunkSimpleClosure(
	Function* function,
	const Value& simpleClosureObjValue,
	FunctionPtrType* dstPtrType,
	Value* resultValue
	)
{
	Type* thisArgType = function->getType()->getThisArgType();
	DerivableType* thisTargetType = function->getType()->getThisTargetType();

	Value thisArgValue;
	bool result = m_module->m_operatorMgr.castOperator(simpleClosureObjValue, thisArgType, &thisArgValue);
	if (!result)
		return false;

	Function* thunkFunction = m_module->m_functionMgr.getDirectThunkFunction(
		function,
		m_module->m_typeMgr.getMemberMethodType(thisTargetType, dstPtrType->getTargetType())
		);

	m_module->m_llvmIrBuilder.createClosureFunctionPtr(thunkFunction, thisArgValue, dstPtrType, resultValue);
	return true;
}

bool
Cast_FunctionPtr_Thin2Fat::llvmCast_FullClosure(
	const Value& opValue,
	FunctionType* srcFunctionType,
	FunctionPtrType* dstPtrType,
	Value* resultValue
	)
{
	Value closureValue;
	bool result = m_module->m_operatorMgr.createClosureObject(
		opValue,
		dstPtrType->getTargetType(),
		dstPtrType->getPtrTypeKind() == FunctionPtrTypeKind_Weak,
		&closureValue
		);

	if (!result)
		return false;

	ASSERT(isClassPtrType(closureValue.getType(), ClassTypeKind_FunctionClosure));

	FunctionClosureClassType* closureType = (FunctionClosureClassType*)((ClassPtrType*)closureValue.getType())->getTargetType();
	m_module->m_llvmIrBuilder.createClosureFunctionPtr(closureType->getThunkFunction(), closureValue, dstPtrType, resultValue);
	return true;
}

//..............................................................................

bool
Cast_FunctionPtr_Thin2Thin::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	if (opValue.getClosure())
	{
		err::setFormatStringError("cannot create thin function pointer to a closure");
		return false;
	}

	FunctionPtrType* srcPtrType = (FunctionPtrType*)opValue.getType();
	FunctionPtrType* dstPtrType = (FunctionPtrType*)type;

	if (srcPtrType->getTargetType()->cmp(dstPtrType->getTargetType()) == 0)
	{
		resultValue->overrideType(opValue, type);
		return true;
	}

	if (opValue.getValueKind() != ValueKind_Function)
	{
		err::setFormatStringError("can only create thin pointer thunk to a function, not a function pointer");
		return false;
	}

	Function* thunkFunction = m_module->m_functionMgr.getDirectThunkFunction(
		opValue.getFunction(),
		dstPtrType->getTargetType()
		);

	resultValue->setFunction(thunkFunction);
	resultValue->overrideType(type);
	return true;
}

//..............................................................................

Cast_FunctionPtr::Cast_FunctionPtr()
{
	memset(m_operatorTable, 0, sizeof(m_operatorTable));

	m_operatorTable[FunctionPtrTypeKind_Normal][FunctionPtrTypeKind_Normal] = &m_fromFat;
	m_operatorTable[FunctionPtrTypeKind_Normal][FunctionPtrTypeKind_Weak]   = &m_fromFat;
	m_operatorTable[FunctionPtrTypeKind_Weak][FunctionPtrTypeKind_Normal]   = &m_weak2Normal;
	m_operatorTable[FunctionPtrTypeKind_Weak][FunctionPtrTypeKind_Weak]     = &m_fromFat;
	m_operatorTable[FunctionPtrTypeKind_Thin][FunctionPtrTypeKind_Normal]   = &m_thin2Fat;
	m_operatorTable[FunctionPtrTypeKind_Thin][FunctionPtrTypeKind_Weak]     = &m_thin2Fat;
	m_operatorTable[FunctionPtrTypeKind_Thin][FunctionPtrTypeKind_Thin]     = &m_thin2Thin;
}

CastOperator*
Cast_FunctionPtr::getCastOperator(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(type->getTypeKind() == TypeKind_FunctionPtr);

	FunctionPtrType* dstPtrType = (FunctionPtrType*)type;
	FunctionPtrTypeKind dstPtrTypeKind = dstPtrType->getPtrTypeKind();

	Type* srcType = opValue.getType();
	if (!srcType)
	{
		ASSERT(opValue.getValueKind() == ValueKind_Function && opValue.getFunction()->isOverloaded());
		ASSERT(dstPtrTypeKind >= 0 && dstPtrTypeKind < 2);

		return m_operatorTable[FunctionPtrTypeKind_Thin][dstPtrTypeKind];
	}

	TypeKind typeKind = srcType->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		return isClassPtrType(srcType, ClassTypeKind_Multicast) ? &m_fromMulticast : NULL;

	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		return &m_fromDataPtr;

	default:
		return NULL;
	}

	FunctionPtrType* srcPtrType = (FunctionPtrType*)srcType;
	FunctionPtrTypeKind srcPtrTypeKind = srcPtrType->getPtrTypeKind();

	ASSERT((size_t)srcPtrTypeKind < FunctionPtrTypeKind__Count);
	ASSERT((size_t)dstPtrTypeKind < FunctionPtrTypeKind__Count);

	return m_operatorTable[srcPtrTypeKind][dstPtrTypeKind];
}

//..............................................................................

CastKind
Cast_FunctionRef::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(type->getTypeKind() == TypeKind_FunctionRef);

	Type* intermediateSrcType = m_module->m_operatorMgr.getUnaryOperatorResultType(UnOpKind_Addr, opValue);
	if (!intermediateSrcType)
		return CastKind_None;

	FunctionPtrType* ptrType = (FunctionPtrType*)type;
	FunctionPtrType* intermediateDstType = ptrType->getTargetType()->getFunctionPtrType(
		TypeKind_FunctionPtr,
		ptrType->getPtrTypeKind(),
		ptrType->getFlags()
		);

	return m_module->m_operatorMgr.getCastKind(intermediateSrcType, intermediateDstType);
}

bool
Cast_FunctionRef::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(type->getTypeKind() == TypeKind_FunctionRef);

	FunctionPtrType* ptrType = (FunctionPtrType*)type;
	FunctionPtrType* intermediateType = ptrType->getTargetType()->getFunctionPtrType(
		TypeKind_FunctionPtr,
		ptrType->getPtrTypeKind(),
		ptrType->getFlags()
		);

	Value intermediateValue;

	return
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, opValue, &intermediateValue) &&
		m_module->m_operatorMgr.castOperator(&intermediateValue, intermediateType) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, intermediateValue, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
