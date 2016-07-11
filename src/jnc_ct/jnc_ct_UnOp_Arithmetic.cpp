#include "pch.h"
#include "jnc_ct_UnOp_Arithmetic.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Type*
getArithmeticOperatorResultType (Type* opType)
{
	TypeKind typeKind = opType->getTypeKind ();

	switch (typeKind)
	{
	case TypeKind_Int8:
	case TypeKind_Int16:
	case TypeKind_Int16_be:
	case TypeKind_Int32_be:
		typeKind = TypeKind_Int32;
		break;

	case TypeKind_Int8_u:
	case TypeKind_Int16_u:
	case TypeKind_Int16_beu:
	case TypeKind_Int32_beu:
		typeKind = TypeKind_Int32_u;
		break;
	
	case TypeKind_Int64_be:
		typeKind = TypeKind_Int64;
		break;

	case TypeKind_Int64_beu:
		typeKind = TypeKind_Int64_u;
		break;

	case TypeKind_Int32:
	case TypeKind_Int32_u:
	case TypeKind_Int64:
	case TypeKind_Int64_u:
	case TypeKind_Float:
	case TypeKind_Double:
		// no change
		break;

	case TypeKind_Enum:
		return getArithmeticOperatorResultType (((EnumType*) opType)->getBaseType ());

	default:
		return NULL;
	}

	return opType->getModule ()->m_typeMgr.getPrimitiveType (typeKind);
}

//.............................................................................

llvm::Value*
UnOp_Minus::llvmOpInt (
	const Value& opValue,
	Type* resultType,
	Value* resultValue
	)
{
	return m_module->m_llvmIrBuilder.createNeg_i (opValue, resultType, resultValue);
}

llvm::Value*
UnOp_Minus::llvmOpFp (
	const Value& opValue,
	Type* resultType,
	Value* resultValue
	)
{
	return m_module->m_llvmIrBuilder.createNeg_f (opValue, resultType, resultValue);
}

//.............................................................................

llvm::Value*
UnOp_BwNot::llvmOpInt (
	const Value& opValue,
	Type* resultType,
	Value* resultValue
	)
{
	return m_module->m_llvmIrBuilder.createNot (opValue, resultType, resultValue);
}

//.............................................................................

} // namespace ct
} // namespace jnc
