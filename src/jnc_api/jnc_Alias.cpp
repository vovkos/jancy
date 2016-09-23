#include "pch.h"
#include "jnc_Alias.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
const char*
jnc_Alias_getInitializerString_v (jnc_Alias* alias)
{
	return *jnc::getTlsStringBuffer () = alias->getInitializerString ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
