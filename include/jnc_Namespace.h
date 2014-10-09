// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"
#include "jnc_QualifiedName.h"

namespace jnc {

class Namespace;
class EnumType;
class EnumConst;
class MemberCoord;

struct DualPtrTypeTuple;

//.............................................................................

enum NamespaceKind
{
	NamespaceKind_Undefined,
	NamespaceKind_Global,
	NamespaceKind_Scope,
	NamespaceKind_Type,
	NamespaceKind_TypeExtension,
	NamespaceKind_Property,
	NamespaceKind_PropertyTemplate,
	NamespaceKind__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getNamespaceKindString (NamespaceKind namespaceKind);

//.............................................................................

enum
{
	TraverseKind_NoThis               = 0x01,
	TraverseKind_NoExtensionNamespace = 0x02,
	TraverseKind_NoBaseType           = 0x04,
	TraverseKind_NoParentNamespace    = 0x08,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Namespace: public ModuleItemDecl
{
	friend class NamespaceMgr;
	friend class TypeMgr;
	friend class Parser;

protected:
	NamespaceKind m_namespaceKind;

	rtl::Array <ModuleItem*> m_itemArray;
	rtl::StringHashTableMap <ModuleItem*> m_itemMap;
	rtl::StringHashTable m_friendSet;
	rtl::StringHashTableMap <DualPtrTypeTuple*> m_dualPtrTypeTupleMap;

public:
	Namespace ()
	{
		m_namespaceKind = NamespaceKind_Undefined;
	}

	NamespaceKind
	getNamespaceKind ()
	{
		return m_namespaceKind;
	}

	rtl::String
	createQualifiedName (const char* name);

	rtl::String
	createQualifiedName (const rtl::String& name)
	{
		return createQualifiedName (name.cc ());
	}

	rtl::String
	createQualifiedName (const QualifiedName& name)
	{
		return createQualifiedName (name.getFullName ());
	}

	bool
	isFriend (Namespace* nspace)
	{
		return m_friendSet.find (nspace->m_qualifiedName) != NULL;
	}

	ModuleItem*
	getItemByName (const char* name);

	Type*
	getTypeByName (const char* name)
	{
		return verifyModuleItemIsType (getItemByName (name), name);
	}

	DerivableType*
	getDerivableTypeByName (const char* name)
	{
		return verifyModuleItemIsDerivableType (getItemByName (name), name);
	}

	ClassType*
	getClassTypeByName (const char* name)
	{
		return verifyModuleItemIsClassType (getItemByName (name), name);
	}

	Function*
	getFunctionByName (const char* name)
	{
		return verifyModuleItemIsFunction (getItemByName (name), name);
	}

	Function*
	getFunctionByName (
		const char* name,
		size_t overloadIdx
		);

	Property*
	getPropertyByName (const char* name)
	{
		return verifyModuleItemIsProperty (getItemByName (name), name);
	}

	ModuleItem*
	findItem (const char* name);

	ModuleItem*
	findItem (const rtl::String& name)
	{
		return findItem (name.cc ());
	}

	ModuleItem*
	findItem (const QualifiedName& name);

	ModuleItem*
	findItemTraverse (
		const rtl::String& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		)
	{
		return findItemTraverseImpl (name, coord, flags);
	}

	ModuleItem*
	findItemTraverse (
		const char* name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		)
	{
		return findItemTraverseImpl (name, coord, flags);
	}

	ModuleItem*
	findItemTraverse (
		const QualifiedName& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		);

	template <typename T>
	bool
	addItem (T* item)
	{
		return addItem (item, item);
	}

	size_t
	addFunction (Function* function); // returns overload idx or -1 on error

	size_t
	getItemCount ()
	{
		return m_itemArray.getCount ();
	}

	ModuleItem*
	getItem (size_t index)
	{
		ASSERT (index < m_itemArray.getCount ());
		return m_itemArray [index];
	}

	bool
	exposeEnumConsts (EnumType* member);

protected:
	void
	clear ();

	bool
	addItem (
		ModuleItem* item,
		ModuleItemDecl* decl
		);

	virtual
	ModuleItem*
	findItemTraverseImpl (
		const char* name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		);
};

//.............................................................................

enum GlobalNamespaceFlag
{
	GlobalNamespaceFlag_Sealed = 0x0100
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class GlobalNamespace:
	public ModuleItem,
	public Namespace
{
	friend class NamespaceMgr;

public:
	GlobalNamespace ()
	{
		m_itemKind = ModuleItemKind_Namespace;
		m_namespaceKind = NamespaceKind_Global;
		m_itemDecl = this;
	}
};

//.............................................................................

inline
err::Error
setRedefinitionError (const char* name)
{
	return err::setFormatStringError ("redefinition of '%s'", name);
}

//.............................................................................

} // namespace jnc {
