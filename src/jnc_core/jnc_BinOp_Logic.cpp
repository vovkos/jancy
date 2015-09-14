#include "pch.h"
#include "jnc_BinOp_Logic.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Type*
BinOp_LogAnd::getResultType (
	const Value& opValue1,
	const Value& opValue2
	)
{
	return m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
}

//.............................................................................

Type*
BinOp_LogOr::getResultType (
	const Value& opValue1,
	const Value& opValue2
	)
{
	return m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
}

//.............................................................................

} // namespace jnc {
