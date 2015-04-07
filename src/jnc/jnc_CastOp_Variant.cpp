#include "pch.h"
#include "jnc_CastOp_Variant.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool 
Cast_Variant::checkOpType (Type* opType)
{
	if (opType->getSize () > sizeof (DataPtr))
	{
		err::setFormatStringError (
			"'%s' does not fit into 'variant' (current limit is %d bytes)", 
			opType->getTypeString().cc (), 
			sizeof (DataPtr)
			);

		return false;
	}

	return true;
}

bool
Cast_Variant::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Variant);

	Type* opType = opValue.getType ();
	bool result = checkOpType (opType);
	if (!result)
		return false;

	Variant* variant = (Variant*) dst;
	memset (variant, 0, sizeof (Variant));

	const void* src = opValue.getConstData ();
	memcpy (variant, src, opType->getSize ());
	variant->m_type = opType;
	return true;
}

bool
Cast_Variant::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Variant);

	Type* opType = opValue.getType ();
	bool result = checkOpType (opType);
	if (!result)
		return false;
	
	Value opTypeValue (&opType, m_module->m_typeMgr.getStdType (StdType_BytePtr));
	Value variantValue;
	Value castValue;

	m_module->m_llvmIrBuilder.createAlloca (type, "tmpVariant", NULL, &variantValue);
	m_module->m_llvmIrBuilder.createBitCast (variantValue, opType->getDataPtrType_c (), &castValue);
	m_module->m_llvmIrBuilder.createStore (opValue, castValue);
	m_module->m_llvmIrBuilder.createLoad (variantValue, NULL, &variantValue);
	m_module->m_llvmIrBuilder.createInsertValue (variantValue, opTypeValue, 4, type, resultValue);
	return true;
}

//.............................................................................

bool
Cast_FromVariant::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Variant);
	Variant* variant = (Variant*) opValue.getConstData ();
	
	Value tmpValue; 
	if (!variant->m_type)
	{
		memset (dst, 0, type->getSize ());
		return true;
	}

	if (variant->m_type->getSize () > sizeof (DataPtr))
	{
		err::setFormatStringError ("invalid variant type '%s'", variant->m_type->getTypeString ().cc ());
		return false;
	}

	tmpValue.createConst (variant, variant->m_type);

	bool result = m_module->m_operatorMgr.castOperator (&tmpValue, type);
	if (!result)
		return false;

	ASSERT (tmpValue.getValueKind () == ValueKind_Const);
	memcpy (dst, tmpValue.getConstData (), type->getSize ());
	return true;
}

bool
Cast_FromVariant::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Variant);

	Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));
	Value tmpValue;
	
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpValue", type->getDataPtrType_c (), &tmpValue);

	Function* function = m_module->m_functionMgr.getStdFunction (StdFunction_DynamicCastVariant);
	bool result = m_module->m_operatorMgr.callOperator (function, opValue, typeValue, tmpValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createLoad (tmpValue, type, resultValue);
	return true;
}

//.............................................................................

} // namespace jnc {
