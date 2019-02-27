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
#include "jnc_ct_CastOp_DataPtr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_DataPtr_FromArray::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	if (isArrayRefType(opValue.getType()))
	{
		Value ptrValue = m_module->m_operatorMgr.prepareOperandType(opValue, OpFlag_ArrayRefToPtr);
		return m_module->m_operatorMgr.getCastKind(ptrValue, type);
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Array);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	ArrayType* srcType = (ArrayType*)opValue.getType();
	DataPtrType* dstType = (DataPtrType*)type;

	Type* arrayElementType = srcType->getElementType();
	Type* ptrDataType = dstType->getTargetType();

	return
		arrayElementType->cmp(ptrDataType) == 0 ? CastKind_Implicit :
		(arrayElementType->getFlags() & TypeFlag_Pod) ?
			ptrDataType->getTypeKind() == TypeKind_Void ? CastKind_Implicit :
			(ptrDataType->getFlags() & TypeFlag_Pod) ? CastKind_Explicit : CastKind_None : CastKind_None;
}

bool
Cast_DataPtr_FromArray::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	if (isArrayRefType(opValue.getType()))
	{
		Value ptrValue;
		bool result =
			m_module->m_operatorMgr.prepareOperand(opValue, &ptrValue, OpFlag_ArrayRefToPtr) &&
			m_module->m_operatorMgr.castOperator(&ptrValue, type);

		if (!result)
			return false;

		const void* p = ptrValue.getConstData();
		if (((DataPtrType*)type)->getPtrTypeKind() == DataPtrTypeKind_Normal)
			*(DataPtr*)dst = *(DataPtr*)p;
		else // thin or lean
			*(void**) dst = *(void**) p;

		return true;
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Array);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	ArrayType* srcType = (ArrayType*)opValue.getType();
	DataPtrType* dstType = (DataPtrType*)type;

	const Value& savedOpValue = m_module->m_constMgr.saveValue(opValue);
	const void* p = savedOpValue.getConstData();

	AXL_TODO("create a global constant holding the array")

	if (dstType->getPtrTypeKind() == DataPtrTypeKind_Normal)
	{
		DataPtr* ptr = (DataPtr*)dst;
		ptr->m_p = (void*)p;
		ptr->m_validator = m_module->m_constMgr.createConstDataPtrValidator(p, srcType);
	}
	else // thin or lean
	{
		*(const void**) dst = p;
	}

	return true;
}

bool
Cast_DataPtr_FromArray::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	if (isArrayRefType(opValue.getType()))
	{
		Value ptrValue;

		return
			m_module->m_operatorMgr.prepareOperand(opValue, &ptrValue, OpFlag_ArrayRefToPtr) &&
			m_module->m_operatorMgr.castOperator(ptrValue, type, resultValue);
	}

	err::setFormatStringError("casting from array to pointer is currently only implemented for constants");
	return false;
}

//..............................................................................

CastKind
Cast_DataPtr_FromClassPtr::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	ClassPtrType* srcType = (ClassPtrType*)opValue.getType();

	return
		(srcType->getFlags() & PtrTypeFlag_Const) && !(srcType->getFlags() & PtrTypeFlag_Const) ? CastKind_None :
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin ? CastKind_None :
		dstType->getTargetType()->getTypeKind() == TypeKind_Void ? CastKind_ImplicitCrossFamily :
		CastKind_Explicit;
}

bool
Cast_DataPtr_FromClassPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	ClassPtrType* srcType = (ClassPtrType*)opValue.getType();

	if ((srcType->getFlags() & PtrTypeFlag_Const) && !(srcType->getFlags() & PtrTypeFlag_Const))
	{
		setCastError(opValue, type);
		return false;
	}

	if (dstType->getPtrTypeKind() == DataPtrTypeKind_Thin)
	{
		err::setFormatStringError("casting from class pointer to fat data pointer is not yet implemented (thin only for now)");
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
Cast_DataPtr_FromFunctionPtr::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	FunctionPtrType* srcType = (FunctionPtrType*)opValue.getType();

	return
		srcType->getPtrTypeKind() != FunctionPtrTypeKind_Thin ? CastKind_None :
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin ? CastKind_None :
		dstType->getTargetType()->getTypeKind() == TypeKind_Void ? CastKind_ImplicitCrossFamily :
		CastKind_Explicit;
}

bool
Cast_DataPtr_FromFunctionPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	FunctionPtrType* srcType = (FunctionPtrType*)opValue.getType();

	if (srcType->getPtrTypeKind() != FunctionPtrTypeKind_Thin ||
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin)
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
Cast_DataPtr_FromPropertyPtr::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	PropertyPtrType* srcType = (PropertyPtrType*)opValue.getType();

	return
		srcType->getPtrTypeKind() != PropertyPtrTypeKind_Thin ? CastKind_None :
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin ? CastKind_None :
		dstType->getTargetType()->getTypeKind() == TypeKind_Void ? CastKind_ImplicitCrossFamily :
		CastKind_Explicit;
}

bool
Cast_DataPtr_FromPropertyPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	PropertyPtrType* srcType = (PropertyPtrType*)opValue.getType();

	if (srcType->getPtrTypeKind() != PropertyPtrTypeKind_Thin ||
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin)
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
Cast_DataPtr_Base::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* srcType = (DataPtrType*)opValue.getType();
	DataPtrType* dstType = (DataPtrType*)type;

	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	if (isSrcConst && !isDstConst)
		return CastKind_None; // const vs non-const mismatch

	Type* srcDataType = srcType->getTargetType();
	Type* dstDataType = dstType->getTargetType();

	if (srcDataType->cmp(dstDataType) == 0)
		return CastKind_Implicit;

	bool isSrcPod = (srcDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstPod = (dstDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstDerivable = (dstDataType->getTypeKindFlags() & TypeKindFlag_Derivable) != 0;
	bool canCastToPod = isSrcPod || isDstConst || dstType->getPtrTypeKind() == DataPtrTypeKind_Thin;

	if (dstDataType->getStdType() == StdType_AbstractData ||
		dstDataType->getTypeKind() == TypeKind_Void && canCastToPod)
		return CastKind_Implicit;

	if (srcDataType->getTypeKind() == TypeKind_Void &&
		(dstDataType->getTypeKind() == TypeKind_Int8 || dstDataType->getTypeKind() == TypeKind_Int8_u))
		return CastKind_Implicit;

	if ((srcDataType->getTypeKindFlags() & TypeKindFlag_Integer) &&
		(dstDataType->getTypeKindFlags() & TypeKindFlag_Integer) &&
		srcDataType->getSize() == dstDataType->getSize())
		return CastKind_Implicit;

	bool isDstBase =
		srcDataType->getTypeKind() == TypeKind_Struct &&
		((StructType*)srcDataType)->findBaseTypeTraverse(dstDataType);

	return
		isDstBase ? CastKind_Implicit :
		isDstPod && canCastToPod ? CastKind_Explicit :
		isDstDerivable ? CastKind_Dynamic : CastKind_None;
}

size_t
Cast_DataPtr_Base::getOffset(
	DataPtrType* srcType,
	DataPtrType* dstType,
	BaseTypeCoord* coord
	)
{
	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	if (isSrcConst && !isDstConst)
	{
		setCastError(srcType, dstType);
		return -1;
	}

	Type* srcDataType = srcType->getTargetType();
	Type* dstDataType = dstType->getTargetType();

	if (srcDataType->cmp(dstDataType) == 0)
		return 0;

	bool isSrcPod = (srcDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstPod = (dstDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstDerivable = (dstDataType->getTypeKindFlags() & TypeKindFlag_Derivable) != 0;
	bool canCastToPod = isSrcPod || isDstConst || dstType->getPtrTypeKind() == DataPtrTypeKind_Thin;

	if (dstDataType->getStdType() == StdType_AbstractData ||
		dstDataType->getTypeKind() == TypeKind_Void && canCastToPod)
		return 0;

	bool isDstBase =
		srcDataType->getTypeKind() == TypeKind_Struct &&
		((StructType*)srcDataType)->findBaseTypeTraverse(dstDataType, coord);

	if (isDstBase)
		return coord->m_offset;

	if (isDstPod && canCastToPod)
		return 0;

	CastKind castKind = isDstDerivable? CastKind_Dynamic : CastKind_None;
	setCastError(srcType, dstType, castKind);
	return -1;
}

bool
Cast_DataPtr_Base::getOffsetUnsafePtrValue(
	const Value& ptrValue,
	DataPtrType* srcType,
	DataPtrType* dstType,
	bool isFat,
	Value* resultValue
	)
{
	BaseTypeCoord coord;

	size_t offset = getOffset(srcType, dstType, &coord);
	if (offset == -1)
		return false;

	if (isFat)
		dstType = (DataPtrType*)m_module->m_typeMgr.getStdType(StdType_BytePtr);
	else if (dstType->getPtrTypeKind() != DataPtrTypeKind_Thin)
		dstType = dstType->getTargetType()->getDataPtrType_c();

	if (!coord.m_llvmIndexArray.isEmpty())
	{
		coord.m_llvmIndexArray.insert(0, 0);

		srcType = srcType->getTargetType()->getDataPtrType_c();

		Value tmpValue;
		m_module->m_llvmIrBuilder.createBitCast(ptrValue, srcType, &tmpValue);

		m_module->m_llvmIrBuilder.createGep(
			tmpValue,
			coord.m_llvmIndexArray,
			coord.m_llvmIndexArray.getCount(),
			dstType,
			&tmpValue
			);

		if (isFat)
			m_module->m_llvmIrBuilder.createBitCast(tmpValue, dstType, resultValue);

		return true;
	}

	ASSERT(offset == 0);

	m_module->m_llvmIrBuilder.createBitCast(ptrValue, dstType, resultValue);
	return true;
}

//..............................................................................

bool
Cast_DataPtr_Normal2Normal::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	DataPtr* dstPtr = (DataPtr*)dst;
	DataPtr* srcPtr = (DataPtr*)opValue.getConstData();
	dstPtr->m_p = (char*)srcPtr->m_p + offset;
	dstPtr->m_validator = srcPtr->m_validator;
	return true;
}

bool
Cast_DataPtr_Normal2Normal::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	bool result;

	if (type->getFlags() & PtrTypeFlag_Safe)
	{
		result = m_module->m_operatorMgr.checkDataPtrRange(opValue);
		if (!result)
			return false;
	}

	BaseTypeCoord coord;
	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, &coord);
	if (offset == -1)
		return false;

	if (!offset)
	{
		resultValue->overrideType(opValue, type);
		return true;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &ptrValue);

	result = getOffsetUnsafePtrValue(ptrValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, true, &ptrValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createInsertValue(opValue, ptrValue, 0, type, resultValue);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Lean2Normal::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* srcPtrType = (DataPtrType*)opValue.getType();
	ASSERT(srcPtrType->getPtrTypeKind() == DataPtrTypeKind_Lean);

	size_t offset = getOffset(srcPtrType, (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	DataPtr* dstPtr = (DataPtr*)dst;
	const void* src = opValue.getConstData();
	const void* p = (char*)src + offset;

	dstPtr->m_p = (void*)p;
	dstPtr->m_validator = m_module->m_constMgr.createConstDataPtrValidator(p, srcPtrType->getTargetType());
	return true;
}

bool
Cast_DataPtr_Lean2Normal::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	bool result;

	DataPtrType* srcPtrType = (DataPtrType*)opValue.getType();
	ASSERT(srcPtrType->getPtrTypeKind() == DataPtrTypeKind_Lean);

	Value ptrValue;
	result = getOffsetUnsafePtrValue(opValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, true, &ptrValue);
	if (!result)
		return false;

	if (type->getFlags() & PtrTypeFlag_Safe)
	{
		result = m_module->m_operatorMgr.checkDataPtrRange(opValue);
		if (!result)
			return false;
	}

	LeanDataPtrValidator* validator = opValue.getLeanDataPtrValidator();
	Value validatorValue = validator->getValidatorValue();

	Value tmpValue = type->getUndefValue();
	m_module->m_llvmIrBuilder.createInsertValue(tmpValue, ptrValue, 0, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue(tmpValue, validatorValue, 1, type, resultValue);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Normal2Thin::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	*(char**) dst = *(char**) opValue.getConstData() + offset;
	return true;
}

bool
Cast_DataPtr_Normal2Thin::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &ptrValue);
	return getOffsetUnsafePtrValue(ptrValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, false, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Lean2Thin::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	return getOffsetUnsafePtrValue(opValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, false, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Thin2Thin::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	*(char**) dst = *(char**) opValue.getConstData() + offset;
	return true;
}

//..............................................................................

Cast_DataPtr::Cast_DataPtr()
{
	memset(m_operatorTable, 0, sizeof(m_operatorTable));

	m_operatorTable[DataPtrTypeKind_Normal] [DataPtrTypeKind_Normal] = &m_normal2Normal;
	m_operatorTable[DataPtrTypeKind_Normal] [DataPtrTypeKind_Thin]   = &m_normal2Thin;
	m_operatorTable[DataPtrTypeKind_Lean] [DataPtrTypeKind_Normal]   = &m_lean2Normal;
	m_operatorTable[DataPtrTypeKind_Lean] [DataPtrTypeKind_Thin]     = &m_lean2Thin;
	m_operatorTable[DataPtrTypeKind_Thin] [DataPtrTypeKind_Thin]     = &m_thin2Thin;

	m_opFlags = OpFlag_KeepDerivableRef;
}

CastOperator*
Cast_DataPtr::getCastOperator(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstPtrType = (DataPtrType*)type;
	DataPtrTypeKind dstPtrTypeKind = dstPtrType->getPtrTypeKind();

	Type* srcType = opValue.getType();
	if (isArrayRefType(srcType))
		return &m_fromArray;

	TypeKind typeKind = srcType->getTypeKind();
	switch(typeKind)
	{
	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		break;

	case TypeKind_Array:
		return &m_fromArray;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		return &m_fromClassPtr;

	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
		return &m_fromFunctionPtr;

	case TypeKind_PropertyPtr:
	case TypeKind_PropertyRef:
		return &m_fromPropertyPtr;

	default:
		return NULL;
	}

	DataPtrType* srcPtrType = (DataPtrType*)srcType;
	DataPtrTypeKind srcPtrTypeKind = srcPtrType->getPtrTypeKind();

	if (dstPtrTypeKind == DataPtrTypeKind_Normal &&
		(srcPtrType->getFlags() & PtrTypeFlag_Const) &&
		!(dstPtrType->getFlags() & PtrTypeFlag_Const))
		return NULL;

	ASSERT((size_t)srcPtrTypeKind < DataPtrTypeKind__Count);
	ASSERT((size_t)dstPtrTypeKind < DataPtrTypeKind__Count);

	return m_operatorTable[srcPtrTypeKind] [dstPtrTypeKind];
}

//..............................................................................

CastKind
Cast_DataRef::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(type->getTypeKind() == TypeKind_DataRef);

	Type* intermediateSrcType = m_module->m_operatorMgr.getUnaryOperatorResultType(UnOpKind_Addr, opValue);
	if (!intermediateSrcType)
		return CastKind_None;

	DataPtrType* ptrType = (DataPtrType*)type;
	DataPtrType* intermediateDstType = ptrType->getTargetType()->getDataPtrType(
		TypeKind_DataPtr,
		ptrType->getPtrTypeKind(),
		ptrType->getFlags()
		);

	return m_module->m_operatorMgr.getCastKind(intermediateSrcType, intermediateDstType);
}

bool
Cast_DataRef::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT(type->getTypeKind() == TypeKind_DataRef);

	DataPtrType* ptrType = (DataPtrType*)type;
	DataPtrType* intermediateType = ptrType->getTargetType()->getDataPtrType(
		TypeKind_DataPtr,
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
