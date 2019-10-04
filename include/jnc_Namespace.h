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

#pragma once

#define _JNC_NAMESPACE_H

#include "jnc_ModuleItem.h"

/**

\defgroup namespace Namespace
	\ingroup module-subsystem
	\import{jnc_Namespace.h}

\addtogroup namespace
@{

\struct jnc_Namespace
	\verbatim

	Opaque structure used as a handle to Jancy namespace.

	Use functions from the `Namespace` group to access and manage the contents of this structure.

	\endverbatim

*/

#define JNC_GLOBAL_NAMESPACE_DOXID "global"

//..............................................................................

enum jnc_NamespaceKind
{
	jnc_NamespaceKind_Undefined,
	jnc_NamespaceKind_Global,
	jnc_NamespaceKind_Scope,
	jnc_NamespaceKind_Type,
	jnc_NamespaceKind_Extension,
	jnc_NamespaceKind_Property,
	jnc_NamespaceKind_PropertyTemplate,
	jnc_NamespaceKind_DynamicLib,
	jnc_NamespaceKind__Count,
};

typedef enum jnc_NamespaceKind jnc_NamespaceKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getNamespaceKindString(jnc_NamespaceKind namespaceKind);

//..............................................................................

JNC_EXTERN_C
size_t
jnc_Namespace_getItemCount(jnc_Namespace* nspace);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Namespace_getItem(
	jnc_Namespace* nspace,
	size_t index
	);

JNC_EXTERN_C
jnc_FindModuleItemResult
jnc_Namespace_findItem(
	jnc_Namespace* nspace,
	const char* name
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Namespace
{
	size_t
	getItemCount()
	{
		return jnc_Namespace_getItemCount(this);
	}

	jnc_ModuleItem*
	getItem(size_t index)
	{
		return jnc_Namespace_getItem(this, index);
	}

	jnc_FindModuleItemResult
	findItem(const char* name)
	{
		return jnc_Namespace_findItem(this, name);
	}
};
#endif // _JNC_CORE

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_GlobalNamespace: jnc_ModuleItem
{
};
#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_NamespaceKind NamespaceKind;

const NamespaceKind
	NamespaceKind_Undefined        = jnc_NamespaceKind_Undefined,
	NamespaceKind_Global           = jnc_NamespaceKind_Global,
	NamespaceKind_Scope            = jnc_NamespaceKind_Scope,
	NamespaceKind_Type             = jnc_NamespaceKind_Type,
	NamespaceKind_Extension        = jnc_NamespaceKind_Extension,
	NamespaceKind_Property         = jnc_NamespaceKind_Property,
	NamespaceKind_PropertyTemplate = jnc_NamespaceKind_PropertyTemplate,
	NamespaceKind_DynamicLib       = jnc_NamespaceKind_DynamicLib,
	NamespaceKind__Count           = jnc_NamespaceKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getNamespaceKindString(NamespaceKind namespaceKind)
{
	return jnc_getNamespaceKindString(namespaceKind);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
