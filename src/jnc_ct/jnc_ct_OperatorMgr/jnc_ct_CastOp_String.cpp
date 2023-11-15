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
#include "jnc_ct_CastOp_String.h"
#include "jnc_String.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

JNC_EXTERN_C
size_t
jnc_strLen(jnc_DataPtr ptr);

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_StringBase::getCastKind(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_String);
	ASSERT(
		(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr) ||
		isCharArrayType(opValue.getType()) ||
		isCharArrayRefType(opValue.getType())
	);

	return isDataPtrType(opValue.getType(), DataPtrTypeKind_Thin) ?
		CastKind_None :
		CastKind_Implicit;
}

bool
Cast_StringBase::preparePtr(
	const Value& opValue,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);

	DataPtrType* opType = (DataPtrType*)opValue.getType();
	if (opType->getPtrTypeKind() == DataPtrTypeKind_Normal) {
		*resultValue = opValue;
		return true;
	}

	opType = opType->getTargetType()->getDataPtrType(
		opType->getTypeKind(),
		DataPtrTypeKind_Normal,
		PtrTypeFlag_Const
	);

	return m_module->m_operatorMgr.castOperator(opValue, opType, resultValue);
}

DataPtr
Cast_StringBase::saveLiteral(
	const char* p,
	size_t length
) {
	const Value& value = m_module->m_constMgr.saveLiteral(sl::StringRef(p, length));

	DataPtr ptr;
	ptr.m_p = (void*)value.getConstData();
	ptr.m_validator = m_module->m_constMgr.createConstDataPtrValidator(ptr.m_p, value.getType());
	return ptr;
}

void
Cast_StringBase::finalizeString(
	String* string,
	const char* p,
	size_t length,
	DataPtrValidator* validator
) {
	DataPtr ptr;
	ptr.m_p = (void*)p;
	ptr.m_validator = validator;

	if (p + length < (char*)validator->m_rangeEnd) { // in-range
		if (length && !p[length - 1])
			length--;
		else if (p[length])
			ptr = saveLiteral(p, length);
	} else { // out-of-range
		const char* end = (char*)validator->m_rangeEnd;
		if (p < end && !end[-1])
			length = end - p - 1;
		else
			ptr = saveLiteral(p, length);
	}

	string->m_ptr = ptr;
	string->m_ptr_sz = ptr;
	string->m_length = length;
}

//..............................................................................

bool
Cast_String_FromPtr::constCast(
	const Value& rawOpValue,
	Type* type,
	void* dst
) {
	ASSERT(type->getTypeKind() == TypeKind_String && isCharPtrType(rawOpValue.getType()));

	Value opValue;
	bool result = preparePtr(rawOpValue, &opValue);
	if (!result)
		return false;

	String* string = (String*)dst;
	DataPtr ptr = *(DataPtr*)opValue.getConstData();
	finalizeString(string, (char*)ptr.m_p, jnc_strLen(ptr), ptr.m_validator);
	return true;
}

bool
Cast_String_FromPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKind() == TypeKind_String);

	return m_module->m_operatorMgr.callOperator(
		m_module->m_functionMgr.getStdFunction(StdFunc_StringCreate),
		opValue,
		Value(-1, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
		resultValue
	);
}

//..............................................................................

bool
Cast_String_FromArray::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(type->getTypeKind() == TypeKind_String);

	String* string = (String*)dst;

	if (isArrayRefType(opValue.getType())) {
		Value ptrValue;
		bool result = preparePtr(opValue, &ptrValue);
		if (!result)
			return false;

		DataPtr ptr = *(DataPtr*)ptrValue.getConstData();
		ArrayType* arrayType = (ArrayType*)((DataPtrType*)opValue.getType())->getTargetType();
		finalizeString(string, (char*)ptr.m_p, arrayType->getElementCount(), ptr.m_validator);
		return true;
	}

	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Array);
	ArrayType* srcType = (ArrayType*)opValue.getType();
	size_t length = srcType->getElementCount();
	DataPtr ptr = saveLiteral((char*)opValue.getConstData(), length);
	finalizeString(string, (char*)ptr.m_p, length, ptr.m_validator);
	return true;
}

bool
Cast_String_FromArray::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKind() == TypeKind_String);

	if (!isCharArrayRefType(opValue.getType()))
		return setCastError(opValue, type);

	ArrayType* arrayType = (ArrayType*)((DataPtrType*)opValue.getType())->getTargetType();
	size_t length = arrayType->getElementCount();

	return m_module->m_operatorMgr.callOperator(
		m_module->m_functionMgr.getStdFunction(StdFunc_StringCreate),
		opValue,
		Value(length, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
		resultValue
	);
}

//..............................................................................

CastOperator*
Cast_String::getCastOperator(
	const Value& opValue,
	Type* type
) {
	ASSERT(type->getTypeKind() == TypeKind_String);

	Type* opType = opValue.getType();
	TypeKind typeKind = opType->getTypeKind();
	switch (typeKind) {
	case TypeKind_DataPtr:
		return ((DataPtrType*)opType)->getTargetType()->getTypeKind() == TypeKind_Char ? &m_fromPtr : NULL;

	case TypeKind_DataRef:
		return isCharArrayType(((DataPtrType*)opType)->getTargetType()) ? &m_fromArray : NULL;

	case TypeKind_Array:
		return ((ArrayType*)opType)->getElementType()->getTypeKind() == TypeKind_Char ? &m_fromArray : NULL;

	default:
		return NULL;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
