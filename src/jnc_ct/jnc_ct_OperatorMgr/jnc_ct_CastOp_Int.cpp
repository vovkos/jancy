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
#include "jnc_ct_CastOp_Int.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
Cast_IntTrunc::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	size_t srcSize = opValue.getType()->getSize();
	size_t dstSize = type->getSize();

	ASSERT(srcSize > dstSize);

	memcpy(dst, opValue.getConstData(), dstSize);
	return true;
}

bool
Cast_IntTrunc::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createTrunc_i(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_IntExt::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	size_t srcSize = opValue.getType()->getSize();
	size_t dstSize = type->getSize();

	ASSERT(srcSize < dstSize);

	char* src = (char*)opValue.getConstData();

	if (src[srcSize - 1] & 0x80)
		memset(dst, -1, dstSize);
	else
		memset(dst, 0, dstSize);

	memcpy(dst, src, srcSize);
	return true;
}

bool
Cast_IntExt::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createExt_i(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_IntExt_u::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	size_t srcSize = opValue.getType()->getSize();
	size_t dstSize = type->getSize();

	ASSERT(srcSize < dstSize);

	char* src = (char*)opValue.getConstData();

	memset(dst, 0, dstSize);
	memcpy(dst, src, srcSize);
	return true;
}

bool
Cast_IntExt_u::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createExt_u(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_SwapByteOrder::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	size_t srcSize = opValue.getType()->getSize();
	size_t dstSize = type->getSize();

	ASSERT(srcSize == dstSize);

	char* src = (char*)opValue.getConstData();

	sl::swapByteOrder(dst, src, srcSize);
	return true;
}

bool
Cast_SwapByteOrder::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	llvm::Type* llvmType = type->getLlvmType();

	llvm::Function* llvmSwap = llvm::Intrinsic::getDeclaration(
		m_module->getLlvmModule(),
		llvm::Intrinsic::bswap,
		llvm::ArrayRef<llvm::Type*> (llvmType)
		);

	Value swapFunctionValue;
	swapFunctionValue.setLlvmValue(llvmSwap, NULL);
	m_module->m_llvmIrBuilder.createCall(
		swapFunctionValue,
		m_module->m_typeMgr.getCallConv(CallConvKind_Default),
		&opValue, 1,
		type,
		resultValue
		);

	return true;
}

//..............................................................................

bool
Cast_IntFromBeInt::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType
	)
{
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_BigEndian);

	TypeKind intermediateTypeKind = getLittleEndianIntegerTypeKind(opValue.getType()->getTypeKind());

	if (isEquivalentIntegerTypeKind(type->getTypeKind(), intermediateTypeKind))
	{
		*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_SwapByteOrder);
		return true;
	}

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_SwapByteOrder);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Int);
	*intermediateType = m_module->m_typeMgr.getPrimitiveType(intermediateTypeKind);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_BeInt::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType
	)
{
	ASSERT(type->getTypeKindFlags() & TypeKindFlag_BigEndian);

	TypeKind intermediateTypeKind = getLittleEndianIntegerTypeKind(type->getTypeKind());

	if (isEquivalentIntegerTypeKind(opValue.getType()->getTypeKind(), intermediateTypeKind))
	{
		*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_SwapByteOrder);
		return true;
	}

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Int);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_SwapByteOrder);
	*intermediateType = m_module->m_typeMgr.getPrimitiveType(intermediateTypeKind);
	return true;
}

//..............................................................................

bool
Cast_IntFromFp::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createFpToInt(opValue, type, resultValue);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_IntFromFp32::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Float);

	float fp32 = *(float*)opValue.getConstData();

	size_t dstSize = type->getSize();
	switch (dstSize)
	{
	case 1:
		*(int8_t*)dst = (int8_t)fp32;
		break;

	case 2:
		*(int16_t*)dst = (int16_t)fp32;
		break;

	case 4:
		*(int32_t*)dst = (int32_t)fp32;
		break;

	case 8:
		*(int64_t*)dst = (int64_t)fp32;
		break;

	default:
		ASSERT(false);
	}

	return true;
};

//..............................................................................

bool
Cast_IntFromFp64::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Double);

	double fp64 = *(double*)opValue.getConstData();

	size_t dstSize = type->getSize();
	switch (dstSize)
	{
	case 1:
		*(int8_t*)dst = (int8_t)fp64;
		break;

	case 2:
		*(int16_t*)dst = (int16_t)fp64;
		break;

	case 4:
		*(int32_t*)dst = (int32_t)fp64;
		break;

	case 8:
		*(int64_t*)dst = (int64_t)fp64;
		break;

	default:
		ASSERT(false);
	}

	return true;
};

//..............................................................................

bool
Cast_IntFromPtr::constCast(
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT(opValue.getType()->getSize() >= sizeof(intptr_t));

	size_t size = type->getSize();
	if (size > sizeof(intptr_t))
		size = sizeof(intptr_t);

	memcpy(dst, opValue.getConstData(), size);
	return true;
}

bool
Cast_IntFromPtr::llvmCast(
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	Value ptrValue;

	if (opValue.getType()->getSize() > sizeof(intptr_t))
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &ptrValue);
	else
		ptrValue = opValue;

	m_module->m_llvmIrBuilder.createPtrToInt(ptrValue, m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr), &ptrValue);
	return m_module->m_operatorMgr.castOperator(ptrValue, type, resultValue);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_PtrFromInt::constCast(
	const Value& rawOpValue,
	Type* type,
	void* dst
	)
{
	Value opValue;
	bool result = m_module->m_operatorMgr.castOperator(rawOpValue, TypeKind_IntPtr, &opValue);
	if (!result)
		return false;

	ASSERT(opValue.getValueKind() == ValueKind_Const);
	ASSERT(opValue.getType()->getSize() == sizeof(intptr_t));
	ASSERT(type->getSize() == sizeof(intptr_t));

	*(intptr_t*)dst = *(intptr_t*)opValue.getConstData();
	return true;
}

bool
Cast_PtrFromInt::llvmCast(
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	Value opValue;
	bool result = m_module->m_operatorMgr.castOperator(rawOpValue, TypeKind_IntPtr, &opValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createIntToPtr(opValue, type, resultValue);
	return true;
}

//..............................................................................

bool
Cast_IntFromEnum::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType_o
	)
{
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_Enum);

	Type* intermediateType = ((EnumType*)opValue.getType())->getRootType();

	if (isEquivalentIntegerTypeKind(type->getTypeKind(), intermediateType->getTypeKind()))
	{
		*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy);
		return true;
	}

	bool isBigEndian = (intermediateType->getTypeKindFlags() & TypeKindFlag_BigEndian) != 0;

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(isBigEndian ? StdCast_BeInt : StdCast_Int);
	*intermediateType_o = intermediateType;
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

CastKind
Cast_Enum::getCastKind(
	const Value& opValue,
	Type* type
	)
{
	ASSERT(type->getTypeKind() == TypeKind_Enum);
	ASSERT(type->cmp(opValue.getType()) != 0); // identity should have been handled earlier

	Type* opType = opValue.getType();

	// 0 could be put to bitflag enum

	return
		opType->getTypeKind() == TypeKind_Enum && ((EnumType*)type)->isBaseType((EnumType*)opType) ||
		(type->getFlags() & EnumTypeFlag_BitFlag) && opValue.isZero() ?
			CastKind_Implicit :
			CastKind_Explicit;
}

bool
Cast_Enum::getCastOperators(
	const Value& opValue,
	Type* type,
	CastOperator** firstOperator,
	CastOperator** secondOperator,
	Type** intermediateType_o
	)
{
	ASSERT(type->getTypeKind() == TypeKind_Enum);

	Type* intermediateType = ((EnumType*)type)->getRootType();

	if (isEquivalentIntegerTypeKind(opValue.getType()->getTypeKind(), intermediateType->getTypeKind()))
	{
		*firstOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy);
		return true;
	}

	bool isBigEndian = (intermediateType->getTypeKindFlags() & TypeKindFlag_BigEndian) != 0;

	*firstOperator = m_module->m_operatorMgr.getStdCastOperator(isBigEndian ? StdCast_BeInt : StdCast_Int);
	*secondOperator = m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy);
	*intermediateType_o = intermediateType;
	return true;
}

//..............................................................................

CastOperator*
Cast_Int::getCastOperator(
	const Value& opValue,
	Type* type
	)
{
	Type* srcType = opValue.getType();

	TypeKind srcTypeKind = srcType->getTypeKind();
	TypeKind dstTypeKind = type->getTypeKind();

	size_t srcSize = srcType->getSize();
	size_t dstSize = type->getSize();

	ASSERT(dstTypeKind >= TypeKind_Int8 && dstTypeKind <= TypeKind_Int64_u);

	switch (srcTypeKind)
	{
	case TypeKind_Bool:
		return &m_ext_u; // 1 bit -- could only be extended

	case TypeKind_Int8:
	case TypeKind_Int8_u:
	case TypeKind_Int16:
	case TypeKind_Int16_u:
	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int64:
	case TypeKind_Int64_u:
		return
			srcSize == dstSize ? m_module->m_operatorMgr.getStdCastOperator(StdCast_Copy) :
			srcSize > dstSize ? (CastOperator*)&m_trunc :
			(getTypeKindFlags(srcTypeKind) & TypeKindFlag_Unsigned) ?
				(CastOperator*)&m_ext_u :
				(CastOperator*)&m_ext;

	case TypeKind_Int16_be:
	case TypeKind_Int16_beu:
	case TypeKind_Int32_be:
	case TypeKind_Int32_beu:
	case TypeKind_Int64_be:
	case TypeKind_Int64_beu:
		return &m_fromBeInt;

	case TypeKind_Float:
		return &m_fromFp32;

	case TypeKind_Double:
		return &m_fromFp64;

	case TypeKind_Enum:
		return &m_fromEnum;

	case TypeKind_DataPtr:
	case TypeKind_ClassPtr:
	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
	case TypeKind_PropertyPtr:
		return &m_fromPtr;

	default:
		return NULL;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
