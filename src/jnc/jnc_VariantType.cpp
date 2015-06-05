#include "pch.h"
#include "jnc_VariantType.h"
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

} // namespace jnc {
