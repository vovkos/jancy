#include "pch.h"
#include "jnc_Variable.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CVariable::CVariable ()
{
	m_ItemKind = EModuleItem_Variable;
	m_pType = NULL;
	m_pType_i = NULL;
	m_PtrTypeFlags = 0;
	m_pScope = NULL;
	m_pTlsField = NULL;
	m_pLlvmValue = NULL;
	m_pLlvmAllocValue = NULL;
}

CValue
CVariable::GetScopeLevelObjHdr ()
{
	return m_StorageKind == EStorage_Stack ? 
		m_pModule->m_NamespaceMgr.GetScopeLevelObjHdr (m_pScope) :
		m_pModule->m_NamespaceMgr.GetStaticObjHdr ();
}

bool
CVariable::CalcLayout ()
{
	if (m_pType_i)
		m_pType = m_pType_i->GetActualType ();

	return m_pType->EnsureLayout ();
}

void
CVariable::EnsureLlvmValue ()
{
	if (m_pLlvmValue)
		return;

	ASSERT (m_StorageKind == EStorage_Thread);
	m_pModule->m_VariableMgr.AllocateTlsVariable (this);
}

//.............................................................................

} // namespace jnc {
