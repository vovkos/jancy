#include "pch.h"
#include "jnc_Variant.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
variantRelationalOperator (
	BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	)
{
	ASSERT (opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge);

	jnc::Value opValue1;

	if (op1.m_type)
		opValue1.createConst (&op1, op1.m_type);
	else if (op2.m_type)
		opValue1.createConst (NULL, op2.m_type);
	else
		return true;

	jnc::Value opValue2;

	if (op2.m_type)
		opValue2.createConst (&op2, op2.m_type);
	else if (op1.m_type)
		opValue2.createConst (NULL, op1.m_type);
	else 
		return true;

	jnc::Value resultValue;
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
	if (size <= sizeof (uintptr_t) || variant.m_type->getTypeKind () == TypeKind_DataPtr)
		return variant.m_uintptr;

	const void* p = size <= sizeof (DataPtr) ? &variant : variant.m_p;
	return rtl::djb2 (p, size);
}

//.............................................................................

} // namespace jnc {
