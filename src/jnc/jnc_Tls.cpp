#include "pch.h"
#include "jnc_Tls.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

Tls*
getTls ()
{
	Tls* tls = mt::getTlsSlotValue <Tls> ();
	ASSERT (tls && tls->m_stackEpoch);

	// check for stack overflow


	return tls;
}

//.............................................................................

} // namespace jnc 
