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

#else // _JNC_DYNAMIC_EXTENSION_LIB

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

jnc_Variant
jnc_Variant_unaryOperator (
	jnc_UnOpKind opKind,
	const jnc_Variant* variant
	)
{
	using namespace jnc;
	ct::Value opValue;

	if (variant->m_type)
		opValue.createConst (variant, variant->m_type);
	else
		return g_nullVariant;

	ct::Module* module = variant->m_type->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.unaryOperator (
		opKind, 
		opValue, 
		&resultValue
		) && 
		module->m_operatorMgr.castOperator (&resultValue, TypeKind_Variant);
	
	return result ? *(Variant*) resultValue.getConstData () : g_nullVariant;
}

jnc_Variant
jnc_Variant_binaryOperator (
	jnc_BinOpKind opKind,
	const jnc_Variant* variant1,
	const jnc_Variant* variant2
	)
{
	using namespace jnc;
	ct::Value opValue1;

	if (variant1->m_type)
		opValue1.createConst (variant1, variant1->m_type);
	else if (variant2->m_type)
		opValue1.createConst (NULL, variant2->m_type);
	else
		return g_nullVariant;

	ct::Value opValue2;

	if (variant2->m_type)
		opValue2.createConst (&variant2, variant2->m_type);
	else if (variant1->m_type)
		opValue2.createConst (NULL, variant1->m_type);
	else 
		return g_nullVariant;

	ct::Module* module = variant1->m_type->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator (
		opKind, 
		opValue1, 
		opValue2, 
		&resultValue
		) && 
		module->m_operatorMgr.castOperator (&resultValue, TypeKind_Variant);
	
	return result ? *(Variant*) resultValue.getConstData () : g_nullVariant;
}

bool
jnc_Variant_relationalOperator (
	jnc_BinOpKind opKind,
	const jnc_Variant* variant1,
	const jnc_Variant* variant2
	)
{
	using namespace jnc;
	ASSERT (opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge);

	ct::Value opValue1;

	if (variant1->m_type)
		opValue1.createConst (&variant1, variant1->m_type);
	else if (variant2->m_type)
		opValue1.createConst (NULL, variant2->m_type);
	else
		return true;

	ct::Value opValue2;

	if (variant2->m_type)
		opValue2.createConst (&variant2, variant2->m_type);
	else if (variant1->m_type)
		opValue2.createConst (NULL, variant1->m_type);
	else 
		return true;

	ct::Module* module = variant1->m_type->getModule ();

	ct::Value resultValue;
	bool result = module->m_operatorMgr.binaryOperator (
		opKind, 
		opValue1, 
		opValue2, 
		&resultValue
		);

	return result && resultValue.getBool ();
}

size_t 
jnc_Variant_getHash (const jnc_Variant* variant)
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
