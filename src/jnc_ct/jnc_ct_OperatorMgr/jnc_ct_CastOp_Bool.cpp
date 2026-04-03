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
#include "jnc_ct_CastOp_Bool.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
Cast_BoolFromBool::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKindFlags() & TypeKindFlag_Bool);

	if (type->getTypeKind() == TypeKind_Bool8) {
		ASSERT(opValue.getType()->getTypeKind() == TypeKind_Bool1);
		m_module->m_llvmIrBuilder.createExt_u(opValue, type, resultValue);
	} else {
		ASSERT(opValue.getType()->getTypeKind() == TypeKind_Bool8);
		m_module->m_llvmIrBuilder.createTrunc_i(opValue, type, resultValue);
	}

	return true;
}

//..............................................................................

bool
Cast_BoolFromZeroCmp::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(type->getTypeKindFlags() & TypeKindFlag_Bool);

	const char* p = (const char*)opValue.getConstData();
	size_t size = opValue.getType()->getSize();
	bool result;
	switch (size) {
	case 8:
		result = *(uint64_t*)p != 0;
		break;

	case 4:
		result = *(uint32_t*)p != 0;
		break;

	case 2:
		result = *(uint16_t*)p != 0;
		break;

	default:
		result = *(uint8_t*)p != 0;
	}

	*(bool*)dst = result;
	return true;
}

bool
Cast_BoolFromZeroCmp::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKindFlags() & TypeKindFlag_Bool);

	Value zeroValue = opValue.getType()->getZeroValue();
	bool result = m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, opValue, zeroValue, resultValue);
	if (!result)
		return false;

	ASSERT(resultValue->getType()->getTypeKind() == TypeKind_Bool1);
	if (type->getTypeKind() == TypeKind_Bool8)
		m_module->m_llvmIrBuilder.createExt_u(*resultValue, type, resultValue);

	return true;
}

//..............................................................................

bool
Cast_BoolFromString::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(type->getTypeKindFlags() & TypeKindFlag_Bool);

	StructType* stringType = (StructType*)m_module->m_typeMgr.getStdType(StdType_StringStruct);
	Field* lengthField = stringType->getFieldArray()[2];
	Value lengthValue;

	return
		m_module->m_operatorMgr.getField(opValue, stringType, lengthField, &lengthValue) &&
		m_module->m_operatorMgr.castOperator(lengthValue, type, resultValue);
}

//..............................................................................

bool
Cast_BoolFromPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	if (opValue.getType()->getSize() == sizeof(void*))
		return Cast_BoolFromZeroCmp::llvmCast(opValue, type, resultValue);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &ptrValue);
	return Cast_BoolFromZeroCmp::llvmCast(ptrValue, type, resultValue);
}


//..............................................................................

bool
Cast_IntFromBool::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_Bool);
	memset(dst, 0, type->getSize());
	*(char*)dst = *(char*)opValue.getConstData();
	return true;
}

bool
Cast_IntFromBool::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_Bool);
	m_module->m_llvmIrBuilder.createExt_u(opValue, type, resultValue);
	return true;
}

//..............................................................................

CastOperator*
Cast_Bool::getCastOperator(
	const Value& opValue,
	Type* type
) {
	TypeKind srcTypeKind = opValue.getType()->getTypeKind();
	switch (srcTypeKind) {
	case TypeKind_String:
		return &m_fromString;

	case TypeKind_Bool1:
	case TypeKind_Bool8:
		return &m_fromBool;

	case TypeKind_Int8:
	case TypeKind_Int8_u:
	case TypeKind_Int16:
	case TypeKind_Int16_u:
	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int64:
	case TypeKind_Int64_u:
	case TypeKind_Float:
	case TypeKind_Double:
	case TypeKind_Enum:
		return &m_fromZeroCmp;

	case TypeKind_DataPtr:
	case TypeKind_ClassPtr:
	case TypeKind_FunctionPtr:
	case TypeKind_PropertyPtr:
		return &m_fromPtr;

	case TypeKind_Array:
		return &m_true;

	default:
		return NULL;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
