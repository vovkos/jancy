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
#include "jnc_ct_Module.h"
#include "jnc_ct_Closure.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_DynamicLibClassType.h"
#include "jnc_ct_DynamicLibNamespace.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_AsyncLauncherFunction.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_rtl_Regex.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_ct_Parser.llk.cpp"

namespace jnc {
namespace ct {

//..............................................................................

Parser::Parser(
	Module* module,
	Mode mode
	):
	m_doxyParser(&module->m_doxyModule)
{
	m_module = module;
	m_mode = mode;
	m_fieldAlignment = 8;
	m_defaultFieldAlignment = 8;
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_attributeBlock = NULL;
	m_lastDeclaredItem = NULL;
	m_lastPropertyGetterType = NULL;
	m_lastPropertyTypeModifiers = 0;
	m_reactorType = NULL;
	m_reactionIdx = 0;
	m_constructorType = NULL;
	m_constructorProperty = NULL;
	m_namedType = NULL;
	m_topDeclarator = NULL;
}

bool
Parser::tokenizeBody(
	sl::BoxList<Token>* tokenList,
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	Lexer lexer(m_mode == Mode_Parse ? LexerMode_Parse : LexerMode_Compile);

	if ((m_module->getCompileFlags() & ModuleCompileFlag_Documentation) && !unit->getLib())
		lexer.m_channelMask = TokenChannelMask_All; // also include doxy-comments (but not for libs!)

	lexer.create(unit->getFilePath(), body);
	lexer.setLineCol(pos);

	char buffer[256];
	sl::Array<Token*> scopeAnchorTokenStack(ref::BufKind_Stack, buffer, sizeof(buffer));
	bool isScopeFlagMarkupRequired = m_mode != Mode_Parse;

	for (;;)
	{
		const Token* token = lexer.getToken();
		switch (token->m_token)
		{
		case TokenKind_Eof: // don't add EOF token (parseTokenList adds one automatically)
			return true;

		case TokenKind_Error:
			err::setFormatStringError("invalid character '\\x%02x'", (uchar_t) token->m_data.m_integer);
			lex::pushSrcPosError(unit->getFilePath(), token->m_pos);
			return false;
		}

		tokenList->insertTail(*token);
		lexer.nextToken();

		Token* anchorToken;

		if (isScopeFlagMarkupRequired)
			switch (token->m_token)
			{
			case '{':
				anchorToken = tokenList->getTail().p();
				anchorToken->m_data.m_integer = 0; // tokens can be reused, ensure 0
				scopeAnchorTokenStack.append(anchorToken);
				break;

			case '}':
				scopeAnchorTokenStack.pop();
				break;

			case TokenKind_NestedScope:
			case TokenKind_Case:
			case TokenKind_Default:
				if (!scopeAnchorTokenStack.isEmpty())
				{
					anchorToken = tokenList->getTail().p();
					anchorToken->m_data.m_integer = 0; // tokens can be reused, ensure 0
					scopeAnchorTokenStack.getBack() = anchorToken;
				}
				break;

			case TokenKind_Catch:
				if (!scopeAnchorTokenStack.isEmpty())
					scopeAnchorTokenStack.getBack()->m_data.m_integer |= ScopeFlag_CatchAhead | ScopeFlag_HasCatch;
				break;

			case TokenKind_Finally:
				if (!scopeAnchorTokenStack.isEmpty())
					scopeAnchorTokenStack.getBack()->m_data.m_integer |= ScopeFlag_FinallyAhead | ScopeFlag_Finalizable;
				break;
			}
	}
}

bool
Parser::parseBody(
	SymbolKind symbol,
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	sl::BoxList<Token> tokenList;

	bool result = tokenizeBody(&tokenList, pos, body);
	if (!result)
		return false;

	return !tokenList.isEmpty() ?
		parseTokenList(symbol, tokenList) :
		create(symbol) && parseEofToken(pos);
}

bool
Parser::parseTokenList(
	SymbolKind symbol,
	const sl::ConstBoxList<Token>& tokenList
	)
{
	ASSERT(!tokenList.isEmpty());

	create(symbol);

	sl::ConstBoxIterator<Token> token = tokenList.getHead();
	for (; token; token++)
	{
		bool result = parseToken(&*token);
		if (!result)
		{
			lex::ensureSrcPosError(m_module->m_unitMgr.getCurrentUnit()->getFilePath(), token->m_pos);
			return false;
		}
	}

	// important: process EOF token, it might actually trigger actions!

	return parseEofToken(tokenList.getTail()->m_pos);
}

bool
Parser::parseEofToken(const lex::LineCol& pos)
{
	Token eofToken;
	eofToken.m_token = 0;
	eofToken.m_pos.m_line = pos.m_line;
	eofToken.m_pos.m_col = pos.m_col;

	bool result = parseToken(&eofToken);
	if (!result)
	{
		lex::ensureSrcPosError(m_module->m_unitMgr.getCurrentUnit()->getFilePath(), pos);
		return false;
	}

	return true;
}

bool
Parser::isTypeSpecified()
{
	if (m_typeSpecifierStack.isEmpty())
		return false;

	// if we've seen 'unsigned', assume 'int' is implied.
	// checking for 'property' is required for full property syntax e.g.:
	// property foo { int get (); }
	// here 'foo' should be a declarator, not import-type-specifier
	// the same logic applies to 'reactor'

	TypeSpecifier* typeSpecifier = m_typeSpecifierStack.getBack();
	return
		typeSpecifier->getType() != NULL ||
		typeSpecifier->getTypeModifiers() & (TypeModifier_Unsigned | TypeModifier_Property | TypeModifier_Reactor);
}

NamedImportType*
Parser::getNamedImportType(
	const QualifiedName& name,
	const lex::LineCol& pos
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamedImportType* type = m_module->m_typeMgr.getNamedImportType(name, nspace);

	if (!type->m_parentUnit)
	{
		type->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
		type->m_pos = pos;
	}

	return type;
}

Type*
Parser::findType(
	size_t baseTypeIdx,
	const QualifiedName& name,
	const lex::LineCol& pos
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	ModuleItem* item;

	if (m_mode == Mode_Parse)
	{
		if (baseTypeIdx != -1)
			return NULL;

		if (!name.isSimple())
			return getNamedImportType(name, pos);

		sl::String shortName = name.getShortName();
		FindModuleItemResult findResult = nspace->findDirectChildItem(shortName);
		if (!findResult.m_result)
			return NULL;

		if (!findResult.m_item)
			return getNamedImportType(name, pos);

		item = findResult.m_item;
	}
	else
	{
		if (baseTypeIdx != -1)
		{
			DerivableType* baseType = findBaseType(baseTypeIdx);
			if (!baseType)
				return NULL;

			nspace = baseType;

			if (name.isEmpty())
				return baseType;
		}

		FindModuleItemResult findResult = nspace->findItemTraverse(name);
		if (!findResult.m_item)
			return NULL;

		item = findResult.m_item;
	}

	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Type:
		return (Type*)item;

	case ModuleItemKind_Typedef:
		return (m_module->getCompileFlags() & ModuleCompileFlag_KeepTypedefShadow) ?
			((Typedef*)item)->getShadowType() :
			((Typedef*)item)->getType();

	default:
		return NULL;
	}
}

Type*
Parser::getType(
	size_t baseTypeIdx,
	const QualifiedName& name,
	const lex::LineCol& pos
	)
{
	Type* type = findType(baseTypeIdx, name, pos);
	if (!type)
	{
		if (baseTypeIdx == -1)
			err::setFormatStringError("'%s' is not found or not a type", name.getFullName ().sz());
		else if (name.isEmpty())
			err::setFormatStringError("'basetype%d' is not found", baseTypeIdx + 1);
		else
			err::setFormatStringError("'basetype%d.%s' is not found or not a type", baseTypeIdx + 1, name.getFullName ().sz());

		return NULL;
	}

	return type;
}

bool
Parser::setSetAsType(Type* type)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	if (nspace->getNamespaceKind() != NamespaceKind_Type)
	{
		err::setFormatStringError("invalid setas in '%s'", nspace->getQualifiedName().sz());
		return false;
	}

	DerivableType* derivableType = (DerivableType*)(NamedType*)nspace;
	if (derivableType->m_setAsType)
	{
		err::setFormatStringError("setas redefinition for '%s'", derivableType->getTypeString().sz());
		return false;
	}

	derivableType->m_setAsType = type;

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&derivableType->m_setAsType);

	return true;
}

bool
Parser::createAttributeBlock(const lex::LineCol& pos)
{
	ASSERT(!m_attributeBlock);

	m_attributeBlock = m_module->m_attributeMgr.createAttributeBlock();
	m_attributeBlock->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	m_attributeBlock->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	m_attributeBlock->m_pos = pos;
	return true;
}

bool
Parser::createAttribute(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	sl::BoxList<Token>* initializer
	)
{
	ASSERT(m_attributeBlock);

	Attribute* attribute = m_attributeBlock->createAttribute(name, initializer);
	if (!attribute)
		return false;

	attribute->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	attribute->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	attribute->m_pos = pos;
	return true;
}

void
Parser::preDeclaration()
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_lastDeclaredItem = NULL;
}

bool
Parser::bodylessDeclaration()
{
	if (m_mode == Mode_Reaction)
		return true; // reactor declarations are already processed at this stage

	ASSERT(m_lastDeclaredItem);

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Property:
		return finalizeLastProperty(false);

	case ModuleItemKind_Orphan:
		err::setFormatStringError(
			"orphan '%s' without a body",
			m_lastDeclaredItem->getDecl()->getQualifiedName().sz()
			);
		return false;
	}

	return true;
}

bool
Parser::setDeclarationBody(
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	if (!m_lastDeclaredItem)
	{
		err::setFormatStringError("declaration without declarator cannot have a body");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	Function* function;
	Orphan* orphan;
	Type* type;

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind();
	switch (itemKind)
	{
	case ModuleItemKind_Function:
		if (nspace->getNamespaceKind() == NamespaceKind_DynamicLib)
		{
			err::setFormatStringError("dynamiclib function cannot have a body");
			return false;
		}

		function = (Function*)m_lastDeclaredItem;
		function->addUsingSet(nspace);
		return function->setBody(pos, body);

	case ModuleItemKind_Property:
		return parseLastPropertyBody(pos, body);

	case ModuleItemKind_Typedef:
		type = ((Typedef*)m_lastDeclaredItem)->getType();
		break;

	case ModuleItemKind_Type:
		type = (Type*)m_lastDeclaredItem;
		break;

	case ModuleItemKind_Variable:
		type = ((Variable*)m_lastDeclaredItem)->getType();
		break;

	case ModuleItemKind_Field:
		type = ((Field*)m_lastDeclaredItem)->getType();
		break;

	case ModuleItemKind_Orphan:
		orphan = (Orphan*)m_lastDeclaredItem;
		orphan->addUsingSet(nspace);
		return orphan->setBody(pos, body);

	default:
		err::setFormatStringError("'%s' cannot have a body", getModuleItemKindString(m_lastDeclaredItem->getItemKind ()));
		return false;
	}

	if (!isClassType(type, ClassTypeKind_Reactor))
	{
		err::setFormatStringError("only functions and reactors can have bodies, not '%s'", type->getTypeString().sz());
		return false;
	}

	return ((ReactorClassType*)type)->setBody(pos, body);
}

bool
Parser::setStorageKind(StorageKind storageKind)
{
	if (m_storageKind)
	{
		err::setFormatStringError(
			"more than one storage specifier specifiers ('%s' and '%s')",
			getStorageKindString(m_storageKind),
			getStorageKindString(storageKind)
			);
		return false;
	}

	m_storageKind = storageKind;
	return true;
}

bool
Parser::setAccessKind(AccessKind accessKind)
{
	if (m_accessKind)
	{
		err::setFormatStringError(
			"more than one access specifiers ('%s' and '%s')",
			getAccessKindString(m_accessKind),
			getAccessKindString(accessKind)
			);
		return false;
	}

	m_accessKind = accessKind;
	return true;
}

void
Parser::postDeclaratorName(Declarator* declarator)
{
	if (!m_topDeclarator)
		m_topDeclarator = declarator;

	if (!m_topDeclarator->isQualified() || declarator->m_baseType->getTypeKind() != TypeKind_NamedImport)
		return;

	// need to re-anchor import

	QualifiedName anchorName = m_topDeclarator->m_name;
	if (m_topDeclarator->getDeclaratorKind() == DeclaratorKind_Name)
		anchorName.removeLastName();

	ASSERT(!anchorName.isEmpty());

	NamedImportType* importType = m_module->m_typeMgr.getNamedImportType(
		((NamedImportType*)declarator->m_baseType)->m_name,
		m_module->m_namespaceMgr.getCurrentNamespace(),
		anchorName
		);

	importType->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	importType->m_pos = declarator->getPos();

	declarator->m_baseType = importType;
}

void
Parser::postDeclarator(Declarator* declarator)
{
	ASSERT(m_topDeclarator);

	if (declarator == m_topDeclarator)
		m_topDeclarator = NULL;
}

GlobalNamespace*
Parser::getGlobalNamespace(
	GlobalNamespace* parentNamespace,
	const sl::StringRef& name,
	const lex::LineCol& pos
	)
{
	GlobalNamespace* nspace;

	FindModuleItemResult findResult = parentNamespace->findItem(name);
	if (!findResult.m_result)
		return NULL;

	if (!findResult.m_item)
	{
		nspace = m_module->m_namespaceMgr.createGlobalNamespace(name, parentNamespace);
		nspace->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
		nspace->m_pos = pos;
		parentNamespace->addItem(nspace);
	}
	else
	{
		if (findResult.m_item->getItemKind() != ModuleItemKind_Namespace)
		{
			err::setFormatStringError("'%s' exists and is not a namespace", parentNamespace->createQualifiedName(name).sz());
			return NULL;
		}

		nspace = (GlobalNamespace*)findResult.m_item;
	}

	return nspace;
}

GlobalNamespace*
Parser::declareGlobalNamespace(
	const lex::LineCol& pos,
	const QualifiedName& name,
	const lex::LineCol& bodyPos,
	const sl::StringRef& body
	)
{
	Namespace* currentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	if (currentNamespace->getNamespaceKind() != NamespaceKind_Global)
	{
		err::setFormatStringError("cannot open global namespace in '%s'", getNamespaceKindString(currentNamespace->getNamespaceKind ()));
		return NULL;
	}

	GlobalNamespace* nspace = getGlobalNamespace((GlobalNamespace*)currentNamespace, name.getFirstName(), pos);
	if (!nspace)
		return NULL;

	sl::ConstBoxIterator<sl::StringRef> it = name.getNameList().getHead();
	for (; it; it++)
	{
		nspace = getGlobalNamespace(nspace, *it, pos);
		if (!nspace)
			return NULL;
	}

	nspace->addBody(m_module->m_unitMgr.getCurrentUnit(), bodyPos, body);
	return nspace;
}

ExtensionNamespace*
Parser::declareExtensionNamespace(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	Type* type,
	const lex::LineCol& bodyPos,
	const sl::StringRef& body
	)
{
	Namespace* currentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	ExtensionNamespace* nspace = m_module->m_namespaceMgr.createGlobalNamespace<ExtensionNamespace>(
		name,
		currentNamespace
		);

	nspace->m_type = (DerivableType*)type; // force-cast

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&nspace->m_type);

	assignDeclarationAttributes(nspace, nspace, pos);

	bool result = currentNamespace->addItem(nspace);
	if (!result)
		return NULL;

	result = nspace->setBody(bodyPos, body);
	ASSERT(result);
	return nspace;
}

bool
Parser::useNamespace(
	const sl::BoxList<QualifiedName>& nameList,
	NamespaceKind namespaceKind,
	const lex::LineCol& pos
	)
{
	bool result;
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	sl::ConstBoxIterator<QualifiedName> it = nameList.getHead();
	for (; it; it++)
	{
		result = nspace->m_usingSet.addNamespace(nspace, namespaceKind, *it);
		if (!result)
			return false;
	}

	return true;
}

AttributeBlock*
Parser::popAttributeBlock()
{
	AttributeBlock* attributeBlock = m_attributeBlock;
	m_attributeBlock = NULL;
	return attributeBlock;
}

bool
Parser::declareInReaction(Declarator* declarator)
{
	ASSERT(m_mode == Mode_Reaction && m_reactorType);

	if (!declarator->isSimple())
	{
		err::setFormatStringError("invalid declarator in reactor");
		return false;
	}

	sl::String name = declarator->getName()->getShortName();
	FindModuleItemResult findResult = m_reactorType->findItem(name);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("member '%s' not found in reactor '%s'", name.sz(), m_reactorType->getQualifiedName().sz());
		return false;
	}

	m_lastDeclaredItem = findResult.m_item;

	if (declarator->m_initializer.isEmpty())
		return true;

	// modify initializer so it looks like an assignment expression: name = <initializer>

	Token token;
	token.m_pos = declarator->m_initializer.getHead()->m_pos;
	token.m_tokenKind = (TokenKind) '=';
	declarator->m_initializer.insertHead(token);

	token.m_tokenKind = TokenKind_Identifier;
	token.m_data.m_string = name;
	declarator->m_initializer.insertHead(token);

	Parser parser(m_module, Mode_Reaction);
	parser.m_reactorType = m_reactorType;
	parser.m_reactionIdx = m_reactionIdx;

	return parser.parseTokenList(SymbolKind_expression, declarator->m_initializer);
}

bool
Parser::declare(Declarator* declarator)
{
	if (m_mode == Mode_Reaction)
		return declareInReaction(declarator);

	m_lastDeclaredItem = NULL;

	bool isLibrary = m_module->m_namespaceMgr.getCurrentNamespace()->getNamespaceKind() == NamespaceKind_DynamicLib;

	if ((declarator->getTypeModifiers() & TypeModifier_Property) && m_storageKind != StorageKind_Typedef)
	{
		if (isLibrary)
		{
			err::setFormatStringError("only functions can be part of library");
			return false;
		}

		// too early to calctype cause maybe this property has a body
		// declare a typeless property for now

		return declareProperty(declarator, NULL, 0);
	}

	uint_t declFlags;
	Type* type = declarator->calcType(&declFlags);
	if (!type)
		return false;

	DeclaratorKind declaratorKind = declarator->getDeclaratorKind();
	uint_t postModifiers = declarator->getPostDeclaratorModifiers();
	TypeKind typeKind = type->getTypeKind();

	if (isLibrary && typeKind != TypeKind_Function)
	{
		err::setFormatStringError("only functions can be part of library");
		return false;
	}

	if (postModifiers != 0 && typeKind != TypeKind_Function)
	{
		err::setFormatStringError("unused post-declarator modifier '%s'", getPostDeclaratorModifierString(postModifiers).sz());
		return false;
	}

	switch (m_storageKind)
	{
	case StorageKind_Typedef:
		return declareTypedef(declarator, type);

	case StorageKind_Alias:
		return declareAlias(declarator, type, declFlags);

	default:
		switch (typeKind)
		{
		case TypeKind_Void:
			err::setFormatStringError("illegal use of type 'void'");
			return false;

		case TypeKind_Function:
			return declareFunction(declarator, (FunctionType*)type);

		case TypeKind_Property:
			return declareProperty(declarator, (PropertyType*)type, declFlags);

		default:
			return type->getStdType() == StdType_ReactorBase ?
				declareReactor(declarator, declFlags) :
				declareData(declarator, type, declFlags);
		}
	}
}

void
Parser::assignDeclarationAttributes(
	ModuleItem* item,
	ModuleItemDecl* decl,
	const lex::LineCol& pos,
	AttributeBlock* attributeBlock,
	dox::Block* doxyBlock
	)
{
	decl->m_accessKind = m_accessKind ?
		m_accessKind :
		m_module->m_namespaceMgr.getCurrentAccessKind();

	// don't overwrite storage unless explicit

	if (m_storageKind)
		decl->m_storageKind = m_storageKind;

	decl->m_pos = pos;
	decl->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	decl->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	decl->m_attributeBlock = attributeBlock ? attributeBlock : popAttributeBlock();

	if (m_module->getCompileFlags() & ModuleCompileFlag_Documentation)
		m_module->m_doxyHost.setItemBlock(item, decl, doxyBlock ? doxyBlock : m_doxyParser.popBlock());

	item->m_flags |= ModuleItemFlag_User;
	m_lastDeclaredItem = item;
}

bool
Parser::declareTypedef(
	Declarator* declarator,
	Type* type
	)
{
	ASSERT(m_storageKind == StorageKind_Typedef);

	if (!declarator->isSimple())
	{
		err::setFormatStringError("invalid typedef declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	sl::String name = declarator->getName()->getShortName();
	FindModuleItemResult findResult = nspace->findItem(name);

	if (!findResult.m_result)
		return false;

	if (findResult.m_item)
	{
		ModuleItem* prevItem = findResult.m_item;

		if (prevItem->getItemKind() != ModuleItemKind_Typedef ||
			((Typedef*)prevItem)->getType()->cmp(type) != 0)
		{
			setRedefinitionError(name);
			return false;
		}

		m_attributeBlock = NULL;
		m_lastDeclaredItem = prevItem;
		m_doxyParser.popBlock();
		return true;
	}

	sl::String qualifiedName = nspace->createQualifiedName(name);

	ModuleItem* item;
	ModuleItemDecl* decl;

	Typedef* tdef = m_module->m_typeMgr.createTypedef(name, qualifiedName, type);
	item = tdef;
	decl = tdef;

	assignDeclarationAttributes(item, decl, declarator);

	return nspace->addItem(name, item);
}

bool
Parser::declareAlias(
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (!declarator->m_constructor.isEmpty())
	{
		err::setFormatStringError("alias cannot have constructor");
		return false;
	}

	if (declarator->m_initializer.isEmpty())
	{
		err::setFormatStringError("missing alias initializer");
		return false;
	}

	if (!declarator->isSimple())
	{
		err::setFormatStringError("invalid alias declarator");
		return false;
	}

	if (type->getTypeKind() != TypeKind_Void)
	{
		err::setFormatStringError("alias doesn't need a type");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	sl::String name = declarator->getName()->getShortName();
	sl::String qualifiedName = nspace->createQualifiedName(name);
	sl::BoxList<Token>* initializer = &declarator->m_initializer;

	Alias* alias = m_module->m_namespaceMgr.createAlias(name, qualifiedName, initializer);
	assignDeclarationAttributes(alias, alias, declarator);

	if (nspace->getNamespaceKind() == NamespaceKind_Property)
	{
		Property* prop = (Property*)nspace;

		if (ptrTypeFlags & PtrTypeFlag_Bindable)
		{
			result = prop->setOnChanged(alias);
			if (!result)
				return false;
		}
		else if (ptrTypeFlags & PtrTypeFlag_AutoGet)
		{
			result = prop->setAutoGetValue(alias);
			if (!result)
				return false;
		}
	}

	return nspace->addItem(alias);
}

bool
Parser::declareFunction(
	Declarator* declarator,
	FunctionType* type
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	DeclaratorKind declaratorKind = declarator->getDeclaratorKind();
	uint_t postModifiers = declarator->getPostDeclaratorModifiers();
	FunctionKind functionKind = declarator->getFunctionKind();
	bool hasArgs = !type->getArgArray().isEmpty();

	if (declaratorKind == DeclaratorKind_UnaryBinaryOperator)
	{
		ASSERT(functionKind == FunctionKind_UnaryOperator || functionKind == FunctionKind_BinaryOperator);
		functionKind = hasArgs ? FunctionKind_BinaryOperator : FunctionKind_UnaryOperator;
	}

	ASSERT(functionKind);
	uint_t functionKindFlags = getFunctionKindFlags(functionKind);

	if ((functionKindFlags & FunctionKindFlag_NoStorage) && m_storageKind)
	{
		err::setFormatStringError("'%s' cannot have storage specifier", getFunctionKindString(functionKind));
		return false;
	}

	if ((functionKindFlags & FunctionKindFlag_NoArgs) && hasArgs)
	{
		err::setFormatStringError("'%s' cannot have arguments", getFunctionKindString(functionKind));
		return false;
	}

	if (!m_storageKind)
	{
		m_storageKind =
			functionKind == FunctionKind_StaticConstructor ? StorageKind_Static :
			namespaceKind == NamespaceKind_Property ? ((Property*)nspace)->getStorageKind() : StorageKind_Undefined;
	}

	if (namespaceKind == NamespaceKind_PropertyTemplate)
	{
		if (m_storageKind)
		{
			err::setFormatStringError("invalid storage '%s' in property template", getStorageKindString(m_storageKind));
			return false;
		}

		if (postModifiers)
		{
			err::setFormatStringError("unused post-declarator modifier '%s'", getPostDeclaratorModifierString(postModifiers).sz());
			return false;
		}

		bool result = ((PropertyTemplate*)nspace)->addMethod(functionKind, type);
		if (!result)
			return false;

		m_lastDeclaredItem = type;
		return true;
	}

	ModuleItem* functionItem;
	ModuleItemDecl* functionItemDecl;
	FunctionName* functionName;

	if (declarator->isQualified())
	{
		Orphan* orphan = m_module->m_namespaceMgr.createOrphan(OrphanKind_Function, type);
		orphan->m_functionKind = functionKind;
		orphan->m_declaratorName = *declarator->getName();

		functionItem = orphan;
		functionItemDecl = orphan;
		functionName = orphan;
		nspace->addOrphan(orphan);
	}
	else
	{
		Function* function;

		if (type->getFlags() & FunctionTypeFlag_Async)
		{
			function = m_module->m_functionMgr.createFunction<AsyncLauncherFunction>(
				sl::StringRef(),
				sl::StringRef(),
				type
				);
		}
		else
		{
			function = m_module->m_functionMgr.createFunction(type);
			function->m_functionKind = functionKind;
		}

		functionItem = function;
		functionItemDecl = function;
		functionName = function;

		if (!declarator->m_initializer.isEmpty())
			sl::takeOver(&function->m_initializer, &declarator->m_initializer);
	}

	assignDeclarationAttributes(functionItem, functionItemDecl, declarator);

	if (postModifiers & PostDeclaratorModifier_Const)
		functionName->m_thisArgTypeFlags = PtrTypeFlag_Const;

	switch (functionKind)
	{
	case FunctionKind_Normal:
		functionItemDecl->m_name = declarator->getName()->getShortName();
		functionItemDecl->m_qualifiedName = nspace->createQualifiedName(functionItemDecl->m_name);
		break;

	case FunctionKind_UnaryOperator:
		functionName->m_unOpKind = declarator->getUnOpKind();
		functionItemDecl->m_qualifiedName.format(
			"%s.unary operator %s",
			nspace->getQualifiedName().sz(),
			getUnOpKindString(functionName->m_unOpKind)
			);
		break;

	case FunctionKind_BinaryOperator:
		functionName->m_binOpKind = declarator->getBinOpKind();
		functionItemDecl->m_qualifiedName.format(
			"%s.binary operator %s",
			nspace->getQualifiedName().sz(),
			getBinOpKindString(functionName->m_binOpKind)
			);
		break;

	case FunctionKind_CastOperator:
		functionName->m_castOpType = declarator->getCastOpType();
		functionItemDecl->m_qualifiedName.format(
			"%s.cast operator %s",
			nspace->getQualifiedName().sz(),
			functionName->m_castOpType->getTypeString().sz()
			);
		break;

	default:
		functionItemDecl->m_qualifiedName.format(
			"%s.%s",
			nspace->getQualifiedName().sz(),
			getFunctionKindString(functionKind)
			);
	}

	if (functionItem->getItemKind() == ModuleItemKind_Orphan)
	{
		if (namespaceKind == NamespaceKind_DynamicLib)
		{
			err::setFormatStringError("illegal orphan in dynamiclib '%s'", nspace->getQualifiedName().sz());
			return false;
		}

		return true;
	}

	ASSERT(functionItem->getItemKind() == ModuleItemKind_Function);
	Function* function = (Function*)functionItem;
	TypeKind typeKind;

	switch (namespaceKind)
	{
	case NamespaceKind_Extension:
		return ((ExtensionNamespace*)nspace)->addMethod(function);

	case NamespaceKind_Type:
		typeKind = ((NamedType*)nspace)->getTypeKind();
		switch (typeKind)
		{
		case TypeKind_Struct:
			return ((StructType*)nspace)->addMethod(function);

		case TypeKind_Union:
			return ((UnionType*)nspace)->addMethod(function);

		case TypeKind_Class:
			return ((ClassType*)nspace)->addMethod(function);

		default:
			err::setFormatStringError("method members are not allowed in '%s'", ((NamedType*)nspace)->getTypeString().sz());
			return false;
		}

	case NamespaceKind_Property:
		return ((Property*)nspace)->addMethod(function);

	case NamespaceKind_DynamicLib:
		function->m_libraryTableIndex = ((DynamicLibNamespace*)nspace)->m_functionCount++;

		// and fall through

	default:
		if (postModifiers)
		{
			err::setFormatStringError("unused post-declarator modifier '%s'", getPostDeclaratorModifierString(postModifiers).sz());
			return false;
		}

		if (!m_storageKind)
		{
			function->m_storageKind = StorageKind_Static;
		}
		else if (m_storageKind != StorageKind_Static)
		{
			err::setFormatStringError("invalid storage specifier '%s' for a global function", getStorageKindString(m_storageKind));
			return false;
		}
	}

	if (!nspace->getParentNamespace()) // module constructor / destructor
		switch (functionKind)
		{
		case FunctionKind_Constructor:
		case FunctionKind_StaticConstructor:
			return m_module->m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_Constructor, function);

		case FunctionKind_Destructor:
			return m_module->m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_Destructor, function);
		}

	if (functionKind != FunctionKind_Normal)
	{
		err::setFormatStringError(
			"invalid '%s' at '%s' namespace",
			getFunctionKindString(functionKind),
			getNamespaceKindString(namespaceKind)
			);
		return false;
	}

	return nspace->addFunction(function) != -1;
}

bool
Parser::declareProperty(
	Declarator* declarator,
	PropertyType* type,
	uint_t flags
	)
{
	if (!declarator->isSimple())
	{
		err::setFormatStringError("invalid property declarator");
		return false;
	}

	Property* prop = createProperty(declarator);
	if (!prop)
		return false;

	if (type)
	{
		prop->m_flags |= flags;
		return prop->create(type);
	}

	m_lastPropertyTypeModifiers = declarator->getTypeModifiers();
	if (m_lastPropertyTypeModifiers & TypeModifier_Const)
		prop->m_flags |= PropertyFlag_Const;

	if (declarator->getBaseType()->getTypeKind() != TypeKind_Void ||
		!declarator->getPointerPrefixList().isEmpty() ||
		!declarator->getSuffixList().isEmpty())
	{
		DeclTypeCalc typeCalc;
		m_lastPropertyGetterType = typeCalc.calcPropertyGetterType(declarator);
		if (!m_lastPropertyGetterType)
			return false;
	}
	else
	{
		m_lastPropertyGetterType = NULL;
	}

	return true;
}

PropertyTemplate*
Parser::createPropertyTemplate()
{
	PropertyTemplate* propertyTemplate = m_module->m_functionMgr.createPropertyTemplate();
	uint_t modifiers = getTypeSpecifier()->clearTypeModifiers(TypeModifier_Property | TypeModifier_Bindable);

	if (modifiers & TypeModifier_Bindable)
		propertyTemplate->m_typeFlags = PropertyTypeFlag_Bindable;

	return propertyTemplate;
}

Property*
Parser::createProperty(Declarator* declarator)
{
	bool result;

	m_lastDeclaredItem = NULL;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	if (namespaceKind == NamespaceKind_PropertyTemplate)
	{
		err::setFormatStringError("property templates cannot have property members");
		return NULL;
	}

	const sl::String& name = declarator->getName()->getShortName();
	sl::String qualifiedName = nspace->createQualifiedName(name);
	Property* prop = m_module->m_functionMgr.createProperty(name, qualifiedName);

	assignDeclarationAttributes(prop, prop, declarator);

	TypeKind typeKind;
	switch (namespaceKind)
	{
	case NamespaceKind_Extension:
		result = ((ExtensionNamespace*)nspace)->addProperty(prop);
		if (!result)
			return NULL;

		break;

	case NamespaceKind_Type:
		typeKind = ((NamedType*)nspace)->getTypeKind();
		switch (typeKind)
		{
		case TypeKind_Struct:
			result = ((StructType*)nspace)->addProperty(prop);
			break;

		case TypeKind_Union:
			result = ((UnionType*)nspace)->addProperty(prop);
			break;

		case TypeKind_Class:
			result = ((ClassType*)nspace)->addProperty(prop);
			break;

		default:
			err::setFormatStringError("property members are not allowed in '%s'", ((NamedType*) nspace)->getTypeString().sz());
			return NULL;
		}

		if (!result)
			return NULL;

		break;

	case NamespaceKind_Property:
		result = ((Property*)nspace)->addProperty(prop);
		if (!result)
			return NULL;

		break;

	default:
		if (m_storageKind && m_storageKind != StorageKind_Static)
		{
			err::setFormatStringError("invalid storage specifier '%s' for a global property", getStorageKindString(m_storageKind));
			return NULL;
		}

		result = nspace->addItem(prop);
		if (!result)
			return NULL;

		prop->m_storageKind = StorageKind_Static;
	}

	return prop;
}

bool
Parser::parseLastPropertyBody(
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	size_t length = body.getLength();
	sl::BoxList<Token> tokenList;

	return
		tokenizeBody(
			&tokenList,
			lex::LineCol(pos.m_line, pos.m_col + 1),
			body.getSubString(1, length - 2)
			) &&
		parseLastPropertyBody(tokenList);
}

bool
Parser::parseLastPropertyBody(const sl::ConstBoxList<Token>& body)
{
	ASSERT(m_lastDeclaredItem && m_lastDeclaredItem->getItemKind() == ModuleItemKind_Property);

	bool result;

	Property* prop = (Property*)m_lastDeclaredItem;

	Parser parser(m_module, Mode_Parse);

	m_module->m_namespaceMgr.openNamespace(prop);

	result = parser.parseTokenList(SymbolKind_member_block_declaration_list, body);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	return finalizeLastProperty(true);
}

bool
Parser::finalizeLastProperty(bool hasBody)
{
	ASSERT(m_lastDeclaredItem && m_lastDeclaredItem->getItemKind() == ModuleItemKind_Property);

	bool result;

	Property* prop = (Property*)m_lastDeclaredItem;
	if (prop->getType())
		return true;

	// finalize getter

	if (prop->m_getter)
	{
		if (m_lastPropertyGetterType && m_lastPropertyGetterType->cmp(prop->m_getter->getType()) != 0)
		{
			err::setFormatStringError("getter type '%s' does not match property declaration", prop->m_getter->getType ()->getTypeString().sz());
			return false;
		}
	}
	else if (prop->m_autoGetValue)
	{
		ASSERT(prop->m_autoGetValue->getItemKind() == ModuleItemKind_Alias); // otherwise, getter would have been created
	}
	else
	{
		if (!m_lastPropertyGetterType)
		{
			err::setFormatStringError("incomplete property: no 'get' method or 'autoget' field");
			return false;
		}

		Function* getter = (m_lastPropertyTypeModifiers & TypeModifier_AutoGet) ?
			m_module->m_functionMgr.createFunction<Property::AutoGetter>(m_lastPropertyGetterType) :
			m_module->m_functionMgr.createFunction(m_lastPropertyGetterType);

		getter->m_functionKind = FunctionKind_Getter;
		getter->m_flags |= ModuleItemFlag_User;

		result = prop->addMethod(getter);
		if (!result)
			return false;
	}

	// finalize setter

	if (!(m_lastPropertyTypeModifiers & TypeModifier_Const) && !hasBody)
	{
		FunctionType* getterType = prop->m_getter->getType()->getShortType();
		sl::Array<FunctionArg*> argArray = getterType->getArgArray();

		Type* setterArgType = getterType->getReturnType();
		if (setterArgType->getTypeKindFlags() & TypeKindFlag_Derivable)
		{
			Type* setAsType = ((DerivableType*)setterArgType)->getSetAsType();
			if (setAsType)
				setterArgType = setAsType;
		}

		argArray.append(setterArgType->getSimpleFunctionArg());

		FunctionType* setterType = m_module->m_typeMgr.getFunctionType(argArray);
		Function* setter = m_module->m_functionMgr.createFunction(setterType);
		setter->m_functionKind = FunctionKind_Setter;
		setter->m_flags |= ModuleItemFlag_User;

		result = prop->addMethod(setter);
		if (!result)
			return false;
	}

	// finalize binder

	if (m_lastPropertyTypeModifiers & TypeModifier_Bindable)
	{
		if (!prop->m_onChanged)
		{
			result = prop->createOnChanged();
			if (!result)
				return false;
		}
	}

	// finalize auto-get value

	if (m_lastPropertyTypeModifiers & TypeModifier_AutoGet)
	{
		if (!prop->m_autoGetValue)
		{
			result = prop->createAutoGetValue(prop->m_getter->getType()->getReturnType());
			if (!result)
				return false;
		}
	}

	if (prop->m_getter)
		prop->createType();

	return true;
}

bool
Parser::declareReactor(
	Declarator* declarator,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (declarator->getDeclaratorKind() != DeclaratorKind_Name)
	{
		err::setFormatStringError("invalid reactor declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	NamedType* parentType = NULL;

	switch (namespaceKind)
	{
	case NamespaceKind_Property:
		parentType = ((Property*)nspace)->getParentType();
		break;

	case NamespaceKind_Type:
		parentType = (NamedType*)nspace;
		break;
	}

	if (parentType && parentType->getTypeKind() != TypeKind_Class)
	{
		err::setFormatStringError("'%s' cannot contain reactor members", parentType->getTypeString().sz());
		return false;
	}

	sl::String name = declarator->getName()->getShortName();
	sl::String qualifiedName = nspace->createQualifiedName(name);

	if (declarator->isQualified())
	{
		Orphan* orphan = m_module->m_namespaceMgr.createOrphan(OrphanKind_Reactor, NULL);
		orphan->m_functionKind = FunctionKind_Normal;
		orphan->m_declaratorName = *declarator->getName();
		assignDeclarationAttributes(orphan, orphan, declarator);
		nspace->addOrphan(orphan);
	}
	else
	{
		ReactorClassType* type = m_module->m_typeMgr.createReactorType(name, qualifiedName, (ClassType*)parentType);
		assignDeclarationAttributes(type, type, declarator);
		result = declareData(declarator, type, ptrTypeFlags);
		if (!result)
			return false;
	}

	return true;
}

bool
Parser::declareData(
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (!declarator->isSimple())
	{
		err::setFormatStringError("invalid data declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	switch (namespaceKind)
	{
	case NamespaceKind_PropertyTemplate:
	case NamespaceKind_Extension:
		err::setFormatStringError("'%s' cannot have data fields", getNamespaceKindString(namespaceKind));
		return false;
	}

	sl::String name = declarator->getName()->getShortName();
	size_t bitCount = declarator->getBitCount();
	sl::BoxList<Token>* constructor = &declarator->m_constructor;
	sl::BoxList<Token>* initializer = &declarator->m_initializer;

	if (isAutoSizeArrayType(type))
	{
		if (initializer->isEmpty())
		{
			err::setFormatStringError("auto-size array '%s' should have initializer", type->getTypeString().sz());
			return false;
		}

		ArrayType* arrayType = (ArrayType*)type;
		arrayType->m_elementCount = m_module->m_operatorMgr.parseAutoSizeArrayInitializer(arrayType, *initializer);
		if (arrayType->m_elementCount == -1)
			return false;

		if (m_mode == Mode_Compile)
		{
			result = arrayType->ensureLayout();
			if (!result)
				return false;
		}
	}

	bool isDisposable = false;

	if (namespaceKind != NamespaceKind_Property && (ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable)))
	{
		err::setFormatStringError("'%s' can only be used on property field", getPtrTypeFlagString(ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable)).sz());
		return false;
	}

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	StorageKind storageKind = m_storageKind;
	switch (storageKind)
	{
	case StorageKind_Undefined:
		switch (namespaceKind)
		{
		case NamespaceKind_Scope:
			storageKind = (type->getFlags() & TypeFlag_NoStack) ? StorageKind_Heap : StorageKind_Stack;
			break;

		case NamespaceKind_Type:
			storageKind = StorageKind_Member;
			break;

		case NamespaceKind_Property:
			storageKind = ((Property*)nspace)->getParentType() ? StorageKind_Member : StorageKind_Static;
			break;

		default:
			storageKind = StorageKind_Static;
		}

		break;

	case StorageKind_Static:
		break;

	case StorageKind_Tls:
		if (!scope && (!constructor->isEmpty() || !initializer->isEmpty()))
		{
			err::setFormatStringError("global 'threadlocal' variables cannot have initializers");
			return false;
		}

		break;

	case StorageKind_Mutable:
		switch (namespaceKind)
		{
		case NamespaceKind_Type:
			break;

		case NamespaceKind_Property:
			if (((Property*)nspace)->getParentType())
				break;

		default:
			err::setFormatStringError("'mutable' can only be applied to member fields");
			return false;
		}

		break;

	case StorageKind_Disposable:
		if (namespaceKind != NamespaceKind_Scope)
		{
			err::setFormatStringError("'disposable' can only be applied to local variables");
			return false;
		}

		if (!isDisposableType(type))
		{
			err::setFormatStringError("'%s' is not a disposable type", type->getTypeString().sz());
			return false;
		}

		ASSERT(scope);
		if (!(scope->getFlags() & ScopeFlag_Disposable))
			scope = m_module->m_namespaceMgr.openScope(
				declarator->getPos(),
				ScopeFlag_Disposable | ScopeFlag_FinallyAhead | ScopeFlag_Finalizable | ScopeFlag_Nested
				);

		storageKind = (type->getFlags() & TypeFlag_NoStack) ? StorageKind_Heap : StorageKind_Stack;
		m_storageKind = StorageKind_Undefined; // don't overwrite
		isDisposable = true;
		break;

	default:
		err::setFormatStringError("invalid storage specifier '%s' for variable", getStorageKindString(storageKind));
		return false;
	}

	if (namespaceKind == NamespaceKind_Property)
	{
		Property* prop = (Property*)nspace;
		ModuleItem* dataItem = NULL;

		if (storageKind == StorageKind_Member)
		{
			Field* field = prop->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
			if (!field)
				return false;

			assignDeclarationAttributes(field, field, declarator);

			dataItem = field;
		}
		else
		{
			Variable* variable = m_module->m_variableMgr.createVariable(
				storageKind,
				name,
				nspace->createQualifiedName(name),
				type,
				ptrTypeFlags,
				constructor,
				initializer
				);

			assignDeclarationAttributes(variable, variable, declarator);

			result = nspace->addItem(variable);
			if (!result)
				return false;

			prop->m_staticVariableArray.append(variable);
			dataItem = variable;
		}

		if (ptrTypeFlags & PtrTypeFlag_Bindable)
		{
			result = prop->setOnChanged(dataItem);
			if (!result)
				return false;
		}
		else if (ptrTypeFlags & PtrTypeFlag_AutoGet)
		{
			result = prop->setAutoGetValue(dataItem);
			if (!result)
				return false;
		}

	}
	else if (storageKind != StorageKind_Member && storageKind != StorageKind_Mutable)
	{
		Variable* variable = m_module->m_variableMgr.createVariable(
			storageKind,
			name,
			nspace->createQualifiedName(name),
			type,
			ptrTypeFlags,
			constructor,
			initializer
			);

		assignDeclarationAttributes(variable, variable, declarator);

		result = nspace->addItem(variable);
		if (!result)
			return false;

		if (nspace->getNamespaceKind() == NamespaceKind_Type)
		{
			NamedType* namedType = (NamedType*)nspace;
			TypeKind namedTypeKind = namedType->getTypeKind();

			switch (namedTypeKind)
			{
			case TypeKind_Class:
			case TypeKind_Struct:
			case TypeKind_Union:
				((DerivableType*)namedType)->m_staticVariableArray.append(variable);
				break;

			default:
				err::setFormatStringError("field members are not allowed in '%s'", namedType->getTypeString().sz());
				return false;
			}
		}
		else if (scope)
		{
			result = m_module->m_variableMgr.allocateVariable(variable);
			if (!result)
				return false;

			if (isDisposable)
			{
				result = m_module->m_variableMgr.finalizeDisposableVariable(variable);
				if (!result)
					return false;
			}

			switch (storageKind)
			{
			case StorageKind_Stack:
			case StorageKind_Heap:
				result = m_module->m_variableMgr.initializeVariable(variable);
				if (!result)
					return false;

				break;

			case StorageKind_Static:
			case StorageKind_Tls:
				if (variable->m_initializer.isEmpty() &&
					variable->m_type->getTypeKind() != TypeKind_Class &&
					!isConstructibleType(variable->m_type))
					break;

				OnceStmt stmt;
				m_module->m_controlFlowMgr.onceStmt_Create(&stmt, variable->m_pos, storageKind);

				result = m_module->m_controlFlowMgr.onceStmt_PreBody(&stmt, variable->m_pos);
				if (!result)
					return false;

				result = m_module->m_variableMgr.initializeVariable(variable);
				if (!result)
					return false;

				m_module->m_controlFlowMgr.onceStmt_PostBody(
					&stmt,
					!variable->m_initializer.isEmpty() ? variable->m_initializer.getTail()->m_pos :
					!variable->m_constructor.isEmpty() ? variable->m_constructor.getTail()->m_pos :
					variable->m_pos
					);

				break;

			default:
				ASSERT(false);
			}
		}
	}
	else
	{
		ASSERT(nspace->getNamespaceKind() == NamespaceKind_Type);

		NamedType* namedType = (NamedType*)nspace;
		TypeKind namedTypeKind = namedType->getTypeKind();

		Field* field;

		switch (namedTypeKind)
		{
		case TypeKind_Class:
			field = ((ClassType*)namedType)->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		case TypeKind_Struct:
			field = ((StructType*)namedType)->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		case TypeKind_Union:
			field = ((UnionType*)namedType)->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		default:
			err::setFormatStringError("field members are not allowed in '%s'", namedType->getTypeString().sz());
			return false;
		}

		if (!field)
			return false;

		assignDeclarationAttributes(field, field, declarator);
	}

	return true;
}

bool
Parser::declareUnnamedStructOrUnion(DerivableType* type)
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;

	Declarator declarator;
	declarator.m_declaratorKind = DeclaratorKind_Name;
	declarator.m_pos = type->getPos();
	return declareData(&declarator, type, 0);
}

FunctionArg*
Parser::createFormalArg(
	DeclFunctionSuffix* argSuffix,
	Declarator* declarator
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	uint_t ptrTypeFlags = 0;
	Type* type = declarator->calcType(&ptrTypeFlags);
	if (!type)
		return NULL;

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_Void:
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError(
			"function cannot accept '%s' as an argument",
			type->getTypeString().sz()
			);
		return NULL;
	}

	if (m_storageKind)
	{
		err::setFormatStringError("invalid storage '%s' for argument", getStorageKindString(m_storageKind));
		return NULL;
	}

	m_storageKind = StorageKind_Stack;

	sl::String name;

	if (declarator->isSimple())
	{
		name = declarator->getName()->getShortName();
	}
	else if (declarator->getDeclaratorKind() != DeclaratorKind_Undefined)
	{
		err::setFormatStringError("invalid formal argument declarator");
		return NULL;
	}

	sl::BoxList<Token>* initializer = &declarator->m_initializer;

	FunctionArg* arg = m_module->m_typeMgr.createFunctionArg(name, type, ptrTypeFlags, initializer);
	assignDeclarationAttributes(arg, arg, declarator);

	argSuffix->m_argArray.append(arg);

	return arg;
}

bool
Parser::addEnumFlag(
	uint_t* flags,
	EnumTypeFlag flag
	)
{
	if (*flags & flag)
	{
		err::setFormatStringError("modifier '%s' used more than once", getEnumTypeFlagString(flag));
		return false;
	}

	*flags |= flag;
	return true;
}

EnumType*
Parser::createEnumType(
	const sl::StringRef& name,
	Type* baseType,
	uint_t flags
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	EnumType* enumType = NULL;

	if (name.isEmpty())
	{
		flags |= EnumTypeFlag_Exposed;
		enumType = m_module->m_typeMgr.createUnnamedEnumType(baseType, flags);
	}
	else
	{
		sl::String qualifiedName = nspace->createQualifiedName(name);
		enumType = m_module->m_typeMgr.createEnumType(name, qualifiedName, baseType, flags);
		if (!enumType)
			return NULL;

		bool result = nspace->addItem(enumType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(enumType, enumType, m_lastMatchedToken.m_pos);
	return enumType;
}

EnumConst*
Parser::createEnumConst(
	const sl::StringRef& name,
	const lex::LineCol& pos,
	sl::BoxList<Token>* initializer
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	ASSERT(nspace->getNamespaceKind() == NamespaceKind_Type);
	ASSERT(((NamedType*)nspace)->getTypeKind() == TypeKind_Enum);

	EnumType* type = (EnumType*)m_module->m_namespaceMgr.getCurrentNamespace();
	EnumConst* enumConst = type->createConst(name, initializer);
	if (!enumConst)
		return NULL;

	assignDeclarationAttributes(enumConst, enumConst, pos);
	return enumConst;
}

StructType*
Parser::createStructType(
	const sl::StringRef& name,
	sl::BoxList<Type*>* baseTypeList,
	size_t fieldAlignment,
	uint_t flags
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	StructType* structType = NULL;

	if (name.isEmpty())
	{
		structType = m_module->m_typeMgr.createUnnamedStructType(fieldAlignment, flags);
	}
	else
	{
		sl::String qualifiedName = nspace->createQualifiedName(name);
		structType = m_module->m_typeMgr.createStructType(name, qualifiedName, fieldAlignment, flags);
		if (!structType)
			return NULL;
	}

	if (baseTypeList)
	{
		sl::BoxIterator<Type*> baseType = baseTypeList->getHead();
		for (; baseType; baseType++)
		{
			result = structType->addBaseType(*baseType) != NULL;
			if (!result)
				return NULL;
		}
	}

	if (!name.isEmpty())
	{
		result = nspace->addItem(structType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(structType, structType, m_lastMatchedToken.m_pos);
	return structType;
}

UnionType*
Parser::createUnionType(
	const sl::StringRef& name,
	size_t fieldAlignment,
	uint_t flags
	)
{
	bool result;

	if (flags & TypeFlag_Dynamic)
	{
		err::setError("dynamic unions are not supported yet");
		return NULL;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	UnionType* unionType = NULL;

	if (name.isEmpty())
	{
		unionType = m_module->m_typeMgr.createUnnamedUnionType(fieldAlignment, flags);
	}
	else
	{
		sl::String qualifiedName = nspace->createQualifiedName(name);
		unionType = m_module->m_typeMgr.createUnionType(name, qualifiedName, fieldAlignment, flags);
		if (!unionType)
			return NULL;

		result = nspace->addItem(unionType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(unionType, unionType, m_lastMatchedToken.m_pos);
	return unionType;
}

ClassType*
Parser::createClassType(
	const sl::StringRef& name,
	sl::BoxList<Type*>* baseTypeList,
	size_t fieldAlignment,
	uint_t flags
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	ClassType* classType;

	if (name.isEmpty())
	{
		classType = m_module->m_typeMgr.createUnnamedClassType(fieldAlignment, flags);
	}
	else
	{
		sl::String qualifiedName = nspace->createQualifiedName(name);
		classType = m_module->m_typeMgr.createClassType(name, qualifiedName, fieldAlignment, flags);
	}

	if (baseTypeList)
	{
		sl::BoxIterator<Type*> baseType = baseTypeList->getHead();
		for (; baseType; baseType++)
		{
			result = classType->addBaseType(*baseType) != NULL;
			if (!result)
				return NULL;
		}
	}

	if (!name.isEmpty())
	{
		result = nspace->addItem(classType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(classType, classType, m_lastMatchedToken.m_pos);
	return classType;
}

DynamicLibClassType*
Parser::createDynamicLibType(const sl::StringRef& name)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	sl::String qualifiedName = nspace->createQualifiedName(name);
	DynamicLibClassType* classType = m_module->m_typeMgr.createClassType<DynamicLibClassType>(name, qualifiedName);

	Type* baseType = m_module->m_typeMgr.getStdType(StdType_DynamicLib);

	result =
		classType->addBaseType(baseType) != NULL &&
		nspace->addItem(classType);

	if (!result)
		return NULL;

	assignDeclarationAttributes(classType, classType, m_lastMatchedToken.m_pos);

	DynamicLibNamespace* dynamicLibNamespace = classType->createLibNamespace();
	dynamicLibNamespace->m_parentUnit = classType->getParentUnit();
	return classType;
}

bool
Parser::finalizeDynamicLibType()
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	ASSERT(nspace->getNamespaceKind() == NamespaceKind_DynamicLib);

	DynamicLibNamespace* dynamicLibNamespace = (DynamicLibNamespace*)nspace;
	DynamicLibClassType* dynamicLibType = dynamicLibNamespace->getLibraryType();
	bool result = dynamicLibType->ensureFunctionTable();
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	return true;
}

bool
Parser::addReactionBinding(const Value& value)
{
	ASSERT(m_mode == Mode_Reaction && m_reactorType);

	Function* addBindingFunc = getReactorMethod(m_module, ReactorMethod_AddOnChangedBinding);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	Value onChangedValue;

	return
		m_module->m_operatorMgr.getPropertyOnChanged(value, &onChangedValue) &&
		m_module->m_operatorMgr.callOperator(addBindingFunc, thisValue, onChangedValue);
}

bool
Parser::resetReactionBindings()
{
	Function* resetBindingsFunc = getReactorMethod(m_module, ReactorMethod_ResetOnChangedBindings);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	return m_module->m_operatorMgr.callOperator(resetBindingsFunc, thisValue);
}

bool
Parser::reactorOnEventStmt(
	const sl::ConstBoxList<Value>& valueList,
	Declarator* declarator,
	sl::BoxList<Token>* tokenList
	)
{
	ASSERT(m_mode == Mode_Reaction && m_reactorType);

	DeclFunctionSuffix* suffix = declarator->getFunctionSuffix();
	ASSERT(suffix);

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType(suffix->getArgArray());
	Function* handler = m_reactorType->createOnEventHandler(m_reactionIdx, functionType);
	handler->setTokenList(tokenList);

	Function* addBindingFunc = getReactorMethod(m_module, ReactorMethod_AddOnEventBinding);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	sl::ConstBoxIterator<Value> it = valueList.getHead();
	for (; it; it++)
	{
		bool result = m_module->m_operatorMgr.callOperator(addBindingFunc, thisValue, *it);
		if (!result)
			return false;
	}

	return true;
}

bool
Parser::callBaseTypeMemberConstructor(
	const QualifiedName& name,
	sl::BoxList<Value>* argList
	)
{
	ASSERT(m_constructorType || m_constructorProperty);

	Namespace* nspace = m_module->m_functionMgr.getCurrentFunction()->getParentNamespace();
	FindModuleItemResult findResult = nspace->findItemTraverse(name);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("name '%s' is not found", name.getFullName ().sz());
		return false;
	}

	Type* type = NULL;
	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();

	switch (itemKind)
	{
	case ModuleItemKind_Type:
		return callBaseTypeConstructor((Type*)item, argList);

	case ModuleItemKind_Typedef:
		return callBaseTypeConstructor(((Typedef*)item)->getType(), argList);

	case ModuleItemKind_Property:
		err::setFormatStringError("property construction is not yet implemented");
		return false;

	case ModuleItemKind_Field:
		return callFieldConstructor((Field*)item, argList);

	case ModuleItemKind_Variable:
		err::setFormatStringError("static field construction is not yet implemented");
		return false;

	default:
		err::setFormatStringError("'%s' cannot be used in base-type-member construct list");
		return false;
	}
}

DerivableType*
Parser::findBaseType(size_t baseTypeIdx)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function); // should not be called at pass

	DerivableType* parentType = function->getParentType();
	if (!parentType)
		return NULL;

	BaseTypeSlot* slot = parentType->getBaseTypeByIndex(baseTypeIdx);
	if (!slot)
		return NULL;

	return slot->getType();
}

DerivableType*
Parser::getBaseType(size_t baseTypeIdx)
{
	DerivableType* type = findBaseType(baseTypeIdx);
	if (!type)
	{
		err::setFormatStringError("'basetype%d' is not found", baseTypeIdx + 1);
		return NULL;
	}

	return type;
}

bool
Parser::getBaseType(
	size_t baseTypeIdx,
	Value* resultValue
	)
{
	DerivableType* type = getBaseType(baseTypeIdx);
	if (!type)
		return false;

	resultValue->setNamespace(type);
	return true;
}

bool
Parser::callBaseTypeConstructor(
	size_t baseTypeIdx,
	sl::BoxList<Value>* argList
	)
{
	ASSERT(m_constructorType || m_constructorProperty);

	if (m_constructorProperty)
	{
		err::setFormatStringError("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->getQualifiedName().sz());
		return false;
	}

	BaseTypeSlot* baseTypeSlot = m_constructorType->getBaseTypeByIndex(baseTypeIdx);
	if (!baseTypeSlot)
		return false;

	return callBaseTypeConstructorImpl(baseTypeSlot, argList);
}

bool
Parser::callBaseTypeConstructor(
	Type* type,
	sl::BoxList<Value>* argList
	)
{
	ASSERT(m_constructorType || m_constructorProperty);

	if (m_constructorProperty)
	{
		err::setFormatStringError("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->getQualifiedName().sz());
		return false;
	}

	BaseTypeSlot* baseTypeSlot = m_constructorType->findBaseType(type);
	if (!baseTypeSlot)
	{
		err::setFormatStringError(
			"'%s' is not a base type of '%s'",
			type->getTypeString().sz(),
			m_constructorType->getTypeString().sz()
			);
		return false;
	}

	return callBaseTypeConstructorImpl(baseTypeSlot, argList);
}

bool
Parser::callBaseTypeConstructorImpl(
	BaseTypeSlot* baseTypeSlot,
	sl::BoxList<Value>* argList
	)
{
	DerivableType* type = baseTypeSlot->getType();

	if (baseTypeSlot->m_flags & ModuleItemFlag_Constructed)
	{
		err::setFormatStringError("'%s' is already constructed", type->getTypeString().sz());
		return false;
	}

	OverloadableFunction constructor = type->getConstructor();
	if (!constructor)
	{
		err::setFormatStringError("'%s' has no constructor", type->getTypeString().sz());
		return false;
	}

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	argList->insertHead(thisValue);

	bool result = m_module->m_operatorMgr.callOperator(constructor, argList);
	if (!result)
		return false;

	baseTypeSlot->m_flags |= ModuleItemFlag_Constructed;
	return true;
}

bool
Parser::callFieldConstructor(
	Field* field,
	sl::BoxList<Value>* argList
	)
{
	ASSERT(m_constructorType || m_constructorProperty);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	bool result;

	if (m_constructorProperty)
	{
		err::setFormatStringError("property field construction is not yet implemented");
		return false;
	}

	if (field->getParentNamespace() != m_constructorType)
	{
		err::setFormatStringError(
			"'%s' is not an immediate field of '%s'",
			field->getName().sz(),
			m_constructorType->getTypeString().sz()
			);
		return false;
	}

	if (field->getFlags() & ModuleItemFlag_Constructed)
	{
		err::setFormatStringError("'%s' is already constructed", field->getName().sz());
		return false;
	}

	if (!(field->getType()->getTypeKindFlags() & TypeKindFlag_Derivable) ||
		!((DerivableType*)field->getType())->getConstructor())
	{
		err::setFormatStringError("'%s' has no constructor", field->getName().sz());
		return false;
	}

	OverloadableFunction constructor = ((DerivableType*)field->getType())->getConstructor();

	Value fieldValue;
	result =
		m_module->m_operatorMgr.getField(thisValue, field, NULL, &fieldValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, &fieldValue);

	if (!result)
		return false;

	argList->insertHead(fieldValue);

	result = m_module->m_operatorMgr.callOperator(constructor, argList);
	if (!result)
		return false;

	field->m_flags |= ModuleItemFlag_Constructed;
	return true;
}

bool
Parser::finalizeBaseTypeMemberConstructBlock()
{
	ASSERT(m_constructorType || m_constructorProperty);

	Function* constructor = m_module->m_functionMgr.getCurrentFunction();
	FunctionKind functionKind = constructor->getFunctionKind();

	ASSERT(functionKind == FunctionKind_Constructor || functionKind == FunctionKind_StaticConstructor);

	if (functionKind == FunctionKind_StaticConstructor)
	{
		MemberBlock* memberBlock = m_constructorProperty ?
			(MemberBlock*)m_constructorProperty :
			(MemberBlock*)m_constructorType;

		memberBlock->primeStaticVariables();

		return
			memberBlock->initializeStaticVariables() &&
			memberBlock->callPropertyStaticConstructors();
	}
	else
	{
		Value thisValue = m_module->m_functionMgr.getThisValue();

		if (m_constructorProperty)
			return
				m_constructorProperty->initializeFields(thisValue) &&
				m_constructorProperty->callPropertyConstructors(thisValue);
		else
			return
				m_constructorType->callBaseTypeConstructors(thisValue) &&
				m_constructorType->callStaticConstructor() &&
				m_constructorType->initializeFields(thisValue) &&
				m_constructorType->callPropertyConstructors(thisValue);
	}
}

bool
Parser::lookupIdentifier(
	const sl::StringRef& name,
	const lex::LineCol& pos,
	Value* value
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	MemberCoord coord;
	FindModuleItemResult findResult = nspace->findDirectChildItemTraverse(name, &coord);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("undeclared identifier '%s'", name.sz());
		lex::pushSrcPosError(m_module->m_unitMgr.getCurrentUnit()->getFilePath(), pos);
		return false;
	}

	Value thisValue;
	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();

	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		value->setNamespace((GlobalNamespace*)item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*)item)->getType();
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named))
		{
			err::setFormatStringError("'%s' cannot be used as expression", ((Type*)item)->getTypeString().sz());
			return false;
		}

		value->setNamespace((NamedType*)item);
		break;

	case ModuleItemKind_Const:
		*value = ((Const*)item)->getValue();
		break;

	case ModuleItemKind_Variable:
		value->setVariable((Variable*)item);
		break;

	case ModuleItemKind_Function:
		result = value->trySetFunction((Function*)item);
		if (!result)
			return false;

		if (((Function*)item)->isMember())
		{
			result = m_module->m_operatorMgr.createMemberClosure(value, (Function*)item);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_FunctionOverload:
		value->setFunctionOverload((FunctionOverload*)item);
		if (((FunctionOverload*)item)->getFlags() & FunctionOverloadFlag_HasMembers)
		{
			result = m_module->m_operatorMgr.createMemberClosure(value, (FunctionOverload*)item);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_Property:
		value->setProperty((Property*)item);
		if (((Property*)item)->isMember())
		{
			result = m_module->m_operatorMgr.createMemberClosure(value, (Property*)item);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_EnumConst:
		result = value->trySetEnumConst((EnumConst*)item);
		if (!result)
			return false;
		break;

	case ModuleItemKind_Field:
		result =
			m_module->m_operatorMgr.getThisValue(&thisValue, (Field*)item) &&
			m_module->m_operatorMgr.getField(thisValue, (Field*)item, &coord, value);

		if (!result)
			return false;

		break;

	default:
		err::setFormatStringError(
			"%s '%s' cannot be used as expression",
			getModuleItemKindString(item->getItemKind()),
			name.sz()
			);
		return false;
	};

	return true;
}

bool
Parser::prepareCurlyInitializerNamedItem(
	CurlyInitializer* initializer,
	const sl::StringRef& name
	)
{
	Value memberValue;

	bool result = m_module->m_operatorMgr.memberOperator(
		initializer->m_targetValue,
		name,
		&initializer->m_memberValue
		);

	if (!result)
		return false;

	initializer->m_index = -1;
	m_curlyInitializerTargetValue = initializer->m_memberValue;
	return true;
}

bool
Parser::prepareCurlyInitializerIndexedItem(CurlyInitializer* initializer)
{
	if (initializer->m_index == -1)
	{
		err::setFormatStringError("indexed-based initializer cannot be used after named-based initializer");
		return false;
	}

	bool result = m_module->m_operatorMgr.memberOperator(
		initializer->m_targetValue,
		initializer->m_index,
		&initializer->m_memberValue
		);

	if (!result)
		return false;

	m_curlyInitializerTargetValue = initializer->m_memberValue;
	return true;
}

bool
Parser::skipCurlyInitializerItem(CurlyInitializer* initializer)
{
	if (initializer->m_index == -1)
		return true; // allow finishing comma(s)

	initializer->m_index++;
	return true;
}

bool
Parser::assignCurlyInitializerItem(
	CurlyInitializer* initializer,
	const Value& value
	)
{
	if (initializer->m_index == -1 ||
		value.getValueKind() != ValueKind_Const ||
		!isCharArrayType(value.getType()) ||
		!isCharArrayRefType(initializer->m_targetValue.getType()))
	{
		if (initializer->m_index != -1)
			initializer->m_index++;

		initializer->m_count++;
		return m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, initializer->m_memberValue, value);
	}

	ArrayType* srcType = (ArrayType*)value.getType();
	ArrayType* dstType = (ArrayType*)((DataPtrType*)initializer->m_targetValue.getType())->getTargetType();

	size_t length = srcType->getElementCount();

	if (dstType->getElementCount() < initializer->m_index + length)
	{
		err::setFormatStringError("literal initializer is too big to fit inside the target array");
		return false;
	}

	initializer->m_index += length;
	initializer->m_count++;

	Value memberPtrValue;

	return
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, initializer->m_memberValue, &memberPtrValue) &&
		m_module->m_operatorMgr.memCpy(memberPtrValue, value, length);
}

bool
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	const Value& value,
	bool isIndex,
	const sl::StringRef& fmtSpecifierString
	)
{
	literal->m_binData.append(string.cp(), string.getLength());

	FmtSite* site = AXL_MEM_NEW(FmtSite);
	site->m_offset = literal->m_binData.getCount();
	site->m_fmtSpecifierString = fmtSpecifierString;
	literal->m_fmtSiteList.insertTail(site);
	literal->m_isZeroTerminated = true;

	if (!isIndex)
	{
		site->m_value = value;
		return true;
	}

	if (value.getValueKind() != ValueKind_Const ||
		!(value.getType()->getTypeKindFlags() & TypeKindFlag_Integer))
	{
		err::setFormatStringError("expression is not integer constant");
		return false;
	}

	site->m_index = 0;
	memcpy(&site->m_index, value.getConstData(), value.getType()->getSize());

	literal->m_lastIndex = site->m_index;
	return true;
}

void
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	size_t index
	)
{
	literal->m_binData.append(string.cp(), string.getLength());

	FmtSite* site = AXL_MEM_NEW(FmtSite);
	site->m_offset = literal->m_binData.getCount();
	site->m_index = index;
	literal->m_fmtSiteList.insertTail(site);
	literal->m_lastIndex = index;
	literal->m_isZeroTerminated = true;
}

void
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	const sl::StringRef& fmtSpecifierString
	)
{
	literal->m_binData.append(string.cp(), string.getLength());

	FmtSite* site = AXL_MEM_NEW(FmtSite);
	site->m_offset = literal->m_binData.getCount();
	site->m_index = ++literal->m_lastIndex;
	site->m_fmtSpecifierString = fmtSpecifierString;
	literal->m_fmtSiteList.insertTail(site);
	literal->m_isZeroTerminated = true;
}

bool
Parser::finalizeLiteral(
	Literal* literal,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
	)
{
	bool result;

	if (literal->m_fmtSiteList.isEmpty())
	{
		if (literal->m_isZeroTerminated)
			literal->m_binData.append(0);

		resultValue->setCharArray(literal->m_binData, literal->m_binData.getCount(), m_module);
		return true;
	}

	char buffer[256];
	sl::Array<Value*> argValueArray(ref::BufKind_Stack, buffer, sizeof(buffer));
	size_t argCount = 0;

	if (argValueList)
	{
		argCount = argValueList->getCount();
		argValueArray.setCount(argCount);

		sl::BoxIterator<Value> it = argValueList->getHead();
		for (size_t i = 0; i < argCount; i++, it++)
		{
			ASSERT(it);
			argValueArray[i] = it.p();
		}
	}

	Type* type = m_module->m_typeMgr.getStdType(StdType_FmtLiteral);
	Variable* fmtLiteral = m_module->m_variableMgr.createSimpleStackVariable("fmtLiteral", type);

	result = m_module->m_variableMgr.initializeVariable(fmtLiteral);
	ASSERT(result);

	Value fmtLiteralValue = fmtLiteral;

	size_t offset = 0;

	sl::BitMap argUsageMap;
	argUsageMap.setBitCount(argCount);

	sl::Iterator<FmtSite> siteIt = literal->m_fmtSiteList.getHead();
	for (; siteIt; siteIt++)
	{
		FmtSite* site = *siteIt;
		Value* value;

		if (site->m_index == -1)
		{
			value = &site->m_value;
		}
		else
		{
			size_t i = site->m_index - 1;
			if (i >= argCount)
			{
				err::setFormatStringError("formatting literal doesn't have argument %%%d", site->m_index);
				return false;
			}

			value = argValueArray[i];
			argUsageMap.setBit(i);
		}

		if (site->m_offset > offset)
		{
			size_t length = site->m_offset - offset;
			appendFmtLiteralRawData(
				fmtLiteralValue,
				literal->m_binData + offset,
				length
				);

			offset += length;
		}

		if (value->isEmpty())
		{
			err::setFormatStringError("formatting literals arguments cannot be skipped");
			return false;
		}

		result = appendFmtLiteralValue(fmtLiteralValue, *value, site->m_fmtSpecifierString);
		if (!result)
			return false;
	}

	size_t unusedArgIdx = argUsageMap.findBit(0, false);
	if (unusedArgIdx < argCount)
	{
		err::setFormatStringError("formatting literal argument %%%d is not used", unusedArgIdx + 1);
		return false;
	}

	size_t endOffset = literal->m_binData.getCount();
	if (endOffset > offset)
	{
		size_t length = endOffset - offset;
		appendFmtLiteralRawData(
			fmtLiteralValue,
			literal->m_binData + offset,
			length
			);
	}

	Type* validatorType = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);

	Value fatPtrValue;
	Value thinPtrValue;
	Value validatorValue;

	m_module->m_llvmIrBuilder.createGep2(fmtLiteralValue, 0, NULL, &fatPtrValue);
	m_module->m_llvmIrBuilder.createLoad(fatPtrValue, NULL, &fatPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue(fatPtrValue, 0, NULL, &thinPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue(fatPtrValue, 1, validatorType, &validatorValue);

	resultValue->setLeanDataPtr(
		thinPtrValue.getLlvmValue(),
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getDataPtrType(DataPtrTypeKind_Lean),
		validatorValue
		);

	return true;
}

void
Parser::appendFmtLiteralRawData(
	const Value& fmtLiteralValue,
	const void* p,
	size_t length
	)
{
	Function* append = m_module->m_functionMgr.getStdFunction(StdFunc_AppendFmtLiteral_a);

	Value literalValue;
	literalValue.setCharArray(p, length, m_module);
	bool result = m_module->m_operatorMgr.castOperator(&literalValue, m_module->m_typeMgr.getStdType(StdType_CharConstPtr));
	ASSERT(result);

	Value lengthValue;
	lengthValue.setConstSizeT(length, m_module);

	Value resultValue;
	m_module->m_llvmIrBuilder.createCall3(
		append,
		append->getType(),
		fmtLiteralValue,
		literalValue,
		lengthValue,
		&resultValue
		);
}

bool
Parser::appendFmtLiteralValue(
	const Value& fmtLiteralValue,
	const Value& rawSrcValue,
	const sl::StringRef& fmtSpecifierString
	)
{
	if (fmtSpecifierString == "B") // binary format
		return appendFmtLiteralBinValue(fmtLiteralValue, rawSrcValue);

	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand(rawSrcValue, &srcValue);
	if (!result)
		return false;

	StdFunc appendFunc;

	Type* type = srcValue.getType();
	TypeKind typeKind = type->getTypeKind();
	uint_t typeKindFlags = type->getTypeKindFlags();

	if (typeKindFlags & TypeKindFlag_Integer)
	{
		static StdFunc funcTable[2][2] =
		{
			{ StdFunc_AppendFmtLiteral_i32, StdFunc_AppendFmtLiteral_ui32 },
			{ StdFunc_AppendFmtLiteral_i64, StdFunc_AppendFmtLiteral_ui64 },
		};

		size_t i1 = type->getSize() > 4;
		size_t i2 = (typeKindFlags & TypeKindFlag_Unsigned) != 0;

		appendFunc = funcTable[i1][i2];
	}
	else if (typeKindFlags & TypeKindFlag_Fp)
	{
		appendFunc = StdFunc_AppendFmtLiteral_f;
	}
	else if (typeKind == TypeKind_Variant)
	{
		appendFunc = StdFunc_AppendFmtLiteral_v;
	}
	else if (isCharArrayType(type) || isCharArrayRefType(type) || isCharPtrType(type))
	{
		appendFunc = StdFunc_AppendFmtLiteral_p;
	}
	else
	{
		err::setFormatStringError("don't know how to format '%s'", type->getTypeString().sz());
		return false;
	}

	Function* append = m_module->m_functionMgr.getStdFunction(appendFunc);
	Type* argType = append->getType()->getArgArray() [2]->getType();

	Value argValue;
	result = m_module->m_operatorMgr.castOperator(srcValue, argType, &argValue);
	if (!result)
		return false;

	Value fmtSpecifierValue;
	if (!fmtSpecifierString.isEmpty())
	{
		fmtSpecifierValue.setCharArray(fmtSpecifierString, m_module);
		m_module->m_operatorMgr.castOperator(&fmtSpecifierValue, m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getDataPtrType_c());
	}
	else
	{
		fmtSpecifierValue = m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getDataPtrType_c()->getZeroValue();
	}

	return m_module->m_operatorMgr.callOperator(
		append,
		fmtLiteralValue,
		fmtSpecifierValue,
		argValue
		);
}

bool
Parser::appendFmtLiteralBinValue(
	const Value& fmtLiteralValue,
	const Value& rawSrcValue
	)
{
	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand(rawSrcValue, &srcValue);
	if (!result)
		return false;

	Type* type = srcValue.getType();
	Function* append = m_module->m_functionMgr.getStdFunction(StdFunc_AppendFmtLiteral_a);
	Type* argType = m_module->m_typeMgr.getStdType(StdType_BytePtr);

	Value sizeValue(
		type->getSize(),
		m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)
		);

	Value tmpValue;
	Value resultValue;
	m_module->m_llvmIrBuilder.createAlloca(type, "tmpFmtValue", NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createStore(srcValue, tmpValue);
	m_module->m_llvmIrBuilder.createBitCast(tmpValue, argType, &tmpValue);

	m_module->m_llvmIrBuilder.createCall3(
		append,
		append->getType(),
		fmtLiteralValue,
		tmpValue,
		sizeValue,
		&resultValue
		);

	return true;
}

bool
Parser::finalizeReSwitchCaseLiteral(
	sl::StringRef* data,
	const Value& value,
	bool isZeroTerminated
	)
{
	if (value.getValueKind() != ValueKind_Const)
	{
		err::setFormatStringError("not a constant literal expression");
		return false;
	}

	size_t length = value.m_type->getSize();
	if (isZeroTerminated)
	{
		ASSERT(length);
		length--;
	}

	*data = sl::StringRef(value.m_constData.getHdr(), value.m_constData.cp(), length);
	return true;
}

BasicBlock*
Parser::assertCondition(const sl::BoxList<Token>& tokenList)
{
	bool result;

	Value conditionValue;
	result = m_module->m_operatorMgr.parseExpression(tokenList, &conditionValue);
	if (!result)
		return NULL;

	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock("assert_fail");
	BasicBlock* continueBlock = m_module->m_controlFlowMgr.createBlock("assert_continue");

	result = m_module->m_controlFlowMgr.conditionalJump(conditionValue, continueBlock, failBlock, failBlock);
	if (!result)
		return NULL;

	return continueBlock;
}

bool
Parser::finalizeAssertStmt(
	const sl::BoxList<Token>& conditionTokenList,
	const Value& messageValue,
	BasicBlock* continueBlock
	)
{
	ASSERT(!conditionTokenList.isEmpty());

	sl::String fileName = m_module->m_unitMgr.getCurrentUnit()->getFilePath();
	sl::String conditionString = Token::getTokenListString(conditionTokenList);
	Token::Pos pos = conditionTokenList.getHead()->m_pos;

	Value fileNameValue;
	Value lineValue;
	Value conditionValue;

	fileNameValue.setCharArray(fileName, m_module);
	lineValue.setConstInt32(pos.m_line, m_module);
	conditionValue.setCharArray(conditionString, m_module);

	Function* assertionFailure = m_module->m_functionMgr.getStdFunction(StdFunc_AssertionFailure);

	sl::BoxList<Value> argValueList;
	argValueList.insertTail(fileNameValue);
	argValueList.insertTail(lineValue);
	argValueList.insertTail(conditionValue);

	if (messageValue)
	{
		argValueList.insertTail(messageValue);
	}
	else
	{
		Value nullValue;
		nullValue.setNull(m_module);
		argValueList.insertTail(nullValue);
	}

	bool result = m_module->m_operatorMgr.callOperator(assertionFailure, &argValueList);
	if (!result)
		return false;

	m_module->m_controlFlowMgr.follow(continueBlock);
	return true;
}

void
Parser::addScopeAnchorToken(
	StmtPass1* stmt,
	const Token& token
	)
{
	sl::BoxIterator<Token> it = stmt->m_tokenList.insertTail(token);
	stmt->m_scopeAnchorToken = &*it;
	stmt->m_scopeAnchorToken->m_data.m_integer = 0; // tokens can be reused, ensure 0
}

//..............................................................................

} // namespace ct
} // namespace jnc
