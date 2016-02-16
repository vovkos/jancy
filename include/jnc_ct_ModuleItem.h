// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Lexer.h"

namespace jnc {
namespace ct {

class Module;
class Unit;
class Namespace;
class AttributeBlock;
class Type;
class DerivableType;
class ClassType;
class Function;
class Property;

//.............................................................................

enum ModuleItemKind
{
	ModuleItemKind_Undefined = 0,
	ModuleItemKind_Namespace,
	ModuleItemKind_Scope,
	ModuleItemKind_Type,
	ModuleItemKind_Typedef,
	ModuleItemKind_Alias,
	ModuleItemKind_Const,
	ModuleItemKind_Variable,
	ModuleItemKind_FunctionArg,
	ModuleItemKind_Function,
	ModuleItemKind_Property,
	ModuleItemKind_PropertyTemplate,
	ModuleItemKind_EnumConst,
	ModuleItemKind_StructField,
	ModuleItemKind_BaseTypeSlot,
	ModuleItemKind_Orphan,
	ModuleItemKind_Lazy,
	ModuleItemKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getModuleItemKindString (ModuleItemKind itemKind);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ModuleItemFlag
{
	ModuleItemFlag_User         = 0x01,
	ModuleItemFlag_NeedLayout   = 0x02,
	ModuleItemFlag_NeedCompile  = 0x04,
	ModuleItemFlag_InCalcLayout = 0x10,
	ModuleItemFlag_LayoutReady  = 0x20,
	ModuleItemFlag_Constructed  = 0x40, // fields, properties, base type slots
	ModuleItemFlag_Sealed       = 0x80,
};


//.............................................................................

enum StorageKind
{
	StorageKind_Undefined = 0,
	StorageKind_Alias,
	StorageKind_Typedef,
	StorageKind_Static,
	StorageKind_Thread,
	StorageKind_Stack,
	StorageKind_Heap,
	StorageKind_Member,
	StorageKind_Abstract,
	StorageKind_Virtual,
	StorageKind_Override,
	StorageKind_Mutable,
	StorageKind_This,
	StorageKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getStorageKindString (StorageKind storageKind);

//.............................................................................

enum AccessKind
{
	AccessKind_Undefined = 0,
	AccessKind_Public,
	AccessKind_Protected,
	AccessKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getAccessKindString (AccessKind accessKind);

//.............................................................................

class ModuleItemPos
{
	friend class Parser;

protected:
	Unit* m_parentUnit;
	Token::Pos m_pos;

public:
	ModuleItemPos ()
	{
		m_parentUnit = NULL;
	}

	Unit*
	getParentUnit ()
	{
		return m_parentUnit;
	}

	const Token::Pos*
	getPos ()
	{
		return &m_pos;
	}
};

//.............................................................................

class ModuleItemInitializer
{
	friend class Parser;

protected:
	sl::BoxList <Token> m_initializer;
	sl::String m_initializerString;

public:
	sl::ConstBoxList <Token>
	getInitializer ()
	{
		return m_initializer;
	}

	sl::String
	getInitializerString ();
};

//.............................................................................

class ModuleItemDecl: public ModuleItemPos
{
	friend class Parser;
	friend class Namespace;
	friend class ControlFlowMgr;
	friend class Orphan;

protected:
	StorageKind m_storageKind;
	AccessKind m_accessKind;
	sl::String m_name;
	sl::String m_qualifiedName;
	Namespace* m_parentNamespace;
	AttributeBlock* m_attributeBlock;

public:
	ModuleItemDecl ();

	StorageKind
	getStorageKind ()
	{
		return m_storageKind;
	}

	AccessKind
	getAccessKind ()
	{
		return m_accessKind;
	}

	bool
	isNamed ()
	{
		return !m_name.isEmpty ();
	}

	sl::String
	getName ()
	{
		return m_name;
	}

	sl::String
	getQualifiedName ()
	{
		return m_qualifiedName;
	}

	Namespace*
	getParentNamespace ()
	{
		return m_parentNamespace;
	}

	AttributeBlock*
	getAttributeBlock ()
	{
		return m_attributeBlock;
	}
};

//.............................................................................

class ModuleItem: public sl::ListLink
{
	friend class Module;
	friend class Parser;

protected:
	Module* m_module;
	ModuleItemKind m_itemKind;
	uint_t m_flags;
	ModuleItemDecl* m_itemDecl;

public:
	sl::String m_tag;

public:
	ModuleItem ();

	Module*
	getModule ()
	{
		return m_module;
	}

	ModuleItemKind
	getItemKind ()
	{
		return m_itemKind;
	}

	uint_t
	getFlags ()
	{
		return m_flags;
	}

	ModuleItemDecl*
	getItemDecl ()
	{
		return m_itemDecl;
	}

	bool
	ensureLayout ();

	virtual
	bool
	compile ()
	{
		ASSERT (false);
		return true;
	}

protected:
	virtual
	bool
	calcLayout ()
	{
		ASSERT (false);
		return true;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class UserModuleItem:
	public ModuleItem,
	public ModuleItemDecl
{
public:
	UserModuleItem ()
	{
		m_itemDecl = this;
	}
};

//.............................................................................

enum LazyModuleItemFlag
{
	LazyModuleItemFlag_Touched = 0x010000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LazyModuleItem: public ModuleItem
{
	friend class Namespace;

public:
	LazyModuleItem ()
	{
		m_itemKind = ModuleItemKind_Lazy;
	}

	virtual
	ModuleItem*
	getActualItem () = 0;
};

//.............................................................................

ModuleItem* 
verifyModuleItemKind (
	ModuleItem* item, 
	ModuleItemKind itemKind,
	const char* name
	);

inline
Type*
verifyModuleItemIsType (
	ModuleItem* item,
	const char* name
	)
{
	return (Type*) verifyModuleItemKind (item, ModuleItemKind_Type, name);
}

DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	);

ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	);

inline
Function*
verifyModuleItemIsFunction (	
	ModuleItem* item,
	const char* name
	)

{
	return (Function*) verifyModuleItemKind (item, ModuleItemKind_Function, name);
}

inline
Property*
verifyModuleItemIsProperty (
	ModuleItem* item,
	const char* name
	)
{
	return (Property*) verifyModuleItemKind (item, ModuleItemKind_Property, name);
}

//.............................................................................

} // namespace ct
} // namespace jnc
