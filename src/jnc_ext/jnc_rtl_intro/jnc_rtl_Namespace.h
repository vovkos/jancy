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

#include "jnc_rtl_ModuleItem.h"
#include "jnc_ct_GlobalNamespace.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Namespace)
JNC_DECLARE_OPAQUE_CLASS_TYPE(GlobalNamespace)

//..............................................................................

class Namespace: public ModuleItemDecl
{
public:
	Namespace(ct::Namespace* nspace):
		ModuleItemDecl(nspace)
	{
	}

	NamespaceKind
	JNC_CDECL
	getNamespaceKind()
	{
		return n()->getNamespaceKind();
	}

	size_t
	JNC_CDECL
	getItemCount()
	{
		return n()->getItemCount();
	}

	ModuleItem*
	JNC_CDECL
	getItem(size_t index)
	{
		return rtl::getModuleItem(n()->getItem(index));
	}

	ModuleItem*
	JNC_CDECL
	findItem(DataPtr namePtr)
	{
		return rtl::getModuleItem(n()->findItemByName((char*)namePtr.m_p));
	}

	Variable*
	JNC_CDECL
	findVariable(DataPtr namePtr)
	{
		ct::ModuleItem* item = n()->findItemByName((char*)namePtr.m_p);
		return item && item->getItemKind() == ModuleItemKind_Variable ? rtl::getVariable((ct::Variable*)item) : NULL;
	}

	Function*
	JNC_CDECL
	findFunction(DataPtr namePtr)
	{
		ct::ModuleItem* item = n()->findItemByName((char*)namePtr.m_p);
		return item && item->getItemKind() == ModuleItemKind_Function ? rtl::getFunction((ct::Function*)item) : NULL;
	}

	Property*
	JNC_CDECL
	findProperty(DataPtr namePtr)
	{
		ct::ModuleItem* item = n()->findItemByName((char*)namePtr.m_p);
		return item && item->getItemKind() == ModuleItemKind_Property ? rtl::getProperty((ct::Property*)item) : NULL;
	}

	ClassType*
	JNC_CDECL
	findClassType(DataPtr namePtr)
	{
		ct::ModuleItem* item = n()->findItemByName((char*)namePtr.m_p);
		return
			item &&
			item->getItemKind() == ModuleItemKind_Type &&
			((ct::Type*)item)->getTypeKind() == TypeKind_Class ? (ClassType*)rtl::getType((ct::Type*)item) : NULL;
	}

protected:
	ct::Namespace*
	n()
	{
		return (ct::Namespace*)m_decl;
	}
};

//..............................................................................

class GlobalNamespace:
	public ModuleItemBase<ct::GlobalNamespace>,
	public Namespace
{
public:
	GlobalNamespace(ct::GlobalNamespace* nspace):
		ModuleItemBase(nspace),
		Namespace(nspace)
	{
	}
};

//..............................................................................

Namespace*
JNC_CDECL
getNamespace(ct::Namespace* nspace);

//..............................................................................

} // namespace rtl
} // namespace jnc
