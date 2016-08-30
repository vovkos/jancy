#include "pch.h"
#include "jnc_ct_FunctionArg.h"

namespace jnc {
namespace ct {

//.............................................................................

FunctionArg::FunctionArg ()
{
	m_itemKind = ModuleItemKind_FunctionArg;
	m_type = NULL;
	m_ptrTypeFlags = 0;
}

//.............................................................................

} // namespace ct
} // namespace jnc

