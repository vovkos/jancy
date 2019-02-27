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
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
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
jnc_Variable*
jnc_Namespace_findVariable(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findVariableFunc(nspace, name, isRequired);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_Namespace_findFunction(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findFunctionFunc(nspace, name, isRequired);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Property*
jnc_Namespace_findProperty(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findPropertyFunc(nspace, name, isRequired);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassType*
jnc_Namespace_findClassType(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return jnc_g_dynamicExtensionLibHost->m_namespaceFuncTable->m_findClassTypeFunc(nspace, name, isRequired);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Namespace_getItemCount(jnc_Namespace* nspace)
{
	return nspace->getItemCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleItem*
jnc_Namespace_getItem(
	jnc_Namespace* nspace,
	size_t index
	)
{
	return nspace->getItem(index);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Variable*
jnc_Namespace_findVariable(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return isRequired ?
		nspace->getVariableByName(name) :
		nspace->findVariableByName(name);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_Namespace_findFunction(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return isRequired ?
		nspace->getFunctionByName(name) :
		nspace->findFunctionByName(name);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Property*
jnc_Namespace_findProperty(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return isRequired ?
		nspace->getPropertyByName(name) :
		nspace->findPropertyByName(name);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ClassType*
jnc_Namespace_findClassType(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	)
{
	return isRequired ?
		nspace->getClassTypeByName(name) :
		nspace->findClassTypeByName(name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
