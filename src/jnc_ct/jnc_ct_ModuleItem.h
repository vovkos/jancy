// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Lexer.h"
#include "jnc_ModuleItem.h"

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
class DoxyBlock;

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

protected:
	sl::String
	createDoxyLocationString ();
};

//.............................................................................

class ModuleItem: public sl::ListLink
{
	friend class Module;
	friend class Parser;

protected:
	Module* m_module;
	ModuleItemKind m_itemKind;
	DoxyBlock* m_doxyBlock;
	uint_t m_flags;

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
	getDecl ();

	Namespace*
	getNamespace ();

	Type*
	getType ();

	bool
	ensureLayout ();

	virtual
	bool
	compile ()
	{
		ASSERT (false);
		return true;
	}

	virtual
	bool
	generateDocumentation (
		const char* outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		)
	{
		return true;
	}

	DoxyBlock* 
	getDoxyBlock ();

protected:
	virtual
	bool
	calcLayout ()
	{
		ASSERT (false);
		return true;
	}

	virtual
	sl::String
	createDoxyRefId ();

	sl::String
	createDoxyDescriptionString ();
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
