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
#include "jnc_ct_UnOp_LogNot.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
UnOp_LogNot::op(
	const Value& opValue,
	Value* resultValue
	)
{
	TypeKind srcTypeKind = opValue.getType()->getTypeKind();
	switch (srcTypeKind)
	{
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
	case TypeKind_Int16_beu:
	case TypeKind_Int32_be:
	case TypeKind_Int32_beu:
	case TypeKind_Int64_be:
	case TypeKind_Int64_beu:
	case TypeKind_Float:
	case TypeKind_Double:
	case TypeKind_BitField:
	case TypeKind_Enum:
		return zeroCmpOperator(opValue, resultValue);

	case TypeKind_DataPtr:
	case TypeKind_ClassPtr:
	case TypeKind_FunctionPtr:
	case TypeKind_PropertyPtr:
		return ptrOperator(opValue, resultValue);

	case TypeKind_Variant:
		return variantOperator(opValue, resultValue);

	default:
		setOperatorError(opValue);
		return false;
	}
}

bool
UnOp_LogNot::zeroCmpOperator(
	const Value& opValue,
	Value* resultValue
	)
{
	Value zeroValue = opValue.getType()->getZeroValue();
	return m_module->m_operatorMgr.binaryOperator(BinOpKind_Eq, opValue, zeroValue, resultValue);
}

bool
UnOp_LogNot::ptrOperator(
	const Value& opValue,
	Value* resultValue
	)
{
	if (opValue.getType()->getSize() == sizeof(void*))
		return zeroCmpOperator(opValue, resultValue);

	if (!m_module->hasCodeGen())
	{
		resultValue->setType(m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool));
		return true;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, m_module->m_typeMgr.getStdType(StdType_BytePtr), &ptrValue);
	return zeroCmpOperator(ptrValue, resultValue);
}

bool
UnOp_LogNot::variantOperator(
	const Value& opValue,
	Value* resultValue
	)
{
	Value boolValue;

	return
		m_module->m_operatorMgr.castOperator(opValue, TypeKind_Bool, &boolValue) &&
		zeroCmpOperator(boolValue, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
