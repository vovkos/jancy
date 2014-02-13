#include "pch.h"
#include "jnc_BinOp_Logic.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CBinOp_LogAnd::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
}

//.............................................................................

CType*
CBinOp_LogOr::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
}

//.............................................................................

} // namespace jnc {
