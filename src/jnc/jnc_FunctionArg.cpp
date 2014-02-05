#include "pch.h"
#include "jnc_FunctionArg.h"

namespace jnc {

//.............................................................................

CFunctionArg::CFunctionArg ()
{
	m_ItemKind = EModuleItem_FunctionArg;
	m_pType = NULL;
	m_pType_i = NULL;
	m_PtrTypeFlags = 0;
}

bool
CFunctionArg::CalcLayout ()
{
	if (m_pType_i)
		m_pType = m_pType_i->GetActualType ();

	// TODO: check for valid argument type

	return m_pType->EnsureLayout ();
}

//.............................................................................

} // namespace jnc {

