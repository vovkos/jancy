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
#include "jnc_Variant.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_cast (
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_castFunc (variant, type, buffer);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_unaryOperator (
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_unaryOperatorFunc (variant, opKind, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
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
JNC_EXPORT_O
bool_t
jnc_Variant_relationalOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_relationalOperatorFunc (variant, variant2, opKind, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getMember (
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_getMemberFunc (variant, name, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setMember (
	jnc_Variant* variant,
	const char* name,
	jnc_Variant value
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_setMemberFunc (variant, name, value);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getElement (
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* result
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_getElementFunc (variant, index, result);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setElement (
	jnc_Variant* variant,
	size_t index,
	jnc_Variant value
	)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_setElementFunc (variant, index, value);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Variant_hash (const jnc_Variant* variant)
{
	return jnc_g_dynamicExtensionLibHost->m_variantFuncTable->m_hashFunc (variant);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
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
JNC_EXPORT_O
bool_t
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
JNC_EXPORT_O
bool_t
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
		return false;

	*resultVariant = *(Variant*) resultValue.getConstData ();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_relationalOperator (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* resultBool
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
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getMember (
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError ("cannot apply member operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule ();

	ct::Value opValue (variant, variant->m_type);
	ct::Value memberValue;
	bool result =
		module->m_operatorMgr.memberOperator (opValue, name, &memberValue) &&
		module->m_operatorMgr.castOperator (&memberValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*) memberValue.getConstData ();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setMember (
	jnc_Variant* variant,
	const char* name,
	jnc_Variant valueVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError ("cannot apply member operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule ();

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags () & TypeKindFlag_Ptr)
	{
		opValue.createConst (variant, variant->m_type);
	}
	else
	{
		ASSERT (variant->m_type->getSize () <= sizeof (DataPtr));
		opValue.createConst (&variant, variant->m_type->getDataPtrType_c (TypeKind_DataRef));
	}

	ct::Value opValue2 (&valueVariant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	ct::Value memberValue;

	return
		module->m_operatorMgr.memberOperator (opValue, name, &memberValue) &&
		module->m_operatorMgr.binaryOperator (BinOpKind_Assign, memberValue, opValue2);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_getElement (
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* resultVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError ("cannot apply index operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule ();

	// turning it into ref is only necessary because of current implementation of OperatorMgr::memberOperator (size_t)

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags () & TypeKindFlag_Ptr)
	{
		opValue.createConst (variant, variant->m_type);
	}
	else
	{
		ASSERT (variant->m_type->getSize () <= sizeof (DataPtr));
		opValue.createConst (&variant, variant->m_type->getDataPtrType_c (TypeKind_DataRef));
	}

	ct::Value memberValue;
	bool result =
		module->m_operatorMgr.memberOperator (opValue, index, &memberValue) &&
		module->m_operatorMgr.castOperator (&memberValue, TypeKind_Variant);

	if (!result)
		return false;

	*resultVariant = *(Variant*) memberValue.getConstData ();
	return true;
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Variant_setElement (
	jnc_Variant* variant,
	size_t index,
	jnc_Variant valueVariant
	)
{
	using namespace jnc;

	if (!variant->m_type)
	{
		err::setError ("cannot apply index operator to 'null'");
		return true;
	}

	ct::Module* module = variant->m_type->getModule ();

	ct::Value opValue;
	if (variant->m_type->getTypeKindFlags () & TypeKindFlag_Ptr)
	{
		opValue.createConst (variant, variant->m_type);
	}
	else
	{
		ASSERT (variant->m_type->getSize () <= sizeof (DataPtr));
		opValue.createConst (&variant, variant->m_type->getDataPtrType_c (TypeKind_DataRef));
	}

	ct::Value opValue2 (&valueVariant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	ct::Value memberValue;

	return
		module->m_operatorMgr.memberOperator (opValue, index, &memberValue) &&
		module->m_operatorMgr.binaryOperator (BinOpKind_Assign, memberValue, opValue2);
}

JNC_EXTERN_C
JNC_EXPORT_O
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

//..............................................................................

bool
jnc::Variant::relationalOperator (
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool* result
	) const
{
	bool_t intResult;
	if (!jnc_Variant_relationalOperator (this, variant2, opKind, &intResult))
		return false;

	*result = intResult != 0;
	return true;
}

//..............................................................................
