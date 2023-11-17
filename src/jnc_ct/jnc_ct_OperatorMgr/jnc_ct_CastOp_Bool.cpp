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
Cast_BoolFromZeroCmp::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	const char* p = (const char*) opValue.getConstData();
	const char* end = p + opValue.getType()->getSize();

	bool result = false;

	for (; p < end; p++) {
		if (*p) {
			result = true;
			break;
		}
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
	Value zeroValue = opValue.getType()->getZeroValue();
	return m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, opValue, zeroValue, resultValue);
}

//..............................................................................

bool
Cast_BoolFromString::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
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
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Bool);

	memset(dst, 0, type->getSize());

	if (*(bool*)opValue.getConstData())
		*(char*)dst = 1;

	return true;
}

bool
Cast_IntFromBool::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Bool);
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

	case TypeKind_Bool:
	case TypeKind_Int8:
	case TypeKind_Int8_u:
	case TypeKind_Int16:
	case TypeKind_Int16_u:
	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int64:
	case TypeKind_Int64_u:
	case TypeKind_Int16_be:
	case TypeKind_Int16_ube:
	case TypeKind_Int32_be:
	case TypeKind_Int32_ube:
	case TypeKind_Int64_be:
	case TypeKind_Int64_ube:
	case TypeKind_Float:
	case TypeKind_Double:
	case TypeKind_BitField:
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
