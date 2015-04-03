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

	Module* module = NULL;
		op1.m_type ? op1.m_type->getModule () :
		op2.m_type ? op2.m_type->getModule () :
		NULL;

	if (!module)
		return true; // 2 null-variants are equal

	jnc::Value opValue1;
	jnc::Value opValue2;
	jnc::Value resultValue;

	if (op1.m_type)
		opValue1.createConst (&op1, op1.m_type);
	else
		opValue1.setNull (module);

	if (op2.m_type)
		opValue2.createConst (&op2, op2.m_type);
	else
		opValue2.setNull (module);

	bool result = module->m_operatorMgr.binaryOperator (opKind, opValue1, opValue2, &resultValue);
	return result && resultValue.getBool ();
}

//.............................................................................

} // namespace jnc {
