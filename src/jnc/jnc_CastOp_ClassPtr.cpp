#include "pch.h"
#include "jnc_CastOp_ClassPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
isMulticastToMulticast (
	ClassPtrType* srcType,
	ClassPtrType* dstType
	)
{
	if (srcType->getTargetType ()->getClassTypeKind () != ClassTypeKind_Multicast ||
		dstType->getTargetType ()->getClassTypeKind () != ClassTypeKind_Multicast)
		return false;

	// event -> multicast is never ok

	if ((srcType->getFlags () & PtrTypeFlagKind_Event) && !(dstType->getFlags () & PtrTypeFlagKind_Event))
		return false;

	MulticastClassType* srcMcType = (MulticastClassType*) srcType->getTargetType ();
	MulticastClassType* dstMcType = (MulticastClassType*) dstType->getTargetType ();

	return srcMcType->getTargetType ()->cmp (dstMcType->getTargetType ()) == 0;
}

//.............................................................................

CastKind
Cast_ClassPtr::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_ClassPtr);

	if (opValue.getType ()->getTypeKind () != TypeKind_ClassPtr)
		return CastKind_None; // TODO: user conversions later via constructors

	ClassPtrType* srcType = (ClassPtrType*) opValue.getType ();
	ClassPtrType* dstType = (ClassPtrType*) type;

	if (srcType->isConstPtrType () && !dstType->isConstPtrType ()) 
		return CastKind_None; // const vs non-const mismatch

	ClassType* srcClassType = srcType->getTargetType ();
	ClassType* dstClassType = dstType->getTargetType ();

	return 
		(dstClassType->getClassTypeKind () == ClassTypeKind_StdObject) ||	
		srcClassType->cmp (dstClassType) == 0 || 
		isMulticastToMulticast (srcType, dstType) ||
		srcClassType->findBaseTypeTraverse (dstClassType) ? 
		CastKind_Implicit : 
		CastKind_Explicit;
}

bool
Cast_ClassPtr::llvmCast (
	StorageKind storageKind,
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () == TypeKind_ClassPtr);

	bool result;

	if (rawOpValue.getType ()->getTypeKind () != TypeKind_ClassPtr)
	{
		setCastError (rawOpValue, type);
		return false; // TODO: user conversions via constructors -- only if target ptr is EPtrTypeFlag_Const
	}

	Value opValue = rawOpValue;

	ClassPtrType* srcType = (ClassPtrType*) rawOpValue.getType ();
	ClassPtrType* dstType = (ClassPtrType*) type;

	if (srcType->getPtrTypeKind () == ClassPtrTypeKind_Weak &&
		dstType->getPtrTypeKind () != ClassPtrTypeKind_Weak)
	{
		LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "strengthen class pointer");

		Function* strengthen = m_module->m_functionMgr.getStdFunction (StdFuncKind_StrengthenClassPtr);

		m_module->m_llvmIrBuilder.createBitCast (opValue, m_module->getSimpleType (StdTypeKind_ObjectPtr), &opValue);
		m_module->m_llvmIrBuilder.createCall (
			strengthen,
			strengthen->getType (),
			opValue,
			&opValue
			);

		m_module->m_llvmIrBuilder.createBitCast (opValue, srcType, &opValue);
	}

	ClassType* srcClassType = srcType->getTargetType ();
	ClassType* dstClassType = dstType->getTargetType ();

	if (dstType->getFlags () & PtrTypeFlagKind_Safe)
		m_module->m_operatorMgr.checkClassPtrNull (opValue);

	if (dstClassType->getClassTypeKind () == ClassTypeKind_StdObject ||
		isMulticastToMulticast (srcType, dstType))
	{
		m_module->m_llvmIrBuilder.createBitCast (opValue, dstType, resultValue);
		return true;
	}

	if (srcClassType->cmp (dstClassType) == 0)
	{
		resultValue->overrideType (opValue, type);
		return true;
	}

	BaseTypeCoord coord;
	result = srcClassType->findBaseTypeTraverse (dstClassType, &coord);
	if (!result)
	{
		Value ptrValue;
		m_module->m_llvmIrBuilder.createBitCast (opValue, m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr), &ptrValue);

		Value typeValue (&dstClassType, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr));

		Function* dynamicCastClassPtr = m_module->m_functionMgr.getStdFunction (StdFuncKind_DynamicCastClassPtr);
		m_module->m_llvmIrBuilder.createCall2 (
			dynamicCastClassPtr,
			dynamicCastClassPtr->getType (),
			ptrValue,
			typeValue,
			&ptrValue
			);

		m_module->m_llvmIrBuilder.createBitCast (ptrValue, dstType, resultValue);
		return true;
	}

	Value srcNullValue = srcType->getZeroValue ();
	Value dstNullValue = dstType->getZeroValue ();

	BasicBlock* cmpBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock ("iface_phi");
	BasicBlock* noNullBlock = m_module->m_controlFlowMgr.createBlock ("iface_nonull");

	Value cmpValue;
	result = 
		m_module->m_operatorMgr.binaryOperator (BinOpKind_Eq, opValue, srcNullValue, &cmpValue) &&
		m_module->m_controlFlowMgr.conditionalJump (cmpValue, phiBlock, noNullBlock, noNullBlock);

	if (!result)
		return false;
	
	coord.m_llvmIndexArray.insert (0, 0);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep (
		opValue, 
		coord.m_llvmIndexArray,
		coord.m_llvmIndexArray.getCount (),
		NULL, 
		&ptrValue
		);		

	m_module->m_controlFlowMgr.follow (phiBlock);

	m_module->m_llvmIrBuilder.createPhi (ptrValue, noNullBlock, dstNullValue, cmpBlock, resultValue);
	resultValue->overrideType (dstType);
	return true;
}

//.............................................................................

} // namespace jnc {
