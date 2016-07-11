#include "pch.h"
#include "jnc_Namespace.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_ct_Namespace.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Namespace_findFunction (
	jnc_Namespace* nspace,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findFunctionFunc (nspace, name);
}

JNC_EXTERN_C
jnc_Property*
jnc_Namespace_findProperty (
	jnc_Namespace* nspace,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findPropertyFunc (nspace, name);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_Namespace_findFunction (
	jnc_Namespace* nspace,
	const char* name
	)
{
	return nspace->findFunctionByName (name);
}

JNC_EXTERN_C
jnc_Property*
jnc_Namespace_findProperty (
	jnc_Namespace* nspace,
	const char* name
	)
{
	return nspace->findPropertyByName (name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
