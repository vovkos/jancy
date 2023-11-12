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
#include "jnc_ct_CastOp_Fp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
Cast_FpTrunc::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	m_module->m_llvmIrBuilder.createTrunc_f(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_FpExt::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	m_module->m_llvmIrBuilder.createExt_f(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_FpFromInt::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	m_module->m_llvmIrBuilder.createIntToFp(opValue, type, resultValue);
	return true;
}

bool
Cast_FpFromInt::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	TypeKind dstTypeKind = type->getTypeKind();
	switch (dstTypeKind) {
	case TypeKind_Float:
		constCast_Fp32(opValue, (float*)dst);
		break;

	case TypeKind_Double:
		constCast_Fp64(opValue, (double*)dst);
		break;

	default:
		ASSERT(false);
	}

	return true;
}

void
Cast_FpFromInt::constCast_Fp32(
	const Value& opValue,
	float* fp32
) {
	const void* src = opValue.getConstData();

	size_t srcSize = opValue.getType()->getSize();
	switch (srcSize) {
	case 1:
		*fp32 = *(char*)src;
		break;

	case 2:
		*fp32 = *(short*)src;
		break;

	case 4:
		*fp32 = (float)*(int32_t*)src;
		break;

	case 8:
		*fp32 = (float)*(int64_t*)src;
		break;

	default:
		ASSERT(false);
	}
};

void
Cast_FpFromInt::constCast_Fp64(
	const Value& opValue,
	double* fp64
) {
	const void* src = opValue.getConstData();

	size_t srcSize = opValue.getType()->getSize();
	switch (srcSize) {
	case 1:
		*fp64 = *(char*)src;
		break;

	case 2:
		*fp64 = *(short*)src;
		break;

	case 4:
		*fp64 = *(int32_t*)src;
		break;

	case 8:
		*fp64 = (double)*(int64_t*)src;
		break;

	default:
		ASSERT(false);
	}
};

//..............................................................................

bool
Cast_FpFromInt_u::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
) {
	m_module->m_llvmIrBuilder.createIntToFp_u(opValue, type, resultValue);
	return true;
}

bool
Cast_FpFromInt_u::constCast(
	const Value& opValue,
	Type* type,
	void* dst
) {
	TypeKind dstTypeKind = type->getTypeKind();
	switch (dstTypeKind) {
	case TypeKind_Float:
		constCast_Fp32(opValue, (float*)dst);
		break;

	case TypeKind_Double:
		constCast_Fp64(opValue, (double*)dst);
		break;

	default:
		ASSERT(false);
	}

	return true;
}

void
Cast_FpFromInt_u::constCast_Fp32(
	const Value& opValue,
	float* fp32
) {
	const void* src = opValue.getConstData();

	size_t srcSize = opValue.getType()->getSize();
	switch (srcSize) {
	case 1:
		*fp32 = *(uint8_t*)src;
		break;

	case 2:
		*fp32 = *(uint16_t*)src;
		break;

	case 4:
		*fp32 = (float)*(uint32_t*)src;
		break;

	case 8:
		*fp32 = (float)*(uint64_t*)src;
		break;

	default:
		ASSERT(false);
	}
};

void
Cast_FpFromInt_u::constCast_Fp64(
	const Value& opValue,
	double* fp64
) {
	const void* src = opValue.getConstData();

	size_t srcSize = opValue.getType()->getSize();
	switch (srcSize) {
	case 1:
		*fp64 = *(uint8_t*)src;
		break;

	case 2:
		*fp64 = *(uint16_t*)src;
		break;

	case 4:
		*fp64 = *(uint32_t*)src;
		break;

	case 8:
		*fp64 = (double)*(uint64_t*)src;
		break;

	default:
		ASSERT(false);
	}
};

//..............................................................................

bool
Cast_FpFromBeInt::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_BigEndian);

	TypeKind intermediateTypeKind = getLittleEndianIntegerTypeKind(opValue.getType()->getTypeKind());

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_SwapByteOrder);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Fp);
	*intermediateType = m_module->m_typeMgr.getPrimitiveType(intermediateTypeKind);
	return true;
}

//..............................................................................

bool
Cast_FpFromEnum::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType_o
) {
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Enum);

	Type* intermediateType = ((EnumType*)opValue.getType())->getBaseType();

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Fp);
	*intermediateType_o = intermediateType;
	return true;
}

//..............................................................................

CastOperator*
Cast_Fp::getCastOperator(
	const Value& opValue,
	Type* type
) {
	Type* srcType = opValue.getType();

	TypeKind srcTypeKind = srcType->getTypeKind();
	TypeKind dstTypeKind = type->getTypeKind();

	size_t srcSize = srcType->getSize();
	size_t dstSize = type->getSize();

	ASSERT(dstTypeKind == TypeKind_Float || dstTypeKind == TypeKind_Double);

	switch (srcTypeKind) {
	case TypeKind_Int8:
	case TypeKind_Int16:
	case TypeKind_Int32:
	case TypeKind_Int64:
		return &m_fromInt;

	case TypeKind_Bool:
	case TypeKind_Int8_u:
	case TypeKind_Int16_u:
	case TypeKind_Int32_u:
	case TypeKind_Int64_u:
		return &m_fromInt_u;

	case TypeKind_Int16_be:
	case TypeKind_Int16_ube:
	case TypeKind_Int32_be:
	case TypeKind_Int32_ube:
	case TypeKind_Int64_be:
	case TypeKind_Int64_ube:
		return &m_fromBeInt;

	case TypeKind_Float:
	case TypeKind_Double:
		return
			srcSize == dstSize ? m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy) :
			srcSize > dstSize ? (CastOperator*) &m_trunc :
			(CastOperator*) &m_ext;

	case TypeKind_Enum:
		return &m_fromEnum;

	default:
		return NULL;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
