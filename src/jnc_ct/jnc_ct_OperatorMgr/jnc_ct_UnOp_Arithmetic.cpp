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
#include "jnc_ct_UnOp_Arithmetic.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

static TypeKind g_arithmeticOperatorResultTypeTable[TypeKind_Double - TypeKind_Int8 + 1] = {
	TypeKind_Int32,   // TypeKind_Int8
	TypeKind_Int32,   // TypeKind_Int8_u
	TypeKind_Int32,   // TypeKind_Int16
	TypeKind_Int32,   // TypeKind_Int16_u
	TypeKind_Int32,   // TypeKind_Int32
	TypeKind_Int32_u, // TypeKind_Int32_u
	TypeKind_Int64,   // TypeKind_Int64
	TypeKind_Int64_u, // TypeKind_Int64_u
	TypeKind_Int32,   // TypeKind_Int16_be
	TypeKind_Int32,   // TypeKind_Int16_ube
	TypeKind_Int32,   // TypeKind_Int32_be
	TypeKind_Int32_u, // TypeKind_Int32_ube
	TypeKind_Int64,   // TypeKind_Int64_be
	TypeKind_Int64_u, // TypeKind_Int64_ube
	TypeKind_Float,   // TypeKind_Float
	TypeKind_Double,  // TypeKind_Double
};

Type*
getArithmeticOperatorResultType(Type* opType) {
	TypeKind typeKind = opType->getTypeKind();
	if (typeKind == TypeKind_Enum)
		return getArithmeticOperatorResultType(((EnumType*)opType)->getBaseType());

	size_t i = typeKind - TypeKind_Int8;
	return i < countof(g_arithmeticOperatorResultTypeTable) ?
		opType->getModule()->m_typeMgr.getPrimitiveType(g_arithmeticOperatorResultTypeTable[i]) :
		NULL;
}

//..............................................................................

llvm::Value*
UnOp_Minus::llvmOpInt(
	const Value& opValue,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createNeg_i(opValue, resultType, resultValue);
}

llvm::Value*
UnOp_Minus::llvmOpFp(
	const Value& opValue,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createNeg_f(opValue, resultType, resultValue);
}

//..............................................................................

llvm::Value*
UnOp_BwNot::llvmOpInt(
	const Value& opValue,
	Type* resultType,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createNot(opValue, resultType, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
