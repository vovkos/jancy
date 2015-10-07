#include "pch.h"
#include "jnc_ct_CastOp_Bool.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

bool
Cast_BoolFromZeroCmp::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	const char* p = (const char*) opValue.getConstData ();
	const char* end = p + opValue.getType ()->getSize ();
	
	bool result = false;

	for (; p < end; p++)
	{
		if (*p)
		{
			result = true;
			break;
		}
	}

	*(bool*) dst = result;
	return true;
}

bool
Cast_BoolFromZeroCmp::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	Value zeroValue = opValue.getType ()->getZeroValue ();
	return m_module->m_operatorMgr.binaryOperator (BinOpKind_Ne, opValue, zeroValue, resultValue);
}

//.............................................................................

bool
Cast_BoolFromPtr::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	if (opValue.getType ()->getSize () == sizeof (void*))
		return Cast_BoolFromZeroCmp::llvmCast (opValue, type, resultValue);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
	return Cast_BoolFromZeroCmp::llvmCast (ptrValue, type, resultValue);
}


//.............................................................................

bool
Cast_IntFromBool::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Bool);

	memset (dst, 0, type->getSize ());

	if (*(bool*) opValue.getConstData ())
		*(char*) dst = 1;

	return true;
}

bool
Cast_IntFromBool::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Bool);
	m_module->m_llvmIrBuilder.createExt_u (opValue, type, resultValue);
	return true;
}

//.............................................................................

CastOperator*
Cast_Bool::getCastOperator (
	const Value& opValue,
	Type* type
	)
{
	TypeKind srcTypeKind = opValue.getType ()->getTypeKind ();
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
		return &m_fromZeroCmp;

	case TypeKind_DataPtr:
	case TypeKind_ClassPtr:
	case TypeKind_FunctionPtr:
	case TypeKind_PropertyPtr:
		return &m_fromPtr;

	default:
		return NULL;
	}
}

//.............................................................................

} // namespace ct
} // namespace jnc
