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
#include "jnc_rtl_Namespace.h"
#include "jnc_rtl_Property.h"
#include "jnc_rtl_Type.h"
#include "jnc_ct_Namespace.h"
#include "jnc_ct_Type.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Namespace,
	"jnc.Namespace",
	sl::g_nullGuid,
	-1,
	Namespace,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Namespace)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Namespace, ct::Namespace*>))
	JNC_MAP_CONST_PROPERTY("m_namespaceKind", &Namespace::getNamespaceKind)
	JNC_MAP_CONST_PROPERTY("m_itemCount", &Namespace::getItemCount)
	JNC_MAP_CONST_PROPERTY("m_itemArray", &Namespace::getItem)
	JNC_MAP_FUNCTION("findItem", &Namespace::findItem)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	GlobalNamespace,
	"jnc.GlobalNamespace",
	sl::g_nullGuid,
	-1,
	GlobalNamespace,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(GlobalNamespace)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<GlobalNamespace, ct::GlobalNamespace*>))
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Namespace*
JNC_CDECL
getNamespace(ct::Namespace* nspace)
{
	NamespaceKind nspaceKind = nspace->getNamespaceKind();

	switch (nspaceKind)
	{
	case NamespaceKind_Global:
		return getGlobalNamespace((ct::GlobalNamespace*)nspace);

	case NamespaceKind_Type:
		return (NamedType*)getType((ct::NamedType*)nspace);

	case NamespaceKind_Property:
		return getProperty((ct::Property*)nspace);

	default:
		return (Namespace*)getIntrospectionClass(nspace, StdType_Namespace);
	};
}

//..............................................................................

} // namespace rtl
} // namespace jnc
