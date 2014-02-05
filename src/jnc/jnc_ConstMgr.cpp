#include "pch.h"
#include "jnc_ConstMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CConstMgr::CConstMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);
}

void
CConstMgr::Clear ()
{
	m_ConstList.Clear ();
	m_UnsafeLeanDataPtrValidator = ref::EPtr_Null;
}

CLeanDataPtrValidator*
CConstMgr::GetUnsafeLeanDataPtrValidator ()
{
	if (m_UnsafeLeanDataPtrValidator)
		return m_UnsafeLeanDataPtrValidator;

	void* pRangeBegin = NULL;

	m_UnsafeLeanDataPtrValidator = AXL_REF_NEW (CLeanDataPtrValidator);
	m_UnsafeLeanDataPtrValidator->m_SizeValue.SetConstSizeT (-1);
	m_UnsafeLeanDataPtrValidator->m_ScopeValidatorValue.SetConstSizeT (0);
	m_UnsafeLeanDataPtrValidator->m_RangeBeginValue.CreateConst (&pRangeBegin, m_pModule->GetSimpleType (EStdType_BytePtr));

	return m_UnsafeLeanDataPtrValidator;
}

const CValue& 
CConstMgr::SaveLiteral (
	const char* p,
	size_t Length
	)
{
	if (Length == -1)
		Length = strlen (p);

	CValue Value;
	Value.SetCharArray (p, Length + 1);
	return SaveValue (Value);
}

//.............................................................................

} // namespace jnc {
