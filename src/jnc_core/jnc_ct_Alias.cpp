#include "pch.h"
#include "jnc_ct_Alias.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Alias::Alias ()
{
	m_itemKind = ModuleItemKind_Alias;
	m_type = NULL;
	m_type_i = NULL;
}

bool
Alias::calcLayout ()
{
	if (m_type_i)
		m_type = m_type_i->getActualType ();

	return m_type->ensureLayout ();
}

//.............................................................................

} // namespace ct
} // namespace jnc
