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

#include "jnc_ModuleItem.h"
#include "jnc_ct_Lexer.h"
#include "jnc_ct_UnitMgr.h"

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

//..............................................................................

class ModuleItemInitializer
{
	friend class Parser;

protected:
	sl::BoxList<Token> m_initializer;

public:
	bool
	hasInitializer()
	{
		return !m_initializer.isEmpty();
	}

	const sl::BoxList<Token>&
	getInitializer()
	{
		return m_initializer;
	}

	sl::String
	getInitializerString()
	{
		return Token::getTokenListString(m_initializer);
	}
};

//..............................................................................

class ModuleItemPos
{
	friend class Parser;

protected:
	Unit* m_parentUnit;
	lex::LineCol m_pos;

public:
	ModuleItemPos()
	{
		m_parentUnit = NULL;
	}

	Unit*
	getParentUnit() const
	{
		return m_parentUnit;
	}

	const lex::LineCol&
	getPos() const
	{
		return m_pos;
	}

	void
	pushSrcPosError()
	{
		lex::pushSrcPosError(m_parentUnit->getFilePath(), m_pos);
	}

	void
	ensureSrcPosError()
	{
		lex::ensureSrcPosError(m_parentUnit->getFilePath(), m_pos);
	}
};

//..............................................................................

class ModuleItemDecl: public ModuleItemPos
{
	friend class ModuleItem;
	friend class DoxyHost;
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
	dox::Block* m_doxyBlock;

public:
	ModuleItemDecl();

	StorageKind
	getStorageKind()
	{
		return m_storageKind;
	}

	AccessKind
	getAccessKind()
	{
		return m_accessKind;
	}

	bool
	isNamed()
	{
		return !m_name.isEmpty();
	}

	const sl::String&
	getName()
	{
		return m_name;
	}

	const sl::String&
	getQualifiedName();

	Namespace*
	getParentNamespace()
	{
		return m_parentNamespace;
	}

	AttributeBlock*
	getAttributeBlock()
	{
		return m_attributeBlock;
	}

	dox::Block*
	getDoxyBlock()
	{
		return m_doxyBlock;
	}

	bool
	ensureAttributeValuesReady();

protected:
	void
	prepareQualifiedName();

	sl::String
	getDoxyLocationString();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const sl::String&
ModuleItemDecl::getQualifiedName()
{
	if (m_qualifiedName.isEmpty())
		prepareQualifiedName();

	return m_qualifiedName;
}

//..............................................................................

class ModuleItemBodyDecl: public ModuleItemDecl
{
	friend class Parser;

protected:
	lex::LineColOffset m_bodyPos;
	sl::StringRef m_body;
	sl::BoxList<Token> m_bodyTokenList;

public:
	bool
	hasBody() const
	{
		return !m_body.isEmpty() || !m_bodyTokenList.isEmpty();
	}

	const lex::LineColOffset&
	getBodyPos() const
	{
		return m_bodyPos;
	}

	const sl::StringRef&
	getBody() const
	{
		return m_body;
	}

	bool
	setBody(
		const lex::LineColOffset& pos,
		const sl::StringRef& body
		);

	bool
	setBody(sl::BoxList<Token>* tokenList);

protected:
	bool
	canSetBody();
};

//..............................................................................

class ModuleItem: public sl::ListLink
{
	friend class Module;
	friend class Parser;

protected:
	Module* m_module;
	ModuleItemKind m_itemKind;
	uint_t m_flags;

public:
	ModuleItem();

	virtual
	~ModuleItem ()
	{
	}

	Module*
	getModule()
	{
		return m_module;
	}

	ModuleItemKind
	getItemKind()
	{
		return m_itemKind;
	}

	uint_t
	getFlags()
	{
		return m_flags;
	}

	ModuleItemDecl*
	getDecl();

	Namespace*
	getNamespace();

	Type*
	getType();

	sl::String
	getSynopsis(bool isQualifiedName = true);

	virtual
	bool
	require()
	{
		err::setFormatStringError("don't know how to require '%s'", getModuleItemKindString(m_itemKind));
		return false;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		)
	{
		return true;
	}

	virtual
	sl::String
	createDoxyRefId();
};

//..............................................................................

} // namespace ct
} // namespace jnc
