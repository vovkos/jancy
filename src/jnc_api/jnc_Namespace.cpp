//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_Namespace.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Namespace.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getNamespaceKindString(jnc_NamespaceKind namespaceKind)
{
	static const char* stringTable[jnc_NamespaceKind__Count] =
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

	return (size_t)namespaceKind < jnc_NamespaceKind__Count ?
		stringTable[namespaceKind] :
		stringTable[jnc_NamespaceKind_Undefined];
}

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Namespace_isReady(jnc_Namespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_isReadyFunc(nspace);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_NamespaceKind
jnc_Namespace_getNamespaceKind(jnc_Namespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_getNamespaceKindFunc(nspace);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Namespace*
jnc_Namespace_getParentNamespace(jnc_Namespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_getParentNamespaceFunc(nspace);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Namespace_getParentItem(jnc_Namespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_getParentItemFunc(nspace);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Namespace_getItemCount(jnc_Namespace* nspace)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_getItemCountFunc(nspace);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Namespace_getItem(
	jnc_Namespace* nspace,
	size_t index
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_getItemFunc(nspace, index);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FindModuleItemResult
jnc_Namespace_findItem(
	jnc_Namespace* nspace,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findItemFunc(nspace, name);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Namespace_isReady(jnc_Namespace* nspace)
{
	return nspace->isNamespaceReady();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_NamespaceKind
jnc_Namespace_getNamespaceKind(jnc_Namespace* nspace)
{
	return nspace->getNamespaceKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Namespace*
jnc_Namespace_getParentNamespace(jnc_Namespace* nspace)
{
	return nspace->getParentNamespace();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Namespace_getParentItem(jnc_Namespace* nspace)
{
	return nspace->getParentItem();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Namespace_getItemCount(jnc_Namespace* nspace)
{
	return nspace->getItemArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Namespace_getItem(
	jnc_Namespace* nspace,
	size_t index
	)
{
	return nspace->getItemArray()[index];
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FindModuleItemResult
jnc_Namespace_findItem(
	jnc_Namespace* nspace,
	const char* name
	)
{
	return nspace->findItem(name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
