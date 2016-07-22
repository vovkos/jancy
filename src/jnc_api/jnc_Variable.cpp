#include "pch.h"
#include "jnc_Variable.h"

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
jnc_ModuleItemDecl*
jnc_Variable_getItemDecl (jnc_Variable* variable)
{
	return variable;
}

JNC_EXTERN_C
jnc_Type*
jnc_Variable_getType (jnc_Variable* variable)
{
	return variable->getType ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
