#include "pch.h"
#include "jnc_Multicast.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Multicast_getCallMethod (jnc_Multicast* multicast)
{
	return jnc_g_dynamicExtensionLibHost->m_multicastFuncTable->m_getCallMethodFunc (multicast);
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshot_getCallMethod (jnc_McSnapshot* snapshot)
{
	return jnc_g_dynamicExtensionLibHost->m_multicastFuncTable->m_getSnapshotCallMethodFunc (snapshot);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Multicast_getCallMethod (jnc_Multicast* multicast)
{
	using namespace jnc;
	ct::MulticastClassType* type = (ct::MulticastClassType*) multicast->m_box->m_type;
	return type->getMethod (ct::MulticastMethodKind_Call);
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshot_getCallMethod (jnc_McSnapshot* snapshot)
{
	using namespace jnc;
	ct::McSnapshotClassType* type = (ct::McSnapshotClassType*) snapshot->m_box->m_type;
	return type->getMethod (ct::McSnapshotMethodKind_Call);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
