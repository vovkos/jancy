#include "pch.h"
#include "jnc_Alias.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CAlias::CAlias ()
{
	m_ItemKind = EModuleItem_Alias;
	m_pType = NULL;
	m_pType_i = NULL;
}

bool
CAlias::CalcLayout ()
{
	if (m_pType_i)
		m_pType = m_pType_i->GetActualType ();

	return m_pType->EnsureLayout ();
}

//.............................................................................

} // namespace jnc {
