// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"
#include "jnc_QualifiedName.h"
#include "jnc_UsingSet.h"

namespace jnc {

class Namespace;
class GlobalNamespace;
class ExtensionNamespace;
class DerivableType;
class EnumType;
class EnumConst;
class Const;
class MemberCoord;
class Value;

struct DualPtrTypeTuple;

//.............................................................................

enum
{
	TraverseKind_NoThis                = 0x01,
	TraverseKind_NoBaseType            = 0x04,
	TraverseKind_NoParentNamespace     = 0x08,
	TraverseKind_NoUsingNamespaces     = 0x10,
	TraverseKind_NoExtensionNamespaces = 0x20,
};

//.............................................................................

enum NamespaceKind
{
	NamespaceKind_Undefined,
	NamespaceKind_Global,
	NamespaceKind_Scope,
	NamespaceKind_Type,
	NamespaceKind_Extension,
	NamespaceKind_Property,
	NamespaceKind_PropertyTemplate,
	NamespaceKind_DynamicLib,
	NamespaceKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getNamespaceKindString (NamespaceKind namespaceKind);

//.............................................................................

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
	UsingSet m_usingSet;

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

	UsingSet*
	getUsingSet ()
	{
		return &m_usingSet;
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
	findItemByName (const char* name);

	ModuleItem*
	getItemByName (const char* name);

	Type*
	findTypeByName (const char* name)
	{
		ModuleItem* item = findItemByName (name);
		return item ? verifyModuleItemIsType (item, name) : NULL;
	}

	Type*
	getTypeByName (const char* name)
	{
		ModuleItem* item = getItemByName (name);
		return item ? verifyModuleItemIsType (item, name) : NULL;
	}

	DerivableType*
	findDerivableTypeByName (const char* name)
	{
		ModuleItem* item = findItemByName (name);
		return item ? verifyModuleItemIsDerivableType (item, name) : NULL;
	}

	DerivableType*
	getDerivableTypeByName (const char* name)
	{
		ModuleItem* item = getItemByName (name);
		return item ? verifyModuleItemIsDerivableType (item, name) : NULL;
	}

	ClassType*
	findClassTypeByName (const char* name)
	{
		ModuleItem* item = findItemByName (name);
		return item ? verifyModuleItemIsClassType (item, name) : NULL;
	}

	ClassType*
	getClassTypeByName (const char* name)
	{
		ModuleItem* item = getItemByName (name);
		return item ? verifyModuleItemIsClassType (item, name) : NULL;
	}

	Function*
	findFunctionByName (const char* name)
	{
		ModuleItem* item = findItemByName (name);
		return item ? verifyModuleItemIsFunction (item, name) : NULL;
	}

	Function*
	getFunctionByName (const char* name)
	{
		ModuleItem* item = getItemByName (name);
		return item ? verifyModuleItemIsFunction (item, name) : NULL;
	}

	Property*
	findPropertyByName (const char* name)
	{
		ModuleItem* item = findItemByName (name);
		return item ? verifyModuleItemIsProperty (item, name) : NULL;
	}

	Property*
	getPropertyByName (const char* name)
	{
		ModuleItem* item = getItemByName (name);
		return item ? verifyModuleItemIsProperty (item, name) : NULL;
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

	Const*
	createConst (
		const rtl::String& name,
		const Value& value
		);

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

class ExtensionNamespace: public GlobalNamespace
{
	friend class NamespaceMgr;

protected:
	DerivableType* m_type;

public:
	ExtensionNamespace ()
	{
		m_namespaceKind = NamespaceKind_Extension;
		m_type = NULL;
	}

	DerivableType* getType ()
	{
		return m_type;
	}
};

//.............................................................................

class DynamicLibNamespace: public GlobalNamespace
{
	friend class NamespaceMgr;

protected:
	ClassType* m_dynamicLibType;

public:
	DynamicLibNamespace ()
	{
		m_namespaceKind = NamespaceKind_DynamicLib;
		m_dynamicLibType = NULL;
	}

	ClassType* getLibraryType ()
	{
		return m_dynamicLibType;
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
