#include "pch.h"
#include "jnc_Variant.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
int
jnc_Variant_cast (
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_castFunc (variant, type, buffer);
}

JNC_EXTERN_C
int
jnc_Variant_unaryOperator (
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_unaryOperatorFunc (variant, opKind, result);
}

JNC_EXTERN_C
int
jnc_Variant_binaryOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_binaryOperatorFunc (variant, variant2, opKind, result);
}

JNC_EXTERN_C
int
jnc_Variant_relationalOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	int* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_relationalOperatorFunc (variant, variant2, opKind, result);
}

JNC_EXTERN_C
size_t 
jnc_Variant_hash (const jnc_Variant* variant)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_hashFunc (variant);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
int
jnc_Variant_cast (
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	using namespace jnc;
	ct::Module* module = type->getModule ();

	ct::Value opValue (variant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	ct::CastOperator* castOp = module->m_operatorMgr.getStdCastOperator (ct::StdCast_FromVariant);

	memset (buffer, 0, type->getSize ());
	return castOp->constCast (opValue, type, buffer);
}

JNC_EXTERN_C
int
jnc_Variant_unaryOperator (
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;
	ct::Value opValue;

	if (variant->m_type)
	{
		opValue.createConst (variant, variant->m_type);
	}
	else
	{
		*resultVariant = *variant;
		return true;
	}

	ct::Module* module = variant->m_type->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.unaryOperator (
		opKind, 
		opValue, 
		&resultValue
		) && 
		module->m_operatorMgr.castOperator (&resultValue, TypeKind_Variant);
	
	if (!result)
		return false;

	*resultVariant = *(Variant*) resultValue.getConstData (); 
	return true;
}

JNC_EXTERN_C
int
jnc_Variant_binaryOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;
	ct::Value opValue1;

	if (variant->m_type)
	{
		opValue1.createConst (variant, variant->m_type);
	}
	else if (variant2->m_type)
	{
		opValue1.createConst (NULL, variant2->m_type);
	}
	else
	{
		*resultVariant = *variant;
		return true;
	}

	ct::Value opValue2;

	if (variant2->m_type)
	{
		opValue2.createConst (variant2, variant2->m_type);
	}
	else
	{
		ASSERT (variant->m_type);
		opValue2.createConst (NULL, variant->m_type);
	}

	ct::Module* module = opValue1.getType ()->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator (
		opKind, 
		opValue1, 
		opValue2, 
		&resultValue
		) && 
		module->m_operatorMgr.castOperator (&resultValue, TypeKind_Variant);
	
	if (!result)
		return 0;

	*resultVariant = *(Variant*) resultValue.getConstData (); 
	return 1;
}

JNC_EXTERN_C
int
jnc_Variant_relationalOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	int* resultBool
	)
{
	using namespace jnc;
	ASSERT (opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge);

	ct::Value opValue1;

	if (variant->m_type)
	{
		opValue1.createConst (variant, variant->m_type);
	}
	else if (variant2->m_type)
	{
		opValue1.createConst (NULL, variant2->m_type);
	}
	else
	{
		*resultBool = opKind == jnc_BinOpKind_Eq;
		return true;
	}

	ct::Value opValue2;

	if (variant2->m_type)
	{
		opValue2.createConst (variant2, variant2->m_type);
	}
	else 
	{
		ASSERT (variant->m_type);
		opValue2.createConst (NULL, variant->m_type);
	}

	ct::Module* module = opValue1.getType ()->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator (
		opKind, 
		opValue1, 
		opValue2, 
		&resultValue
		) && 
		module->m_operatorMgr.castOperator (&resultValue, TypeKind_Bool);

	if (!result)
		return false;

	*resultBool = *(bool*) resultValue.getConstData (); 
	return 1;
}

JNC_EXTERN_C
size_t 
jnc_Variant_hash (const jnc_Variant* variant)
{
	using namespace jnc;
	if (!variant->m_type)
		return 0;

	size_t size = variant->m_type->getSize ();

	if (variant->m_type->getTypeKindFlags () & TypeKindFlag_BigEndian)
	{
		uint64_t result = 0;
		axl::sl::swapByteOrder (&result, &variant->m_int64, size);
		return (uintptr_t) result;		
	}

	if (size <= sizeof (uintptr_t) || variant->m_type->getTypeKind () == TypeKind_DataPtr)
		return variant->m_uintptr;

	const void* p = size <= sizeof (DataPtr) ? &variant : variant->m_p;
	return sl::djb2 (p, size);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................

bool
jnc::Variant::relationalOperator (
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool* result
	) const
{
	int intResult;
	if (!jnc_Variant_relationalOperator (this, variant2, opKind, &intResult))
		return false;

	*result = intResult != 0;
	return true;
}

//.............................................................................
