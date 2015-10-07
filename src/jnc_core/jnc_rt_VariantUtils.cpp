#include "pch.h"
#include "jnc_rt_VariantUtils.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace rt {

//.............................................................................

bool
variantRelationalOperator (
	ct::BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	)
{
	ASSERT (opKind >= ct::BinOpKind_Eq && opKind <= ct::BinOpKind_Ge);

	ct::Value opValue1;

	if (op1.m_type)
		opValue1.createConst (&op1, op1.m_type);
	else if (op2.m_type)
		opValue1.createConst (NULL, op2.m_type);
	else
		return true;

	ct::Value opValue2;

	if (op2.m_type)
		opValue2.createConst (&op2, op2.m_type);
	else if (op1.m_type)
		opValue2.createConst (NULL, op1.m_type);
	else 
		return true;

	ct::Value resultValue;
	bool result = op1.m_type->getModule ()->m_operatorMgr.binaryOperator (
		opKind, 
		opValue1, 
		opValue2, 
		&resultValue
		);

	return result && resultValue.getBool ();
}

//.............................................................................

uintptr_t
HashVariant::operator () (const Variant& variant)
{
	if (!variant.m_type)
		return 0;

	size_t size = variant.m_type->getSize ();
	if (size <= sizeof (uintptr_t) || variant.m_type->getTypeKind () == ct::TypeKind_DataPtr)
		return variant.m_uintptr;

	const void* p = size <= sizeof (DataPtr) ? &variant : variant.m_p;
	return sl::djb2 (p, size);
}

//.............................................................................

} // namespace rt
} // namespace jnc
