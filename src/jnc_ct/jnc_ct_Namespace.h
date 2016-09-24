// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_QualifiedName.h"
#include "jnc_ct_UsingSet.h"
#include "jnc_Namespace.h"

namespace jnc {
namespace ct {

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Namespace: public ModuleItemDecl
{
	friend class NamespaceMgr;
	friend class TypeMgr;
	friend class Parser;

protected:
	NamespaceKind m_namespaceKind;

	sl::Array <ModuleItem*> m_itemArray;
	sl::Array <DoxyBlock*> m_footnoteArray;
	sl::StringHashTableMap <ModuleItem*> m_itemMap;
	sl::StringHashTable m_friendSet;
	sl::StringHashTableMap <DualPtrTypeTuple*> m_dualPtrTypeTupleMap;
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

	sl::String
	createQualifiedName (const char* name);

	sl::String
	createQualifiedName (const sl::String& name)
	{
		return createQualifiedName (name.cc ());
	}

	sl::String
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
	findItem (const sl::StringRef& name)
	{
		return findItem (name.cc ());
	}

	ModuleItem*
	findItem (const QualifiedName& name);

	ModuleItem*
	findItemTraverse (
		const sl::String& name,
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
		return addItem (item->m_name, item);
	}

	size_t
	addFunction (Function* function); // returns overload idx or -1 on error

	void
	addFootnote (DoxyBlock* footnote)
	{
		m_footnoteArray.append (footnote);
	}

	Const*
	createConst (
		const sl::String& name,
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
		const char* name,
		ModuleItem* item
		);
	
	virtual
	ModuleItem*
	findItemTraverseImpl (
		const char* name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
		);

	bool
	generateMemberDocumentation (
		const char* outputDir,
		sl::String* itemXml,
		sl::String* indexXml,
		bool useSectionDef
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
	}

	virtual
	sl::String
	createDoxyRefId ();

	virtual
	bool
	generateDocumentation (
		const char* outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
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

JNC_INLINE
err::Error
setRedefinitionError (const char* name)
{
	return err::setFormatStringError ("redefinition of '%s'", name);
}

//.............................................................................

} // namespace ct
} // namespace jnc
