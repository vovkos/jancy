#include "pch.h"
#include "jnc_Namespace.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

JNC_EXTERN_C
const char*
jnc_getNamespaceKindString (jnc_NamespaceKind namespaceKind)
{
	static const char* stringTable [jnc_NamespaceKind__Count] =
	{
		"undefined-namespace-kind",  // jnc_Namespace_Undefined = 0,
		"global-namespace",          // jnc_Namespace_Global,
		"scope",                     // jnc_Namespace_Scope,
		"named-type",                // jnc_Namespace_Type,
		"extension-namespace",       // jnc_Namespace_Extension,
		"property",                  // jnc_Namespace_Property,
		"property-template",         // jnc_Namespace_PropertyTemplate,
		"library",                   // jnc_Namespace_Library,
	};

	return (size_t) namespaceKind < jnc_NamespaceKind__Count ?
		stringTable [namespaceKind] :
		stringTable [jnc_NamespaceKind_Undefined];
}

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_GlobalNamespace_getItemDecl (jnc_GlobalNamespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_globalNamespaceFuncTable->m_getItemDeclFunc (nspace);
}

JNC_EXTERN_C
jnc_Namespace*
jnc_GlobalNamespace_getNamespace (jnc_GlobalNamespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_globalNamespaceFuncTable->m_getNamespaceFunc (nspace);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
size_t
jnc_Namespace_getItemCount (jnc_Namespace* nspace)
{
	return nspace->getItemCount ();
}

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Namespace_getItem (
	jnc_Namespace* nspace,
	size_t index
	)
{
	return nspace->getItem (index);
}

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

JNC_EXTERN_C
jnc_ClassType*
jnc_Namespace_findClassType (
	jnc_Namespace* nspace,
	const char* name
	)
{
	return nspace->findClassTypeByName (name);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_GlobalNamespace_getItemDecl (jnc_GlobalNamespace* nspace)
{
	return nspace;
}

JNC_EXTERN_C
jnc_Namespace*
jnc_GlobalNamespace_getNamespace (jnc_GlobalNamespace* nspace)
{
	return nspace;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
