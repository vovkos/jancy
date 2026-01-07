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
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_LeanDataPtrValidator.h"
#include "jnc_ct_UnOp_Ptr.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_DataPtr_FromArray::getCastKind(
	const Value& opValue,
	Type* type
) {
	if (isArrayRefType(opValue.getType())) {
		Type* ptrType = m_module->m_operatorMgr.prepareArrayRefType((DataPtrType*)opValue.getType());
		return m_module->m_operatorMgr.getCastKind(ptrType, type);
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Array);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	ArrayType* srcType = (ArrayType*)opValue.getType();
	DataPtrType* dstType = (DataPtrType*)type;

	if (opValue.getValueKind() == ValueKind_Const && !(dstType->getFlags() & PtrTypeFlag_Const))
		return CastKind_None;

	Type* arrayElementType = srcType->getElementType();
	Type* ptrDataType = dstType->getTargetType();

	return
		arrayElementType->isEqual(ptrDataType) ? CastKind_Implicit :
		(arrayElementType->getFlags() & TypeFlag_Pod) ?
			ptrDataType->getTypeKind() == TypeKind_Void ? CastKind_Implicit :
			(ptrDataType->getFlags() & TypeFlag_Pod) ? CastKind_Explicit : CastKind_None : CastKind_None;
}

bool
Cast_DataPtr_FromArray::cast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	if (opValue.getValueKind() != ValueKind_Const)
		return llvmCast(opValue, type, resultValue);

	if (m_module->isConstOperatorOnly())
		return constCastImpl(opValue, type, resultValue);

	Type* opType = opValue.getType();
	Variable* constVar = isCharArrayType(opType) ?
		m_module->m_variableMgr.getStaticLiteralVariable(sl::StringRef((char*)opValue.getConstData(), opType->getSize())) :
		m_module->m_variableMgr.createSimpleStaticVariable("const", opType, opValue, PtrTypeFlag_Const);

	return llvmCast(constVar, type, resultValue);
}

bool
Cast_DataPtr_FromArray::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	if (isArrayRefType(opValue.getType())) {
		Value ptrValue;
		m_module->m_operatorMgr.prepareArrayRef(opValue, &ptrValue);

		bool result = m_module->m_operatorMgr.castOperator(&ptrValue, type);
		if (!result)
			return false;

		const void* p = ptrValue.getConstData();
		if (((DataPtrType*)type)->getPtrTypeKind() == DataPtrTypeKind_Normal)
			*(DataPtr*)dst = *(DataPtr*)p;
		else // thin or lean
			*(void**)dst = *(void**)p;

		return true;
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Array);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	if (!(dstType->getFlags() & PtrTypeFlag_Const)) {
		setCastError(opValue, type);
		return false;
	}

	if (dstType->getPtrTypeKind() == DataPtrTypeKind_Normal) {
		DataPtr ptr = m_module->m_operatorMgr.createDataPtrToConst(opValue);
		if (!ptr.m_p)
			return false;

		*(DataPtr*)dst = ptr;
	} else { // thin or lean
		void* p = (void*)m_module->m_operatorMgr.createThinDataPtrToConst(opValue);
		if (!p)
			return false;

		*(void**)dst = p;
	}

	return true;
}

bool
Cast_DataPtr_FromArray::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(isArrayRefType(opValue.getType()));

	Value ptrValue;
	m_module->m_operatorMgr.prepareArrayRef(opValue, &ptrValue);
	return m_module->m_operatorMgr.castOperator(ptrValue, type, resultValue);
}

//..............................................................................

CastKind
Cast_DataPtr_FromString::getCastKind(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	ASSERT(
		opValue.getType()->getTypeKind() == TypeKind_String ||
		opValue.getType()->getTypeKind() == TypeKind_DataRef &&
		((DataPtrType*)opValue.getType())->getTargetType()->getTypeKind() == TypeKind_String
	);

	DataPtrType* dstType = (DataPtrType*)type;
	if (!(dstType->getFlags() & PtrTypeFlag_Const))
		return CastKind_None;

	Type* targetType = dstType->getTargetType();
	switch (targetType->getTypeKind()) {
	case TypeKind_Void:
	case TypeKind_Char:
	case TypeKind_Byte:
		return CastKind_Implicit;

	default:
		return (targetType->getFlags() & TypeFlag_Pod) ?
			CastKind_Explicit :
			CastKind_None;
	}
}

bool
Cast_DataPtr_FromString::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	if (!(dstType->getFlags() & PtrTypeFlag_Const)) {
		setCastError(opValue, type);
		return false;
	}

	if (opValue.getType()->getTypeKind() == TypeKind_DataRef) {
		ASSERT(((DataPtrType*)opValue.getType())->getTargetType()->getTypeKind() == TypeKind_String);
		err::setError("casting from string_t reference constants not supported");
		return false;
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_String);
	String* string = (String*)opValue.getConstData();
	ASSERT(string->m_ptr.m_p == string->m_ptr_sz.m_p); // constants are always null-terminated

	if (dstType->getPtrTypeKind() == DataPtrTypeKind_Normal)
		*(DataPtr*)dst = string->m_ptr;
	else
		*(void**)dst = string->m_ptr.m_p;

	return true;
}

bool
Cast_DataPtr_FromString::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	Value charPtrValue;

	if (opValue.getType()->getTypeKind() == TypeKind_DataRef) {
		ASSERT(((DataPtrType*)opValue.getType())->getTargetType()->getTypeKind() == TypeKind_String);

		Function* func = m_module->m_functionMgr.getStdFunction(StdFunc_StringRefSz);
		Value stringPtrValue;

		return
			m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, opValue, &stringPtrValue) &&
			m_module->m_operatorMgr.callOperator(func, stringPtrValue, &charPtrValue) &&
			m_module->m_operatorMgr.castOperator(charPtrValue, type, resultValue);
	} else {
		ASSERT(opValue.getType()->getTypeKind() == TypeKind_String);

		Function* func = m_module->m_functionMgr.getStdFunction(StdFunc_StringSz);

		return
			m_module->m_operatorMgr.callOperator(func, opValue, &charPtrValue) &&
			m_module->m_operatorMgr.castOperator(charPtrValue, type, resultValue);
	}
}

//..............................................................................

bool
Cast_DataPtr_FromRvalue::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	return err::fail("TODO: implement rvalue -> const pointer const cast");
}

bool
Cast_DataPtr_FromRvalue::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(
		type->getTypeKind() == TypeKind_DataPtr &&
		(type->getFlags() & PtrTypeFlag_Const) &&
		(opValue.getType()->getTypeKindFlags() & TypeKindFlag_Derivable)
	);

	Value rvalueStorage = m_module->m_variableMgr.createSimpleStackVariable("rvalue_storage", opValue.getType());
	bool result = m_module->m_operatorMgr.storeDataRef(rvalueStorage, opValue);
	ASSERT(result);

	return m_module->m_operatorMgr.castOperator(rvalueStorage, type, resultValue);
}

//..............................................................................

CastKind
Cast_DataPtr_FromClassPtr::getCastKind(
	const Value& opValue,
	Type* type
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	ClassPtrType* srcType = (ClassPtrType*)opValue.getType();

	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	return
		isSrcConst && !isDstConst ? CastKind_None :
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin ? CastKind_None :
		dstType->getTargetType()->getTypeKind() == TypeKind_Void ? CastKind_ImplicitCrossFamily :
		CastKind_Explicit;
}

bool
Cast_DataPtr_FromClassPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	ClassPtrType* srcType = (ClassPtrType*)opValue.getType();

	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	if (isSrcConst && !isDstConst) {
		setCastError(opValue, type);
		return false;
	}

	if (dstType->getPtrTypeKind() == DataPtrTypeKind_Thin) {
		err::setError("casting from class pointer to fat data pointer is not yet implemented (thin only for now)");
		return false;
	}

	if (!m_module->m_operatorMgr.isUnsafeRgn()) {
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
) {
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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	FunctionPtrType* srcType = (FunctionPtrType*)opValue.getType();

	if (srcType->getPtrTypeKind() != FunctionPtrTypeKind_Thin ||
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin) {
		setCastError(opValue, type);
		return false;
	}

	if (!m_module->m_operatorMgr.isUnsafeRgn()) {
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
) {
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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstType = (DataPtrType*)type;
	PropertyPtrType* srcType = (PropertyPtrType*)opValue.getType();

	if (srcType->getPtrTypeKind() != PropertyPtrTypeKind_Thin ||
		dstType->getPtrTypeKind() != DataPtrTypeKind_Thin) {
		setCastError(opValue, type);
		return false;
	}

	if (!m_module->m_operatorMgr.isUnsafeRgn()) {
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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* srcType = (DataPtrType*)opValue.getType();
	DataPtrType* dstType = (DataPtrType*)type;

	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	if (isSrcConst && !isDstConst)
		return CastKind_None; // const vs non-const mismatch

	CastKind implicitCastKind = isDstConst != isSrcConst ?
		CastKind_ImplicitCrossConst :
		CastKind_Implicit;

	Type* srcDataType = srcType->getTargetType();
	Type* dstDataType = dstType->getTargetType();

	if (srcDataType->isEqual(dstDataType))
		return implicitCastKind;

	bool result = srcDataType->ensureLayout() && dstDataType->ensureLayout();
	if (!result)
		return CastKind_None;

	bool isSrcPod = (srcDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstPod = (dstDataType->getFlags() & TypeFlag_Pod) != 0;
	bool isDstDerivable = (dstDataType->getTypeKindFlags() & TypeKindFlag_Derivable) != 0;
	bool canCastToPod = isSrcPod || isDstConst || dstType->getPtrTypeKind() == DataPtrTypeKind_Thin;

	if (dstDataType->getStdType() == StdType_AbstractData ||
		dstDataType->getTypeKind() == TypeKind_Void && canCastToPod)
		return implicitCastKind;

	if (srcDataType->getTypeKind() == TypeKind_Void &&
		(dstDataType->getTypeKind() == TypeKind_Int8 || dstDataType->getTypeKind() == TypeKind_Int8_u))
		return implicitCastKind;

	if ((srcDataType->getTypeKindFlags() & TypeKindFlag_Integer) &&
		(dstDataType->getTypeKindFlags() & TypeKindFlag_Integer) &&
		srcDataType->getSize() == dstDataType->getSize())
		return implicitCastKind;

	bool isDstBase =
		srcDataType->getTypeKind() == TypeKind_Struct &&
		((StructType*)srcDataType)->findBaseTypeTraverse(dstDataType);

	return
		isDstBase ? implicitCastKind :
		isDstPod && canCastToPod ? CastKind_Explicit :
		isDstDerivable ? CastKind_Dynamic : CastKind_None;
}

size_t
Cast_DataPtr_Base::getOffset(
	DataPtrType* srcType,
	DataPtrType* dstType,
	BaseTypeCoord* coord
) {
	bool isSrcConst = (srcType->getFlags() & PtrTypeFlag_Const) != 0;
	bool isDstConst = (dstType->getFlags() & PtrTypeFlag_Const) != 0;

	if (isSrcConst && !isDstConst) {
		setCastError(srcType, dstType);
		return -1;
	}

	Type* srcDataType = srcType->getTargetType();
	Type* dstDataType = dstType->getTargetType();

	if (srcDataType->isEqual(dstDataType))
		return 0;

	bool result = srcDataType->ensureLayout() && dstDataType->ensureLayout();
	if (!result)
		return -1;

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
) {
	BaseTypeCoord coord;

	size_t offset = getOffset(srcType, dstType, &coord);
	if (offset == -1)
		return false;

	if (isFat)
		dstType = (DataPtrType*)m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
	else if (dstType->getPtrTypeKind() != DataPtrTypeKind_Thin)
		dstType = dstType->getTargetType()->getDataPtrType_c();

	if (!coord.m_llvmIndexArray.isEmpty()) {
		coord.m_llvmIndexArray.insert(0, 0);

		Type* targetType = srcType->getTargetType();
		srcType = targetType->getDataPtrType_c();

		Value typedPtrValue;
		m_module->m_llvmIrBuilder.createBitCast(ptrValue, srcType, &typedPtrValue);

		m_module->m_llvmIrBuilder.createGep(
			typedPtrValue,
			targetType,
			coord.m_llvmIndexArray,
			coord.m_llvmIndexArray.getCount(),
			dstType,
			resultValue
		);

		if (isFat)
			m_module->m_llvmIrBuilder.createBitCast(*resultValue, dstType, resultValue);

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
) {
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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	bool result;

	if (type->getFlags() & PtrTypeFlag_Safe) {
		result = m_module->m_operatorMgr.checkDataPtrRange(opValue);
		if (!result)
			return false;
	}

	BaseTypeCoord coord;
	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, &coord);
	if (offset == -1)
		return false;

	if (!offset) {
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
Cast_DataPtr_Lean2Normal::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	bool result;

	DataPtrType* srcPtrType = (DataPtrType*)opValue.getType();
	ASSERT(srcPtrType->getPtrTypeKind() == DataPtrTypeKind_Lean);

	Value ptrValue;
	result = getOffsetUnsafePtrValue(opValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, true, &ptrValue);
	if (!result)
		return false;

	if (type->getFlags() & PtrTypeFlag_Safe) {
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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	*(char**)dst = *(char**)opValue.getConstData() + offset;
	return true;
}

bool
Cast_DataPtr_Normal2Thin::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	if (type->getFlags() & PtrTypeFlag_Safe) {
		bool result = m_module->m_operatorMgr.checkDataPtrRange(opValue);
		if (!result)
			return false;
	}

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
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	if (type->getFlags() & PtrTypeFlag_Safe) {
		bool result = m_module->m_operatorMgr.checkDataPtrRange(opValue);
		if (!result)
			return false;
	}

	return getOffsetUnsafePtrValue(opValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, false, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Thin2Thin::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	size_t offset = getOffset((DataPtrType*)opValue.getType(), (DataPtrType*)type, NULL);
	if (offset == -1)
		return false;

	*(char**)dst = *(char**)opValue.getConstData() + offset;
	return true;
}

bool
Cast_DataPtr_Thin2Thin::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	if (type->getFlags() & PtrTypeFlag_Safe)
		if (!(opValue.getType()->getFlags() & PtrTypeFlag_Safe)) {
			err::setFormatStringError("cannot validate '%s'", opValue.getType()->getTypeString().sz());
			return false;
		}

	return getOffsetUnsafePtrValue(opValue, (DataPtrType*)opValue.getType(), (DataPtrType*)type, false, resultValue);
}

//..............................................................................

Cast_DataPtr::Cast_DataPtr() {
	memset(m_operatorTable, 0, sizeof(m_operatorTable));

	m_operatorTable[DataPtrTypeKind_Normal][DataPtrTypeKind_Normal] = &m_normal2Normal;
	m_operatorTable[DataPtrTypeKind_Normal][DataPtrTypeKind_Thin]   = &m_normal2Thin;
	m_operatorTable[DataPtrTypeKind_Lean][DataPtrTypeKind_Normal]   = &m_lean2Normal;
	m_operatorTable[DataPtrTypeKind_Lean][DataPtrTypeKind_Thin]     = &m_lean2Thin;
	m_operatorTable[DataPtrTypeKind_Thin][DataPtrTypeKind_Thin]     = &m_thin2Thin;

	m_opFlags = OpFlag_KeepDerivableRef | OpFlag_KeepStringRef;
}

CastOperator*
Cast_DataPtr::getCastOperator(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_DataPtr);

	DataPtrType* dstPtrType = (DataPtrType*)type;
	DataPtrTypeKind dstPtrTypeKind = dstPtrType->getPtrTypeKind();

	Type* srcType = opValue.getType();
	TypeKind typeKind = srcType->getTypeKind();
	switch (typeKind) {
	case TypeKind_DataPtr:
		break;

	case TypeKind_DataRef:
		switch (((DataPtrType*)srcType)->getTargetType()->getTypeKind()) {
		case TypeKind_Array:
			return &m_fromArray;

		case TypeKind_String:
			return &m_fromString;
		}
		break;

	case TypeKind_Array:
		return &m_fromArray;

	case TypeKind_String:
		return &m_fromString;

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
		if ((dstPtrType->getFlags() & PtrTypeFlag_Const) &&
			(srcType->getTypeKindFlags() & TypeKindFlag_Derivable) &&
			srcType->isEqual(dstPtrType->getTargetType())
		)
			return &m_fromRvalue;

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

	return m_operatorTable[srcPtrTypeKind][dstPtrTypeKind];
}

//..............................................................................

CastKind
Cast_DataRef::getCastKind(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_DataRef);

	Type* intermediateSrcType = UnOp_Addr::getResultType(opValue);
	if (!intermediateSrcType)
		return CastKind_None;

	DataPtrType* ptrType = (DataPtrType*)type;
	DataPtrType* intermediateDstType = ptrType->getTargetType()->getDataPtrType(
		TypeKind_DataPtr,
		ptrType->getPtrTypeKind(),
		ptrType->getFlags() & PtrTypeFlag__All
	);

	return m_module->m_operatorMgr.getCastKind(intermediateSrcType, intermediateDstType);
}

bool
Cast_DataRef::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKind() == TypeKind_DataRef);

	DataPtrType* ptrType = (DataPtrType*)type;
	DataPtrType* intermediateType = ptrType->getTargetType()->getDataPtrType(
		TypeKind_DataPtr,
		ptrType->getPtrTypeKind(),
		ptrType->getFlags() & PtrTypeFlag__All
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
