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
#include "jnc_ct_CastOp_ClassPtr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

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

	if ((srcType->getFlags () & PtrTypeFlag_Event) && !(dstType->getFlags () & PtrTypeFlag_Event))
		return false;

	MulticastClassType* srcMcType = (MulticastClassType*) srcType->getTargetType ();
	MulticastClassType* dstMcType = (MulticastClassType*) dstType->getTargetType ();

	return srcMcType->getTargetType ()->cmp (dstMcType->getTargetType ()) == 0;
}

//..............................................................................

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

	if ((srcType->getFlags () & PtrTypeFlag_Const) && 
		!(dstType->getFlags () & PtrTypeFlag_Const))
		return CastKind_None; // const vs non-const mismatch

	ClassType* srcClassType = srcType->getTargetType ();
	ClassType* dstClassType = dstType->getTargetType ();

	return
		(dstClassType->getClassTypeKind () == ClassTypeKind_Abstract) ||
		srcClassType->cmp (dstClassType) == 0 ||
		isMulticastToMulticast (srcType, dstType) ||
		srcClassType->findBaseTypeTraverse (dstClassType) ?
		CastKind_Implicit :
		CastKind_Dynamic;
}

bool
Cast_ClassPtr::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (type->getTypeKind () == TypeKind_ClassPtr);

	bool result;

	if (opValue.getType ()->getTypeKind () != TypeKind_ClassPtr)
		return false; // TODO: user conversions later via constructors

	IfaceHdr* srcIface = *(IfaceHdr**) opValue.getConstData ();
	ClassType* srcClassType = (ClassType*) srcIface->m_box->m_type;

	ClassPtrType* srcType = (ClassPtrType*) opValue.getType ();
	ClassPtrType* dstType = (ClassPtrType*) type;
	ClassType* dstClassType = dstType->getTargetType ();

	if (dstType->getFlags () & PtrTypeFlag_Safe)
		m_module->m_operatorMgr.checkNullPtr (opValue);

	if (srcIface == NULL ||
		dstClassType->getClassTypeKind () == ClassTypeKind_Abstract ||
		isMulticastToMulticast (srcType, dstType) ||
		srcClassType->cmp (dstClassType) == 0)
	{
		*(void**) dst = srcIface;
		return true;
	}

	BaseTypeCoord coord;
	result = srcClassType->findBaseTypeTraverse (dstClassType, &coord);
	if (!result)
		return false;
		 
	*(void**) dst = (char*) srcIface + coord.m_offset;
	return true;
}

bool
Cast_ClassPtr::llvmCast (
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
		return false; // TODO: user conversions via constructors -- only if target ptr is PtrTypeFlag_Const
	}

	Value opValue = rawOpValue;

	ClassPtrType* srcType = (ClassPtrType*) rawOpValue.getType ();
	ClassPtrType* dstType = (ClassPtrType*) type;

	if (srcType->getPtrTypeKind () == ClassPtrTypeKind_Weak &&
		dstType->getPtrTypeKind () != ClassPtrTypeKind_Weak)
	{
		Function* strengthen = m_module->m_functionMgr.getStdFunction (StdFunc_StrengthenClassPtr);

		m_module->m_llvmIrBuilder.createBitCast (opValue, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &opValue);
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

	if (dstType->getFlags () & PtrTypeFlag_Safe)
		m_module->m_operatorMgr.checkNullPtr (opValue);

	if (dstClassType->getClassTypeKind () == ClassTypeKind_Abstract ||
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
		setCastError (opValue, type, CastKind_Dynamic);
		return false;
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

//..............................................................................

} // namespace ct
} // namespace jnc
