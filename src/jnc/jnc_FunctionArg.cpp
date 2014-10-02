#include "pch.h"
#include "jnc_FunctionArg.h"

namespace jnc {

//.............................................................................

FunctionArg::FunctionArg ()
{
	m_itemKind = ModuleItemKind_FunctionArg;
	m_type = NULL;
	m_type_i = NULL;
	m_ptrTypeFlags = 0;
}

bool
FunctionArg::calcLayout ()
{
	if (m_type_i)
		m_type = m_type_i->getActualType ();

	// TODO: check for valid argument type

	return m_type->ensureLayout ();
}

//.............................................................................

} // namespace jnc {

