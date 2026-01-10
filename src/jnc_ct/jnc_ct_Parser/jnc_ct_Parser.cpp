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
#include "jnc_ct_Closure.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_DynamicLibClassType.h"
#include "jnc_ct_DynamicLibNamespace.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_AsyncLauncherFunction.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_ct_Parser.llk.cpp"

namespace jnc {
namespace ct {

//..............................................................................

inline
Parser::Literal::Literal() {
	m_fmtMlOffset = 0;
	m_fmtIndex = 0;
	m_isZeroTerminated = false;
}

Parser::FmtSite*
Parser::Literal::addFmtSite(
	const sl::StringRef& string,
	uint_t flags
) {
	size_t offset = m_binData.getCount();
	m_binData.append(string.cp(), string.getLength());

	FmtSite* site = new FmtSite;
	site->m_offset = m_binData.getCount();
	m_fmtSiteList.insertTail(site);
	m_isZeroTerminated = true;

	if ((flags & FmtLiteralTokenFlag_Ml) && !m_fmtMlFirstIt) {
		m_fmtMlFirstIt = site;
		m_fmtMlOffset = offset;
	}

	return site;
}

//..............................................................................

Parser::Parser(
	Module* module,
	const PragmaConfig* pragmaConfig,
	Mode mode
):
	m_doxyParser(&module->m_doxyModule) {
	m_module = module;
	m_mode = mode;

	if (pragmaConfig)
		m_pragmaConfig = *pragmaConfig;

	m_pragmaConfigSnapshot = pragmaConfig;
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_attributeBlockState = AttributeBlockState_Undefined;
	m_attributeBlock = NULL;
	m_lastDeclaredItem = NULL;
	m_lastPropertyGetterType = NULL;
	m_lastPropertyTypeModifiers = 0;
	m_topDeclarator = NULL;
	m_importTypeNameAnchor = NULL;
	m_declarationId = 0;

	m_constructorType = NULL;
	m_constructorProperty = NULL;
}

bool
Parser::tokenizeBody(
	sl::List<Token>* tokenList,
	const lex::LineColOffset& pos,
	const sl::StringRef& body
) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	uint_t flags = m_mode == Mode_Parse ? LexerFlag_Parse : 0;
	if ((m_module->getCompileFlags() & ModuleCompileFlag_Documentation) && !unit->getLib())
		flags |= LexerFlag_DoxyComments; // also include doxy-comments (but not for libs!)

	Lexer lexer(flags);
	lexer.create(unit->getFilePath(), body);
	lexer.setLineColOffset(pos);

	char buffer[256];
	sl::Array<Token*> scopeAnchorTokenStack(rc::BufKind_Stack, buffer, sizeof(buffer));
	bool isScopeFlagMarkupRequired = m_mode != Mode_Parse;
	bool hasLandingPadFlags = false;

	for (;;) {
		const Token* token0 = lexer.getToken();
		switch (token0->m_token) {
		case TokenKind_Eof: // don't add EOF token (parseTokenList adds one automatically)
			if (hasLandingPadFlags) {
				Token* token = tokenList->getHead().p();
				if (token->m_token == '{')
					token->m_data.m_integer |= ScopeFlag_HasLandingPads;
			}
			return true;

		case TokenKind_Error:
			err::setFormatStringError("invalid character '\\x%02x'", (uchar_t)token0->m_data.m_integer);
			pushSrcPosError(token0->m_pos);
			return false;
		}

		Token* token = lexer.takeToken();
		tokenList->insertTail(token);
		ASSERT(token == token0);

		Token* anchorToken;
		if (isScopeFlagMarkupRequired)
			switch (token->m_token) {
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
				if (!scopeAnchorTokenStack.isEmpty()) {
					anchorToken = tokenList->getTail().p();
					anchorToken->m_data.m_integer = 0; // tokens can be reused, ensure 0
					scopeAnchorTokenStack.getBack() = anchorToken;
				}
				break;

			case TokenKind_Catch:
				if (!scopeAnchorTokenStack.isEmpty())
					scopeAnchorTokenStack.getBack()->m_data.m_integer |= ScopeFlag_CatchAhead | ScopeFlag_HasCatch;

				hasLandingPadFlags = true;
				break;

			case TokenKind_Finally:
				if (!scopeAnchorTokenStack.isEmpty())
					scopeAnchorTokenStack.getBack()->m_data.m_integer |= ScopeFlag_FinallyAhead | ScopeFlag_Finalizable;

				hasLandingPadFlags = true;
				break;

			case TokenKind_Try:
				hasLandingPadFlags = true;
				break;
			}
	}

	ASSERT(false); // should never get here
}

bool
Parser::pragma(
	const sl::StringRef& name,
	PragmaState state,
	int64_t value
) {
	Pragma pragmaKind = PragmaMap::findValue(name, Pragma_Undefined);
	if (!pragmaKind) {
		err::setFormatStringError("unknown pragma '%s'", name.sz());
		return false;
	}

	m_pragmaConfigSnapshot = NULL;
	return m_pragmaConfig.setPragma(pragmaKind, state, value);
}

void
Parser::addDoxyComment(const Token& token) {
	uint_t compileFlags = m_module->getCompileFlags();
	if (compileFlags & (ModuleCompileFlag_DisableDoxyComment1 << (token.m_token - TokenKind_DoxyComment1)))
		return;

	sl::StringRef comment = token.m_data.m_string;
	ModuleItem* lastDeclaredItem = NULL;
	lex::LineCol pos = token.m_pos;
	pos.m_col += 3; // doxygen comments always start with 3 characters: ///, //!, /** /*!

	if (!comment.isEmpty() && comment[0] == '<') {
		lastDeclaredItem = m_lastDeclaredItem;
		comment = comment.getSubString(1);
		pos.m_col++;
	}

	m_doxyParser.addComment(
		comment,
		pos,
		token.m_token <= TokenKind_DoxyComment2, // only concat single-line comments
		lastDeclaredItem
	);
}

bool
Parser::parseBody(
	SymbolKind symbol,
	const lex::LineColOffset& pos,
	const sl::StringRef& body
) {
	sl::List<Token> tokenList;

	bool result = tokenizeBody(&tokenList, pos, body);
	if (!result)
		return false;

	return !tokenList.isEmpty() ?
		parseTokenList(symbol, &tokenList) :
		create(m_module->m_unitMgr.getCurrentUnit()->getFilePath(), symbol) &&
		parseEofToken(pos, body.getLength());
}

bool
Parser::parseTokenList(
	SymbolKind symbol,
	sl::List<Token>* tokenList
) {
	ASSERT(!tokenList->isEmpty());

	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	create(unit->getFilePath(), symbol);

	Token::Pos lastTokenPos = tokenList->getTail()->m_pos;

	if (!m_module->m_codeAssistMgr.getCodeAssistKind() ||
		!unit->isRootUnit() ||
		!isOffsetInsideTokenList(tokenList, m_module->m_codeAssistMgr.getOffset())
	) {
		while (!tokenList->isEmpty()) {
			Token* token = tokenList->removeHead();
			token->m_data.m_codeAssistFlags = 0; // tokens can be reused -- ensure 0
			bool result = consumeToken(token);
			if (!result)
				return false;
		}

		return parseEofToken(lastTokenPos, lastTokenPos.m_length); // might trigger actions
	} else {
		size_t offset = m_module->m_codeAssistMgr.getOffset();
		size_t declarationId = -1;

		bool result = true;
		bool isAfter = false;

		if (m_mode != Mode_Compile)
			while (!tokenList->isEmpty()) {
				Token* token = tokenList->removeHead();
				if (isAfter)
					token->m_data.m_codeAssistFlags = TokenCodeAssistFlag_After;
				else if (calcTokenCodeAssistFlags(token, offset)) {
					if (token->m_tokenKind == TokenKind_Identifier && (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_At))
						m_module->m_codeAssistMgr.prepareIdentifierFallback(*token);

					if (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_After) {
						m_module->m_codeAssistMgr.prepareNamespaceFallback();
						isAfter = true;
					}
				}

				if (result)
					result = consumeToken(token);
				else { // keep scanning tokens even after error -- until we get any fallback
					m_tokenPool->put(token);
					if (m_module->m_codeAssistMgr.hasFallBack())
						break;
				}
			}
		else
			while (!tokenList->isEmpty()) {
				Token* token = tokenList->removeHead();
				if (isAfter) {
					if (declarationId != m_declarationId) { // keep parsing some more (until the end of the declaration)
						lastTokenPos = token->m_pos;
						m_tokenPool->put(token);
						break;
					}
					token->m_data.m_codeAssistFlags = TokenCodeAssistFlag_After;
				} else if (calcTokenCodeAssistFlags(token, offset)) {
					if (token->m_tokenKind == TokenKind_Identifier && (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_At))
						m_module->m_codeAssistMgr.prepareIdentifierFallback(*token);

					if (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_After) {
						declarationId = m_declarationId;
						m_module->m_codeAssistMgr.prepareNamespaceFallback();
						isAfter = true;
					}
				}

				if (result)
					result = consumeToken(token);
				else { // keep scanning tokens even after error -- until we get any fallback
					m_tokenPool->put(token);
					if (m_module->m_codeAssistMgr.hasFallBack())
						break;
				}
			}

		if (result)
			result = parseEofToken(lastTokenPos, lastTokenPos.m_length); // might trigger actions

		if (!m_module->m_codeAssistMgr.getCodeAssist() &&
			m_module->m_codeAssistMgr.hasArgumentTipStack())
			m_module->m_codeAssistMgr.createArgumentTipFromStack();

		return result;
	}
}

bool
Parser::parseEofToken(
	const lex::LineColOffset& pos,
	size_t length
) {
	Token* token = m_tokenPool->get();
	token->m_token = 0;
	token->m_pos.m_line = pos.m_line;
	token->m_pos.m_col = pos.m_col + length;
	token->m_pos.m_offset = pos.m_offset + length;
	return consumeToken(token);
}

Type*
Parser::getQualifiedTypeName(
	QualifiedName* name,
	const lex::LineCol& pos
) {
	ASSERT(m_mode == Mode_Parse);

	Type* type;
	ModuleItemPos* itemPos;
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	if (nspace->getNamespaceKind() == NamespaceKind_TemplateDeclaration) {
		TemplateTypeName* typeName = m_module->m_typeMgr.getTemplateTypeName(nspace, name);
		type = typeName;
		itemPos = typeName;
	} else {
		// for nested declarators, use top-level anchor
		ImportTypeName* typeName = m_module->m_typeMgr.getImportTypeName(nspace, name, getImportTypeNameAnchor());
		type = typeName;
		itemPos = typeName;
	}

	if (!itemPos->m_parentUnit)
		setItemPos(itemPos, pos);

	return type;
}

Type*
Parser::getQualifiedTypeName(ModuleItem* item) {
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Type:
		return (Type*)item;

	case ModuleItemKind_Typedef:
		return ((Typedef*)item)->getType();

	case ModuleItemKind_Template:
		if (Type* type = m_module->m_namespaceMgr.getCurrentNamespace()->findTemplateInstanceType((Template*)item))
			return type;

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a type", item->getItemName().sz());
		return NULL;
	}
}

bool
Parser::isQualifiedTypeName(ModuleItem* item) {
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Type:
	case ModuleItemKind_Typedef:
		return true;

	case ModuleItemKind_Template:
		return ((Template*)item)->getDerivableTypeKind() != TypeKind_Void;

	default:
		return false;
	}
}

ModuleItem*
Parser::instantiateTemplate(
	ModuleItem* item,
	const sl::ArrayRef<Type*>& argArray
) {
	if (item->getItemKind() != ModuleItemKind_Template) {
		err::setFormatStringError("'%s' is not a template", item->getItemName().sz());
		return NULL;
	}

	return ((Template*)item)->instantiate(argArray);
}

AttributeBlock*
Parser::popAttributeBlock() {
	AttributeBlock* attributeBlock = m_attributeBlock;
	m_attributeBlock = NULL;
	m_attributeBlockState = AttributeBlockState_Undefined;
	return attributeBlock;
}

void Parser::processUnusedAttributes() {
	ASSERT(m_attributeBlock);

	err::setError("unused attribute block");
	m_attributeBlock->ensureSrcPosError();
	m_attributeBlock = NULL;
	m_attributeBlockState = AttributeBlockState_Undefined;
}

bool
Parser::createAttributeBlock(const lex::LineCol& pos) {
	AttributeBlock* attributeBlock = m_module->m_attributeMgr.createAttributeBlock();
	attributeBlock->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	attributeBlock->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	attributeBlock->m_pos = pos;

	bool result;

	if (!m_attributeBlock)
		result = true;
	else {
		err::setError("unused attribute block");
		m_attributeBlock->ensureSrcPosError();
		result = false;
	}

	m_attributeBlock = attributeBlock;
	m_attributeBlockState = AttributeBlockState_Created;
	return result;
}

bool
Parser::createAttribute(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	sl::List<Token>* initializer
) {
	ASSERT(m_attributeBlock);

	Attribute* attribute = m_module->m_attributeMgr.createAttribute(name, initializer);
	attribute->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	attribute->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	attribute->m_pos = pos;
	return m_attributeBlock->addAttribute(attribute);
}

bool
Parser::reuseAttributes(const QualifiedName& name) {
	ASSERT(m_attributeBlock);

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	FindModuleItemResult findResult = nspace->findItemTraverse(*m_attributeBlock, name);
	if (!findResult.m_result)
		return false;

	ModuleItemDecl* decl = findResult.m_item ? findResult.m_item->getDecl() : NULL;
	AttributeBlock* attributeBlock = decl ? decl->getAttributeBlock() : NULL;
	if (!attributeBlock) {
		err::setFormatStringError("declaration '%s' not found or has no attributes", name.getFullName().sz());
		return false;
	}

	m_attributeBlock->addAttributeBlock(attributeBlock);
	return true;
}

bool
Parser::postDeclaration() {
	if (m_module->m_namespaceMgr.getCurrentNamespace()->getNamespaceKind() == NamespaceKind_TemplateDeclaration)
		m_module->m_namespaceMgr.closeAllTemplateDeclNamespaces(); // error recovery

	m_declarationId++;

	if (!m_attributeBlock)
		return true;

	if (m_attributeBlockState == AttributeBlockState_Created) {
		m_attributeBlockState = AttributeBlockState_Staged;
		return true;
	}

	processUnusedAttributes();
	return false;
}

size_t
Parser::prepareDynamicAttributeArgs(
	sl::BoxList<Value>* argList,
	AttributeBlock* attributeBlock
) {
	const sl::Array<Attribute*>& attributeArray = attributeBlock->getAttributeArray();
	size_t count = attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Attribute* attribute = attributeArray[i];
		if (!(attribute->getFlags() & AttributeFlag_DynamicValue))
			continue;

		Value attrValue;
		bool result = m_module->m_operatorMgr.castOperator(attribute->getValue(), TypeKind_Variant, &attrValue);
		if (!result)
			return -1;

		argList->insertTail(attrValue);
	}

	count = argList->getCount();
	if (!count)
		return 0;

	Value countValue(count, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
	argList->insertHead(countValue);
	return count;
}

void
Parser::preDeclaration() {
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_lastDeclaredItem = NULL;
}

bool
Parser::bodylessDeclaration() {
	ASSERT(m_lastDeclaredItem);

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Property:
		return finalizeLastProperty(false);

	case ModuleItemKind_Orphan:
		err::setFormatStringError(
			"orphan '%s' without a body",
			m_lastDeclaredItem->getItemName().sz()
		);
		return false;
	}

	return true;
}

bool
Parser::setDeclarationBody(const Token& bodyToken) {
	if (!m_lastDeclaredItem) {
		err::setError("declaration without declarator cannot have a body");
		return false;
	}

	Type* type;

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Function:
		if (m_module->m_namespaceMgr.getCurrentNamespace()->getNamespaceKind() == NamespaceKind_DynamicLib) {
			err::setError("dynamiclib function cannot have a body");
			return false;
		}

		return setBody((Function*)m_lastDeclaredItem, bodyToken);

	case ModuleItemKind_Property:
		return parseLastPropertyBody(bodyToken);

	case ModuleItemKind_Template:
		return setBody((Template*)m_lastDeclaredItem, bodyToken);

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
		return setBody((Orphan*)m_lastDeclaredItem, bodyToken);

	default:
		err::setFormatStringError("'%s' cannot have a body", m_lastDeclaredItem->getItemName().sz());
		return false;
	}

	if (!isClassType(type, ClassTypeKind_Reactor)) {
		err::setFormatStringError("only functions and reactors can have bodies, not '%s'", type->getTypeString().sz());
		return false;
	}

	return setBody((ReactorClassType*)type, bodyToken);
}

template <typename T>
bool
Parser::setBody(
	T* item,
	const Token& bodyToken
) {
	if (bodyToken.m_data.m_codeAssistFlags & TokenCodeAssistFlag_At)
		m_module->m_codeAssistMgr.m_containerItem = item;

	item->addUsingSet(m_module->m_namespaceMgr.getCurrentNamespace());
	return item->setBody(getPragmaConfigSnapshot(), bodyToken.m_pos, bodyToken.m_data.m_string);
}

template <typename T>
bool
Parser::setBody(
	T* item,
	sl::List<Token>* tokenList
) {
	ASSERT(!tokenList->isEmpty());

	const Token* head = *tokenList->getHead();
	if (head->m_token == TokenKind_Body) {
		ASSERT(tokenList->getCount() == 1);
		return setBody(item, *head);
	}

	const Token* tail = *tokenList->getTail();
	if (head->m_data.m_codeAssistFlags <= TokenCodeAssistFlag_Right &&
		tail->m_data.m_codeAssistFlags >= TokenCodeAssistFlag_Left
	)
		m_module->m_codeAssistMgr.m_containerItem = item;

	item->addUsingSet(m_module->m_namespaceMgr.getCurrentNamespace());
	return item->setBody(getPragmaConfigSnapshot(), tokenList);
}

bool
Parser::setStorageKind(StorageKind storageKind) {
	if (m_storageKind) {
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
Parser::setAccessKind(AccessKind accessKind) {
	if (m_accessKind) {
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
Parser::preDeclarator(
	Declarator* declarator,
	TypeSpecifier* typeSpecifier
) {
	declarator->setTypeSpecifier(typeSpecifier, m_module);
	if (m_topDeclarator)
		return;

	m_topDeclarator = declarator;
	m_importTypeNameAnchor = NULL;
}

template <typename T>
void
Parser::replaceDeclaratorTypeName(
	Declarator* declarator,
	T* typeName
) {
	if (!typeName->m_parentUnit)
		setItemPos(typeName, declarator->getPos());

	declarator->m_baseType = typeName;
}

void
Parser::postDeclaratorName(Declarator* declarator) {
	if (declarator != m_topDeclarator || // nested declarators should be already properly anchored
		declarator->m_baseType->getTypeKind() != TypeKind_ImportTypeName
	)
		return;

	// check if we need to re-anchor the type name:
	//
	// T foo<T>();   <-- here 'T' should be searched for starting at template inst namespace
	// A C<T>.foo(); <-- here 'A' should be searched for starting at 'C<T>'
	// A C.foo();    <-- here 'A' should be searched for starting at 'C'

	ImportTypeName* typeName = (ImportTypeName*)declarator->m_baseType;
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	if (nspace->getNamespaceKind() == NamespaceKind_TemplateDeclaration) {
		QualifiedName name;
		name.copy(typeName->m_name);
		replaceDeclaratorTypeName(
			declarator,
			m_module->m_typeMgr.getTemplateTypeName(nspace, &name)
		);
	} else if (declarator->isQualified()) {
		ASSERT(!m_importTypeNameAnchor);
		m_importTypeNameAnchor = m_module->m_typeMgr.createImportTypeNameAnchor();

		replaceDeclaratorTypeName(
			declarator,
			m_module->m_typeMgr.getAnchoredImportTypeName(
				nspace,
				typeName->m_name,
				m_importTypeNameAnchor
			)
		);
	}
}

void
Parser::postDeclarator(Declarator* declarator) {
	if (declarator == m_topDeclarator)
		m_topDeclarator = NULL;
}

bool
Parser::addTemplateSuffixToDeclarator(Declarator* declarator) {
	if (declarator != m_topDeclarator)
		return err::fail("only top-level declarator can be templated");

	QualifiedNameAtom* atom = declarator->m_name.getLastAtom();
	ASSERT(atom->m_atomKind == QualifiedNameAtomKind_Name);
	atom->m_atomKind = QualifiedNameAtomKind_TemplateDeclSuffix;
	sl::takeOver(&atom->m_templateDeclArgArray, &m_templateArgArray);
	return true;
}

Orphan*
Parser::createOrphan(
	OrphanKind orphanKind,
	FunctionKind functionKind,
	Declarator* declarator,
	Type* type
) {
	Orphan* orphan = m_module->m_namespaceMgr.createOrphan(orphanKind, functionKind, declarator, type);
	orphan->m_importTypeNameAnchor = m_importTypeNameAnchor;
	assignDeclarationAttributes(orphan, orphan, declarator);
	m_module->m_namespaceMgr.getCurrentNamespace()->addOrphan(orphan);
	return orphan;
}

GlobalNamespace*
Parser::getGlobalNamespace(
	GlobalNamespace* parentNamespace,
	const QualifiedNameAtom& name,
	const lex::LineCol& pos
) {
	GlobalNamespace* nspace;

	if (name.m_atomKind != QualifiedNameAtomKind_Name) {
		err::setFormatStringError("invalid namespace name");
		return NULL;
	}

	FindModuleItemResult findResult = parentNamespace->findDirectChildItem(name.m_name);
	if (!findResult.m_result)
		return NULL;

	if (!findResult.m_item) {
		nspace = m_module->m_namespaceMgr.createGlobalNamespace(name.m_name, parentNamespace);
		nspace->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
		nspace->m_pos = pos;
		parentNamespace->addItem(nspace);
	} else {
		if (findResult.m_item->getItemKind() != ModuleItemKind_Namespace) {
			err::setFormatStringError("'%s' exists and is not a namespace", findResult.m_item->getItemName().sz());
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
	const Token& bodyToken
) {
	Namespace* currentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	if (currentNamespace->getNamespaceKind() != NamespaceKind_Global) {
		err::setFormatStringError("cannot open global namespace in '%s'", getNamespaceKindString(currentNamespace->getNamespaceKind()));
		return NULL;
	}

	GlobalNamespace* nspace = getGlobalNamespace((GlobalNamespace*)currentNamespace, name.getFirstAtom(), pos);
	if (!nspace)
		return NULL;

	sl::ConstBoxIterator<QualifiedNameAtom> it = name.getAtomList().getHead();
	for (; it; it++) {
		nspace = getGlobalNamespace(nspace, *it, pos);
		if (!nspace)
			return NULL;
	}

	nspace->addBody(
		m_module->m_unitMgr.getCurrentUnit(),
		getPragmaConfigSnapshot(),
		bodyToken.m_pos,
		bodyToken.m_data.m_string
	);

	if (bodyToken.m_data.m_codeAssistFlags & TokenCodeAssistFlag_At)
		m_module->m_codeAssistMgr.m_containerItem = nspace;

	return nspace;
}

ExtensionNamespace*
Parser::declareExtensionNamespace(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	Type* type,
	const Token& bodyToken
) {
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

	result = nspace->setBody(
		getPragmaConfigSnapshot(),
		bodyToken.m_pos,
		bodyToken.m_data.m_string
	);

	ASSERT(result);

	if (bodyToken.m_data.m_codeAssistFlags & TokenCodeAssistFlag_At)
		m_module->m_codeAssistMgr.m_containerItem = nspace;

	return nspace;
}

bool
Parser::useNamespaces(
	NamespaceKind namespaceKind,
	sl::BoxList<QualifiedName>* nameList
) {
	ModuleItemContext context(m_module);
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	bool finalResult = true;
	while (!nameList->isEmpty()) {
		sl::BoxListEntry<QualifiedName>* entry = nameList->removeHeadEntry();
		bool result = nspace->m_usingSet.addNamespace(context, namespaceKind, &entry->m_value);
		delete entry;
		if (!result)
			finalResult = false;
	}

	return finalResult;
}

bool
Parser::addFriends(sl::BoxList<QualifiedName>* nameList) {
	ModuleItemContext context(m_module);
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	MemberBlock* block = nspace->getMemberBlock();
	ASSERT(block);

	bool finalResult = true;
	while (!nameList->isEmpty()) {
		sl::BoxListEntry<QualifiedName>* entry = nameList->removeHeadEntry();
		bool result = block->m_friendSet.addNamespace(context, NamespaceKind_Undefined, &entry->m_value);
		delete entry;
		if (!result)
			finalResult = false;
	}

	return finalResult;
}

bool
Parser::declareReactorVariable(
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
) {
	ASSERT(m_module->m_controlFlowMgr.isReactor() && declarator->isSimple());

	ReactorClassType* reactorType = m_module->m_controlFlowMgr.getReactorType();
	if (m_storageKind) {
		// StorageKind_Static should be fine, too -- but we currently handle StorageKind_Reactor only
		err::setError("invalid storage kind in reactor");
		return false;
	}

	if (!declarator->m_constructor.isEmpty()) {
		err::setError("reactor variables can't have non-trivial constructors");
		return false;
	}

	const sl::StringRef& name = declarator->getSimpleName();
	Variable* variable = m_module->m_variableMgr.createVariable(StorageKind_Reactor, name, type, ptrTypeFlags);
	assignDeclarationAttributes(variable, variable, declarator->m_pos, declarator->m_attributeBlock, NULL);

	return
		m_module->m_variableMgr.allocateVariable(variable) &&
		m_module->m_namespaceMgr.getCurrentNamespace()->addItem(variable) && (
			declarator->m_initializer.isEmpty() ||
			m_module->m_operatorMgr.parseReactiveInitializer(variable, &declarator->m_initializer)
		);
}

bool
Parser::declareNamedAttributeBlock(Declarator* declarator) {
	ASSERT(declarator->m_attributeBlock);

	if (!declarator->isSimple()) {
		err::setError("invalid named attribute block declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();

	AttributeBlock* attributeBlock = m_module->m_attributeMgr.createAttributeBlock();
	attributeBlock->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	attributeBlock->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	attributeBlock->m_name = declarator->getSimpleName();
	attributeBlock->m_attributeBlock = declarator->m_attributeBlock;
	attributeBlock->m_pos = declarator->m_pos;
	attributeBlock->m_flags |= ModuleItemFlag_User;

	bool result = nspace->addItem(attributeBlock);
	if (!result)
		return false;

	m_lastDeclaredItem = attributeBlock;
	return true;
}

bool
Parser::declare(Declarator* declarator) {
	m_lastDeclaredItem = NULL;

	NamespaceKind nspaceKind = m_module->m_namespaceMgr.getCurrentNamespace()->getNamespaceKind();
	if (nspaceKind == NamespaceKind_TemplateDeclaration)
		return declareTemplate(declarator);

	if ((declarator->getTypeModifiers() & TypeModifier_Property) && m_storageKind != StorageKind_Typedef) {
		if (nspaceKind == NamespaceKind_DynamicLib) {
			err::setError("only functions can be part of library");
			return false;
		}

		// too early to calctype cause maybe this property has a body
		// declare a typeless property for now

		return declareProperty(declarator, NULL, 0);
	}

	uint_t declFlags = 0;
	Type* type = declarator->calcType(&declFlags);
	if (!type)
		return false;

	DeclaratorKind declaratorKind = declarator->getDeclaratorKind();
	TypeKind typeKind = type->getTypeKind();

	if (nspaceKind == NamespaceKind_DynamicLib && typeKind != TypeKind_Function) {
		err::setError("only functions can be part of library");
		return false;
	}

	switch (m_storageKind) {
	case StorageKind_Typedef:
		return declareTypedef(declarator, type);

	case StorageKind_Alias:
		return declareAlias(declarator, type, declFlags);

	default:
		switch (typeKind) {
		case TypeKind_Void:
			if (!declarator->m_attributeBlock &&
				!(declarator->m_attributeBlock = popAttributeBlock())
			) {
				err::setError("illegal use of type 'void'");
				return false;
			}

			return declareNamedAttributeBlock(declarator);

		case TypeKind_Function:
			return declareFunction(declarator, (FunctionType*)type, declFlags);

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
) {
	decl->m_accessKind = m_accessKind ?
		m_accessKind :
		m_module->m_namespaceMgr.getCurrentAccessKind();

	// don't overwrite storage unless explicit

	if (m_storageKind)
		decl->m_storageKind = m_storageKind;

	if (!attributeBlock)
		attributeBlock = popAttributeBlock();

	setItemPos(decl, pos);
	decl->m_pragmaConfig = getPragmaConfigSnapshot();
	decl->m_attributeBlock = attributeBlock;

	if (m_module->getCompileFlags() & ModuleCompileFlag_Documentation)
		m_module->m_doxyHost.setItemBlock(item, decl, doxyBlock ? doxyBlock : m_doxyParser.popBlock());

	item->m_flags |= ModuleItemFlag_User;

	if (attributeBlock)
		m_module->notifyAttributeObserver(item, attributeBlock);

	m_lastDeclaredItem = item;
}

bool
Parser::addTemplateArg(
	const sl::StringRef& name,
	Declarator* defaultTypeDeclarator
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	ASSERT(nspace->getNamespaceKind() == NamespaceKind_TemplateDeclaration);

	size_t index = m_templateArgArray.getCount();
	TemplateArgType* type = m_module->m_typeMgr.createTemplateArgType(name, index, defaultTypeDeclarator);
	bool result = nspace->addItem(name, type);
	if (!result)
		return false;

	m_templateArgArray.append(type);
	return true;
}

bool
Parser::checkTemplateName(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	Namespace* templNspace
) {
	if (!templNspace->findDirectChildItem(name).m_item)
		return true;

	err::setFormatStringError("template name '%s' conflicts with template arguments", name.sz());
	pushSrcPosError(pos);
	return false;
}

bool
Parser::declareTemplate(Declarator* declarator) {
	Namespace* templNspace = m_module->m_namespaceMgr.getCurrentNamespace();
	m_module->m_namespaceMgr.closeAllTemplateDeclNamespaces();

	if (declarator->isQualified()) {
		TemplateDeclType* type = m_module->m_typeMgr.createTemplateDeclType(declarator);
		declarator = type->getDeclarator(); // adjust declarator (original was just moved)
		createOrphan(OrphanKind_Template, declarator->getFunctionKind(), declarator, type);
		return true;
	}

	TemplateDeclType* type = m_module->m_typeMgr.createTemplateDeclType(declarator);
	declarator = type->getDeclarator(); // adjust declarator (original was just moved)
	const sl::StringRef& name = declarator->getName().getFirstAtom().m_name;
	if (!checkTemplateName(declarator->getPos(), name, templNspace))
		return false;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	Template* templ = m_module->m_templateMgr.createTemplate(name, type);
	bool result = nspace->addItem(name, templ);
	if (!result)
		return false;

	assignDeclarationAttributes(templ, templ, declarator);
	return true;
}

bool
Parser::declareTemplate(
	TypeKind typeKind,
	const Token& nameToken,
	const sl::Array<TemplateDeclType*>& baseTypeArray,
	sl::List<Token>* bodyTokenList
) {
	Namespace* templNspace = m_module->m_namespaceMgr.getCurrentNamespace();
	m_module->m_namespaceMgr.closeTemplateDeclNamespace();

	const sl::StringRef& name = nameToken.m_data.m_string;
	if (!checkTemplateName(nameToken.m_pos, name, templNspace))
		return false;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	Template* templ = m_module->m_templateMgr.createTemplate(
		typeKind,
		name,
		m_templateArgArray,
		baseTypeArray
	);

	bool result = nspace->addItem(name, templ);
	if (!result)
		return false;

	assignDeclarationAttributes(templ, templ, nameToken.m_pos);
	setBody(templ, bodyTokenList);
	return true;
}

bool
Parser::declareTypedef(
	Declarator* declarator,
	Type* type
) {
	ASSERT(m_storageKind == StorageKind_Typedef);

	if (!declarator->isSimple()) {
		err::setError("invalid typedef declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	const sl::StringRef& name = declarator->getSimpleName();
	FindModuleItemResult findResult = nspace->findDirectChildItem(name);

	if (!findResult.m_result)
		return false;

	if (findResult.m_item) {
		ModuleItem* prevItem = findResult.m_item;

		if (prevItem->getItemKind() != ModuleItemKind_Typedef ||
			!((Typedef*)prevItem)->getType()->isEqual(type)
		) {
			setRedefinitionError(name);
			return false;
		}

		m_lastDeclaredItem = prevItem;
		popAttributeBlock();
		m_doxyParser.popBlock();
		return true;
	}

	Typedef* tdef = m_module->m_typeMgr.createTypedef(name, type);
	assignDeclarationAttributes(tdef, tdef, declarator);
	return nspace->addItem(name, tdef);
}

bool
Parser::declareAlias(
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
) {
	bool result;

	if (!declarator->m_constructor.isEmpty()) {
		err::setError("alias cannot have constructor");
		return false;
	}

	if (declarator->m_initializer.isEmpty()) {
		err::setError("missing alias initializer");
		return false;
	}

	if (!declarator->isSimple()) {
		err::setError("invalid alias declarator");
		return false;
	}

	if (type->getTypeKind() != TypeKind_Void) {
		err::setError("alias doesn't need a type");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	const sl::StringRef& name = declarator->getSimpleName();
	Alias* alias = m_module->m_namespaceMgr.createAlias(name, &declarator->m_initializer);
	assignDeclarationAttributes(alias, alias, declarator);

	if (nspace->getNamespaceKind() == NamespaceKind_Property) {
		Property* prop = (Property*)nspace;

		if (ptrTypeFlags & PtrTypeFlag_Bindable) {
			result = prop->setOnChanged(alias);
			if (!result)
				return false;
		} else if (ptrTypeFlags & PtrTypeFlag_AutoGet) {
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
	FunctionType* type,
	uint_t thisArgTypeFlags
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	DeclaratorKind declaratorKind = declarator->getDeclaratorKind();
	FunctionKind functionKind = declarator->getFunctionKind();
	bool hasArgs = !type->getArgArray().isEmpty();

	if (declaratorKind == DeclaratorKind_UnaryBinaryOperator) {
		ASSERT(functionKind == FunctionKind_UnaryOperator || functionKind == FunctionKind_BinaryOperator);
		functionKind = hasArgs ? FunctionKind_BinaryOperator : FunctionKind_UnaryOperator;
	}

	ASSERT(functionKind);
	uint_t functionKindFlags = getFunctionKindFlags(functionKind);

	if ((functionKindFlags & FunctionKindFlag_NoStorage) && m_storageKind) {
		err::setFormatStringError("'%s' cannot have storage specifier", getFunctionKindString(functionKind));
		return false;
	}

	if ((functionKindFlags & FunctionKindFlag_NoArgs) && hasArgs) {
		err::setFormatStringError("'%s' cannot have arguments", getFunctionKindString(functionKind));
		return false;
	}

	if (!m_storageKind) {
		m_storageKind =
			functionKind == FunctionKind_StaticConstructor ? StorageKind_Static :
			namespaceKind == NamespaceKind_Property ? ((Property*)nspace)->getStorageKind() : StorageKind_Undefined;
	}

	if (namespaceKind == NamespaceKind_PropertyTemplate) {
		if (m_storageKind) {
			err::setFormatStringError("invalid storage '%s' in property template", getStorageKindString(m_storageKind));
			return false;
		}

		if (thisArgTypeFlags) {
			err::setFormatStringError("unused modifiers '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
			return false;
		}

		bool result = ((PropertyTemplate*)nspace)->addMethod(functionKind, type);
		if (!result)
			return false;

		m_lastDeclaredItem = type;
		return true;
	}

	if (declarator->isQualified()) {
		if (namespaceKind == NamespaceKind_DynamicLib) {
			err::setFormatStringError("illegal orphan in dynamiclib '%s'", nspace->getDeclItem()->getItemName().sz());
			return false;
		}

		createOrphan(OrphanKind_Function, functionKind, declarator, type);
		return true;
	}

	Function* function;

	if (type->getFlags() & FunctionTypeFlag_Async)
		function = m_module->m_functionMgr.createFunction<AsyncLauncherFunction>(sl::StringRef(), type);
	else {
		function = m_module->m_functionMgr.createFunction(type);
		function->m_functionKind = functionKind;
	}

	switch (functionKind) {
	case FunctionKind_Normal:
		function->m_name = declarator->getSimpleName();
		break;

	case FunctionKind_UnaryOperator:
		function->m_unOpKind = declarator->getUnOpKind();
		break;

	case FunctionKind_BinaryOperator:
		function->m_binOpKind = declarator->getBinOpKind();
		break;
	}

	function->m_thisArgTypeFlags = thisArgTypeFlags;

	if (!declarator->m_initializer.isEmpty())
		sl::takeOver(&function->m_initializer, &declarator->m_initializer);

	assignDeclarationAttributes(function, function, declarator);

	if (function->m_storageKind == StorageKind_Static && thisArgTypeFlags) {
		err::setFormatStringError("static method cannot be '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
		return false;
	}

	switch (namespaceKind) {
	case NamespaceKind_Extension:
		return ((ExtensionNamespace*)nspace)->addMethod(function);

	case NamespaceKind_Type:
		switch (TypeKind typeKind = ((NamedType*)nspace)->getTypeKind()) {
		case TypeKind_Struct:
			return ((StructType*)nspace)->addMethod(function);

		case TypeKind_Union:
			return ((UnionType*)nspace)->addMethod(function);

		case TypeKind_Class:
			if (thisArgTypeFlags & PtrTypeFlag_ThinThis) {
				err::setError("'this' of class methods is already 'thin'");
				return false;
			}

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
		if (thisArgTypeFlags) {
			err::setFormatStringError("unused modifier '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
			return false;
		}

		if (!m_storageKind)
			function->m_storageKind = StorageKind_Static;
		else if (m_storageKind != StorageKind_Static) {
			err::setFormatStringError("invalid storage specifier '%s' for a global function", getStorageKindString(m_storageKind));
			return false;
		}
	}

	if (!nspace->getParentNamespace()) // module constructor / destructor
		switch (functionKind) {
		case FunctionKind_Constructor:
		case FunctionKind_StaticConstructor:
			return m_module->m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_Constructor, function);

		case FunctionKind_Destructor:
			return m_module->m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_Destructor, function);
		}

	if (functionKind != FunctionKind_Normal) {
		err::setFormatStringError(
			"invalid '%s' at '%s' namespace",
			getFunctionKindString(functionKind),
			getNamespaceKindString(namespaceKind)
		);
		return false;
	}

	size_t result = nspace->addFunction(function);
	return result != -1 || m_module->m_codeAssistMgr.getCodeAssistKind(); // when doing code-assist, ignore redefinition errors
}

FunctionType*
Parser::calcPropertyGetterType(Declarator* declarator) {
	uint_t typeModifiers = declarator->getTypeModifiers();
	ASSERT(typeModifiers & TypeModifier_Property);

	typeModifiers &= ~(
		TypeModifier_Property |
		TypeModifier_ErrorCode |
		TypeModifier_Const |
		TypeModifier_ReadOnly |
		TypeModifier_CMut |
		TypeModifier_AutoGet |
		TypeModifier_Bindable |
		TypeModifier_Indexed |
		TypeModifier_BigEndian |
		TypeModifier_Volatile
	);

	declarator->addGetterSuffix();

	DeclTypeCalc typeCalc;

	Type* type = typeCalc.calcType(
		declarator->getBaseType(),
		typeModifiers,
		declarator->getPointerPrefixList(),
		declarator->getSuffixList(),
		NULL,
		NULL
	);

	if (!type)
		return NULL;

	ASSERT(type->getTypeKind() == TypeKind_Function);
	return (FunctionType*)type;
}

bool
Parser::declareProperty(
	Declarator* declarator,
	PropertyType* type,
	uint_t flags
) {
	if (!declarator->isSimple()) {
		err::setError("invalid property declarator");
		return false;
	}

	Property* prop = createProperty(declarator);
	if (!prop)
		return false;

	if (type) {
		prop->m_flags |= flags;

		if (prop->m_storageKind != StorageKind_Reactor)
			return prop->create(type);

		sl::ConstIterator<Variable> lastVariableIt = m_module->m_variableMgr.getVariableList().getTail();
		bool result =
			prop->create(type) &&
			m_module->m_variableMgr.allocateNamespaceVariables(lastVariableIt);

		if (!result)
			return false;

		if (declarator->m_initializer.isEmpty())
			return true;

		Value propValue = prop;
		result = m_module->m_operatorMgr.createMemberClosure(&propValue);
		ASSERT(result);

		return m_module->m_operatorMgr.parseReactiveInitializer(propValue, &declarator->m_initializer);
	}

	m_lastPropertyTypeModifiers = declarator->getTypeModifiers();
	if (m_lastPropertyTypeModifiers & TypeModifier_Const)
		prop->m_flags |= PropertyFlag_Const;

	if (declarator->getBaseType()->getTypeKind() != TypeKind_Void ||
		!declarator->getPointerPrefixList().isEmpty() ||
		!declarator->getSuffixList().isEmpty()
	) {
		m_lastPropertyGetterType = calcPropertyGetterType(declarator);
		if (!m_lastPropertyGetterType)
			return false;
	} else {
		m_lastPropertyGetterType = NULL;
	}

	return true;
}

PropertyTemplate*
Parser::createPropertyTemplate() {
	PropertyTemplate* propertyTemplate = m_module->m_functionMgr.createPropertyTemplate();
	uint_t modifiers = getTypeSpecifier()->clearTypeModifiers(TypeModifier_Property | TypeModifier_Bindable);

	if (modifiers & TypeModifier_Bindable)
		propertyTemplate->m_typeFlags = PropertyTypeFlag_Bindable;

	return propertyTemplate;
}

Property*
Parser::createProperty(Declarator* declarator) {
	ASSERT(declarator->isSimple());

	bool result;

	m_lastDeclaredItem = NULL;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	if (namespaceKind == NamespaceKind_PropertyTemplate) {
		err::setError("property templates cannot have property members");
		return NULL;
	}

	Property* prop = m_module->m_functionMgr.createProperty(declarator->getSimpleName());
	assignDeclarationAttributes(prop, prop, declarator);

	TypeKind typeKind;
	switch (namespaceKind) {
	case NamespaceKind_Extension:
		result = ((ExtensionNamespace*)nspace)->addProperty(prop);
		if (!result)
			return NULL;

		break;

	case NamespaceKind_Type:
		typeKind = ((NamedType*)nspace)->getTypeKind();
		switch (typeKind) {
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
			err::setFormatStringError("property members are not allowed in '%s'", ((NamedType*)nspace)->getTypeString().sz());
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
		result = nspace->addItem(prop);
		if (!result)
			return NULL;

		if (m_storageKind && m_storageKind != StorageKind_Static) {
			err::setFormatStringError(
				"invalid storage specifier '%s' for property '%s'",
				getStorageKindString(m_storageKind),
				prop->getItemName().sz()
			);
			return NULL;
		}

		if (!m_module->m_controlFlowMgr.isReactor() || m_storageKind == StorageKind_Static)
			prop->m_storageKind = StorageKind_Static;
		else {
			prop->m_storageKind = StorageKind_Reactor;
			prop->m_parentType = m_module->m_controlFlowMgr.getReactorType();
		}
	}

	return prop;
}

bool
Parser::parseLastPropertyBody(const Token& bodyToken) {
	size_t length = bodyToken.m_data.m_string.getLength();
	sl::List<Token> tokenList;

	if (m_lastPropertyGetterType) { // simple property declaration syntax
		Property* prop = ((Property*)m_lastDeclaredItem);
		if (m_lastPropertyTypeModifiers & TypeModifier_Const) // const property with a body -- the body is the getter
				return
					finalizeLastProperty(true) &&
					setBody(prop->m_getter, bodyToken);

		err::setFormatStringError("simple read-write property '%s' can't have a body", prop->getItemName().sz());
		return false;
	}

	return
		tokenizeBody(
			&tokenList,
			lex::LineColOffset(
				bodyToken.m_pos.m_line,
				bodyToken.m_pos.m_col + 1,
				bodyToken.m_pos.m_offset + 1
			),
			bodyToken.m_data.m_string.getSubString(1, length - 2)
		) &&
		parseLastPropertyBody(&tokenList);
}

bool
Parser::parseLastPropertyBody(sl::List<Token>* body) {
	ASSERT(m_lastDeclaredItem && m_lastDeclaredItem->getItemKind() == ModuleItemKind_Property);

	if (body->isEmpty())
		return finalizeLastProperty(true);

	Parser parser(m_module, getPragmaConfigSnapshot(), Mode_Parse);
	m_module->m_namespaceMgr.openNamespace((Property*)m_lastDeclaredItem);
	bool result = parser.parseTokenList(SymbolKind_member_block_declaration_list, body);
	m_module->m_namespaceMgr.closeNamespace();
	return result && finalizeLastProperty(true);
}

bool
Parser::finalizeLastProperty(bool hasBody) {
	ASSERT(m_lastDeclaredItem && m_lastDeclaredItem->getItemKind() == ModuleItemKind_Property);

	bool result;

	Property* prop = (Property*)m_lastDeclaredItem;
	if (prop->getType())
		return true;

	// finalize getter

	if (prop->m_getter) {
		if (m_lastPropertyGetterType && !m_lastPropertyGetterType->isEqual(prop->m_getter->getType())) {
			err::setFormatStringError("getter type '%s' does not match property declaration", prop->m_getter->getType ()->getTypeString().sz());
			return false;
		}
	} else if (prop->m_autoGetValue) {
		ASSERT(prop->m_autoGetValue->getItemKind() == ModuleItemKind_Alias); // otherwise, getter would have been created
	} else {
		if (!m_lastPropertyGetterType) {
			err::setError("incomplete property: no 'get' method or 'autoget' field");
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

	if (!(m_lastPropertyTypeModifiers & TypeModifier_Const) && !hasBody) {
		FunctionType* getterType = prop->m_getter->getType()->getShortType();
		sl::Array<FunctionArg*> argArray = getterType->getArgArray();

		Type* setterArgType = getterType->getReturnType();
		argArray.append(setterArgType->getSimpleFunctionArg());

		Type* returnType;
		uint_t typeFlags;

		if (m_lastPropertyTypeModifiers & TypeModifier_ErrorCode) {
			returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
			typeFlags = FunctionTypeFlag_ErrorCode;
		} else {
			returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
			typeFlags = 0;
		}

		FunctionType* setterType = m_module->m_typeMgr.getFunctionType(returnType, argArray, typeFlags);
		Function* setter = m_module->m_functionMgr.createFunction(setterType);
		setter->m_functionKind = FunctionKind_Setter;
		setter->m_flags |= ModuleItemFlag_User;

		result = prop->addMethod(setter);
		if (!result)
			return false;
	}

	// finalize binder

	if (m_lastPropertyTypeModifiers & TypeModifier_Bindable) {
		if (!prop->m_onChanged) {
			result = prop->createOnChanged();
			if (!result)
				return false;
		}
	}

	// finalize auto-get value

	uint_t autogetValuePtrModifiers = m_lastPropertyTypeModifiers & (TypeModifier_BigEndian | TypeModifier_Volatile);
	if (m_lastPropertyTypeModifiers & TypeModifier_AutoGet) {
		if (!prop->m_autoGetValue) {
			result = prop->createAutoGetValue(prop->m_getter->getType()->getReturnType(), autogetValuePtrModifiers);
			if (!result)
				return false;
		}
	} else if (autogetValuePtrModifiers) {
		err::setFormatStringError("unused modifier '%s'", getTypeModifierString(m_lastPropertyTypeModifiers & autogetValuePtrModifiers).sz());
		return false;
	}

	if (prop->m_getter)
		prop->createType();

	return true;
}

bool
Parser::declareReactor(
	Declarator* declarator,
	uint_t ptrTypeFlags
) {
	if (declarator->getDeclaratorKind() != DeclaratorKind_Name) {
		err::setError("invalid reactor declarator");
		return false;
	}

	if (declarator->isQualified()) {
		createOrphan(OrphanKind_Reactor, FunctionKind_Normal, declarator, NULL);
		return true;
	}

	ASSERT(declarator->isSimple());

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();
	NamedType* parentType = NULL;

	switch (namespaceKind) {
	case NamespaceKind_Property:
		parentType = ((Property*)nspace)->getParentType();
		break;

	case NamespaceKind_Type:
		parentType = (NamedType*)nspace;
		break;
	}

	if (parentType && parentType->getTypeKind() != TypeKind_Class) {
		err::setFormatStringError("'%s' cannot contain reactor members", parentType->getTypeString().sz());
		return false;
	}

	ReactorClassType* type = m_module->m_typeMgr.createReactorType(declarator->getSimpleName(), (ClassType*)parentType);
	assignDeclarationAttributes(type, type, declarator);
	return declareData(declarator, type, ptrTypeFlags);
}

bool
Parser::declareData(
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
) {
	bool result;

	if (!declarator->isSimple()) {
		err::setError("invalid data declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	switch (namespaceKind) {
	case NamespaceKind_PropertyTemplate:
	case NamespaceKind_Extension:
		err::setFormatStringError("'%s' cannot have data fields", getNamespaceKindString(namespaceKind));
		return false;
	}

	const sl::StringRef& name = declarator->getSimpleName();
	size_t bitCount = declarator->getBitCount();
	sl::List<Token>* constructor = &declarator->m_constructor;
	sl::List<Token>* initializer = &declarator->m_initializer;

	if (isAutoSizeArrayType(type)) {
		if (initializer->isEmpty()) {
			err::setFormatStringError("auto-size array '%s' must be initialized", type->getTypeString().sz());
			return false;
		}

		ArrayType* arrayType = (ArrayType*)type;
		arrayType->m_elementCount = m_module->m_operatorMgr.getAutoSizeArrayElementCount(arrayType, *initializer);
		if (arrayType->m_elementCount == -1)
			return false;

		if (m_mode == Mode_Compile) {
			result = arrayType->ensureLayout();
			if (!result)
				return false;
		}
	} else if (
		(type->getTypeKindFlags() & TypeKindFlag_Ptr) &&
		(type->getFlags() & PtrTypeFlag_Safe) &&
		initializer->isEmpty()
	) {
		err::setFormatStringError("safe pointer '%s' must be initialized", type->getTypeString().sz());
		return false;
	}

	bool isDisposable = false;

	if (namespaceKind != NamespaceKind_Property && (ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable))) {
		err::setFormatStringError("'%s' can only be used on property field", getPtrTypeFlagString(ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable)).sz());
		return false;
	}

	if (m_module->m_controlFlowMgr.isReactor())
		return declareReactorVariable(declarator, type, ptrTypeFlags);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	StorageKind storageKind = m_storageKind;
	switch (storageKind) {
	case StorageKind_Undefined:
		switch (namespaceKind) {
		case NamespaceKind_Scope:
			type = type->getActualTypeIfImport(); // get rid of imports at this stage
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
		if (!scope && (!constructor->isEmpty() || !initializer->isEmpty())) {
			err::setError("global 'threadlocal' variables cannot have initializers");
			return false;
		}

		break;

	case StorageKind_Mutable:
		switch (namespaceKind) {
		case NamespaceKind_Type:
			break;

		case NamespaceKind_Property:
			if (((Property*)nspace)->getParentType())
				break;

		default:
			err::setError("'mutable' can only be applied to member fields");
			return false;
		}

		break;

	case StorageKind_Disposable:
		if (namespaceKind != NamespaceKind_Scope) {
			err::setError("'disposable' can only be applied to local variables");
			return false;
		}

		if (!isDisposableType(type)) {
			err::setFormatStringError("'%s' is not a disposable type", type->getTypeString().sz());
			return false;
		}

		ASSERT(scope);
		if (!(scope->getFlags() & ScopeFlag_Disposable)) {
			scope = m_module->m_namespaceMgr.openScope(
				declarator->getPos(),
				ScopeFlag_Disposable | ScopeFlag_FinallyAhead | ScopeFlag_Finalizable
			);

			if (!scope)
				return false;
		}

		storageKind = (type->getFlags() & TypeFlag_NoStack) ? StorageKind_Heap : StorageKind_Stack;
		m_storageKind = StorageKind_Undefined; // don't overwrite
		isDisposable = true;
		break;

	case StorageKind_DynamicField: {
		DynamicLayoutStmt* stmt = findDynamicLayoutStmt();
		if (!stmt) {
			err::setError("dynamic fields are only allowed inside dynamic layouts");
			return false;
		}

		if (!constructor->isEmpty() || !initializer->isEmpty()) {
			err::setError("dynamic fields can't have initializers");
			return false;
		}

		sl::BoxList<Value> dynamicAttributeArgList;

		if (declarator->m_attributeBlock || (declarator->m_attributeBlock = popAttributeBlock())) {
			result = declarator->m_attributeBlock->prepareAttributeValues(true);
			if (!result)
				return false;

			if (declarator->m_attributeBlock->m_flags & AttributeBlockFlag_DynamicValues) {
				result = prepareDynamicAttributeArgs(&dynamicAttributeArgList, declarator->m_attributeBlock) != -1;
				if (!result)
					return false;
			}
		}

		bool isAsync = m_module->m_functionMgr.getCurrentFunction()->getFunctionKind() == FunctionKind_AsyncSequencer;

		if (type->getTypeKind() == TypeKind_Array) {
			ASSERT(type->getFlags() & ModuleItemFlag_User);
			Parser parser(m_module, NULL, Parser::Mode_Compile);
			UserArrayType* arrayType = (UserArrayType*)type;
			Value countValue;

			result =
				parser.parseTokenList(SymbolKind_expression_save, &arrayType->m_initializer) &&
				m_module->m_operatorMgr.castOperator(parser.m_lastExpressionValue, TypeKind_SizeT, &countValue);

			if (!result)
				return false;

			if (parser.m_lastExpressionValue.getValueKind() == ValueKind_Const) {
				arrayType->m_elementCount = countValue.getSizeT();
				result = arrayType->ensureLayout();
				if (!result)
					return false;
			} else {
				Type* elementType = arrayType->getElementType();
				result =
					finalizeDynamicStructSection(stmt) &&
					elementType->ensureLayout();

				if (!result)
					return false;

				if (!(elementType->getFlags() & TypeFlag_Pod)) {
					err::setFormatStringError("non-POD '%s' cannot be used in a dynamic layout", elementType->getTypeString().sz());
					return false;
				}

				m_storageKind = StorageKind_Undefined;

				DynamicDataSection* field = m_module->m_dynamicLayoutMgr.createDataSection(
					DynamicSectionKind_Array,
					name,
					elementType,
					ptrTypeFlags
				);

				assignDeclarationAttributes(field, field, declarator);

				Value funcValue;
				Value declValue((int64_t)(ModuleItemDecl*)field, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
				Value typeValue((int64_t)elementType, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
				Value ptrTypeFlagsValue(ptrTypeFlags, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int_u));
				Value isAsyncValue(isAsync, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool));
				Value offsetValue;

				m_module->disableAccessChecks();

				sl::BoxList<Value> argValueList;
				argValueList.insertTail(declValue);
				argValueList.insertTail(typeValue);
				argValueList.insertTail(countValue);
				argValueList.insertTail(ptrTypeFlagsValue);
				argValueList.insertTail(isAsyncValue);

				result =
					nspace->addItem(field) &&
					m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "addArray", &funcValue) &&
					m_module->m_operatorMgr.callOperator(funcValue, &argValueList, &offsetValue) &&	(
						dynamicAttributeArgList.isEmpty() ||
						m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "setDynamicAttributes", &funcValue) &&
						m_module->m_operatorMgr.callOperator(funcValue, &dynamicAttributeArgList)
					) && (
						!isAsync ||
						m_module->m_operatorMgr.awaitDynamicLayout(stmt->m_layoutValue)
					);

				if (!result)
					return false;

				m_module->enableAccessChecks();
				field->m_offsetValue = offsetValue;
				return result;
			}
		}

		if (!(type->getFlags() & TypeFlag_Pod)) {
			err::setFormatStringError("non-POD '%s' cannot be used in a dynamic layout", type->getTypeString().sz());
			return false;
		}

		if (bitCount || !dynamicAttributeArgList.isEmpty()) {
			result = finalizeDynamicStructSection(stmt);
			if (!result)
				return false;

			DynamicDataSection* field = m_module->m_dynamicLayoutMgr.createDataSection(
				DynamicSectionKind_Field,
				name,
				type,
				ptrTypeFlags
			);

			assignDeclarationAttributes(field, field, declarator);

			Value funcValue;
			Value declValue((int64_t)(ModuleItemDecl*)field, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
			Value typeValue((int64_t)type, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
			Value ptrTypeFlagsValue(ptrTypeFlags, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int_u));
			Value isAsyncValue(isAsync, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool));
			Value offsetValue;
			Value bitOffsetValue;

			sl::BoxList<Value> argValueList;
			argValueList.insertTail(declValue);
			argValueList.insertTail(typeValue);

			sl::StringRef addMethodName;

			if (!bitCount)
				addMethodName = "addField";
			else {
				addMethodName = "addBitField";
				Value bitCountValue(bitCount, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int_u));
				argValueList.insertTail(bitCountValue);
			}

			argValueList.insertTail(ptrTypeFlagsValue);
			argValueList.insertTail(isAsyncValue);

			m_module->disableAccessChecks();

			result =
				nspace->addItem(field) &&
				m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, addMethodName, &funcValue) &&
				m_module->m_operatorMgr.callOperator(funcValue, &argValueList, &offsetValue) && (
					dynamicAttributeArgList.isEmpty() ||
					m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "setDynamicAttributes", &funcValue) &&
					m_module->m_operatorMgr.callOperator(funcValue, &dynamicAttributeArgList)
				) && (
					!isAsync ||
					m_module->m_operatorMgr.awaitDynamicLayout(stmt->m_layoutValue)
				);

			m_module->enableAccessChecks();

			if (bitCount) {
				Value bitOffsetValue;
				Value shrValue((int64_t)8, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int_u));

				result =
					m_module->m_operatorMgr.castOperator(offsetValue, TypeKind_Byte, &bitOffsetValue) &&
					m_module->m_operatorMgr.binaryOperator(jnc_BinOpKind_Shr, &offsetValue, shrValue);

				field->m_bitCount = bitCount;
				field->m_bitOffsetValue = bitOffsetValue;
			}

			field->m_offsetValue = offsetValue;

			return result;
		}

		if (stmt->m_structType &&
#if (_JNC_DYLAYOUT_FINALIZE_STRUCT_SECTIONS_ON_CALLS)
			(
				stmt->m_structBlock != m_module->m_controlFlowMgr.getCurrentBlock() ||
				stmt->m_callCount != m_module->m_operatorMgr.getCallCount()
			)
#else
			stmt->m_structBlock != m_module->m_controlFlowMgr.getCurrentBlock()
#endif
		) {
			result = finalizeDynamicStructSection(stmt);
			if (!result)
				return false;
		}

		if (!stmt->m_structType) {
			sl::String name = m_module->createUniqueName("Section");
			StructType* structType = m_module->m_typeMgr.createInternalStructType(name, 1);

			Value funcValue;
			Value typeValue(&structType, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));
			Value isAsyncValue(isAsync, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool));
			Value offsetValue;

			m_module->disableAccessChecks();

			result =
				m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "addStruct", &funcValue) &&
				m_module->m_operatorMgr.callOperator(funcValue, typeValue, isAsyncValue, &offsetValue) && (
					!isAsync ||
					m_module->m_operatorMgr.awaitDynamicLayout(stmt->m_layoutValue)
				);

			m_module->enableAccessChecks();

			if (!result)
				return false;

			structType->m_dynamicStructSectionId = stmt->m_offsetValueArray.getCount();
			stmt->m_offsetValueArray.append(offsetValue);
			stmt->m_structType = structType;
			stmt->m_structBlock = m_module->m_controlFlowMgr.getCurrentBlock();
#if (_JNC_DYLAYOUT_FINALIZE_STRUCT_SECTIONS_ON_CALLS)
			stmt->m_callCount = m_module->m_operatorMgr.getCallCount();
#endif
		}

		// bitfields are handled separately (so that we can pack them dynamically)

		Field* field = stmt->m_structType->createField(name, type, 0, ptrTypeFlags);
		if (!field || !nspace->addItem(field))
			return false;

		assignDeclarationAttributes(field, field, declarator);
		field->m_parentNamespace = stmt->m_structType; // don't reparent
		return true;
		}

	default:
		err::setFormatStringError("invalid storage specifier '%s' for variable", getStorageKindString(storageKind));
		return false;
	}

	if (namespaceKind == NamespaceKind_Property) {
		Property* prop = (Property*)nspace;
		ModuleItem* dataItem = NULL;

		if (storageKind == StorageKind_Member) {
			Field* field = prop->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
			if (!field)
				return false;

			assignDeclarationAttributes(field, field, declarator);

			dataItem = field;
		} else {
			if (bitCount) {
				err::setError("bit fields are not applicable here");
				return false;
			}

			Variable* variable = m_module->m_variableMgr.createVariable(
				storageKind,
				name,
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

		if (ptrTypeFlags & PtrTypeFlag_Bindable) {
			result = prop->setOnChanged(dataItem);
			if (!result)
				return false;
		} else if (ptrTypeFlags & PtrTypeFlag_AutoGet) {
			result = prop->setAutoGetValue(dataItem);
			if (!result)
				return false;
		}

	} else if (storageKind != StorageKind_Member && storageKind != StorageKind_Mutable) {
		if (bitCount) {
			err::setError("bit fields are not applicable here");
			return false;
		}

		Variable* variable = m_module->m_variableMgr.createVariable(
			storageKind,
			name,
			type,
			ptrTypeFlags,
			constructor,
			initializer
		);

		assignDeclarationAttributes(variable, variable, declarator);

		result = nspace->addItem(variable);
		if (!result)
			return false;

		if (nspace->getNamespaceKind() == NamespaceKind_Type) {
			NamedType* namedType = (NamedType*)nspace;
			TypeKind namedTypeKind = namedType->getTypeKind();

			switch (namedTypeKind) {
			case TypeKind_Struct:
			case TypeKind_Union:
			case TypeKind_Class:
				((DerivableType*)namedType)->m_staticVariableArray.append(variable);
				break;

			default:
				err::setFormatStringError("field members are not allowed in '%s'", namedType->getTypeString().sz());
				return false;
			}
		} else if (scope) {
			result = m_module->m_variableMgr.allocateVariable(variable);
			if (!result)
				return false;

			if (isDisposable) {
				result = m_module->m_variableMgr.finalizeDisposableVariable(variable);
				if (!result)
					return false;
			}

			switch (storageKind) {
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

				m_module->m_controlFlowMgr.onceStmt_PostBody(&stmt);
				break;

			default:
				ASSERT(false);
			}
		}
	} else {
		ASSERT(nspace->getNamespaceKind() == NamespaceKind_Type);

		NamedType* namedType = (NamedType*)nspace;
		TypeKind namedTypeKind = namedType->getTypeKind();

		Field* field;

		switch (namedTypeKind) {
		case TypeKind_Struct:
		case TypeKind_Union:
		case TypeKind_Class:
			field = ((DerivableType*)namedType)->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
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

FunctionArg*
Parser::createFormalArg(
	DeclFunctionSuffix* argSuffix,
	Declarator* declarator
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	uint_t ptrTypeFlags = 0;
	Type* type;

	if (nspace->getNamespaceKind() == NamespaceKind_TemplateDeclaration) {
		type = m_module->m_typeMgr.createTemplateDeclType(declarator);
		declarator = ((TemplateDeclType*)type)->getDeclarator();
	} else {
		type = declarator->calcType(&ptrTypeFlags);
		if (!type)
			return NULL;
	}

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_Array:
		if (type->getFlags() & ArrayTypeFlag_AutoSize) {
			err::setFormatStringError(
				"function cannot accept auto-size array '%s' as an argument",
				type->getTypeString().sz()
			);
			return NULL;
		}

		break;

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

	if (m_storageKind) {
		err::setFormatStringError("invalid storage '%s' for argument", getStorageKindString(m_storageKind));
		return NULL;
	}

	m_storageKind = StorageKind_Stack;

	sl::String name;

	if (declarator->isSimple()) {
		name = declarator->getSimpleName();
	} else if (declarator->getDeclaratorKind() != DeclaratorKind_Undefined) {
		err::setError("invalid formal argument declarator");
		return NULL;
	}

	FunctionArg* arg = m_module->m_typeMgr.createFunctionArg(
		name,
		type,
		ptrTypeFlags,
		&declarator->m_initializer
	);

	assignDeclarationAttributes(arg, arg, declarator);
	argSuffix->m_argArray.append(arg);
	return arg;
}

EnumType*
Parser::createEnumType(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	Type* baseType,
	uint_t flags
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	EnumType* enumType = NULL;

	if (name.isEmpty()) {
		flags |= EnumTypeFlag_Exposed;
		enumType = m_module->m_typeMgr.createUnnamedEnumType(baseType, flags);
	} else {
		enumType = m_module->m_typeMgr.createEnumType(name, baseType, flags);
		if (!enumType)
			return NULL;

		bool result = nspace->addItem(enumType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(enumType, enumType, pos);
	return enumType;
}

EnumConst*
Parser::createEnumConst(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	sl::List<Token>* initializer
) {
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
	const lex::LineCol& pos,
	const sl::StringRef& name,
	const sl::Array<Type*>& baseTypeArray
) {
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	StructType* structType = m_module->m_typeMgr.createStructType(name, m_pragmaConfig.m_fieldAlignment);

	if (!name.isEmpty()) {
		result = nspace->addItem(structType);
		if (!result)
			return NULL;
	}

	size_t count = baseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = structType->addBaseType(baseTypeArray[i]) != NULL;
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(structType, structType, pos);
	return structType;
}

UnionType*
Parser::createUnionType(
	const lex::LineCol& pos,
	const sl::StringRef& name
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	UnionType* unionType = m_module->m_typeMgr.createUnionType(name, m_pragmaConfig.m_fieldAlignment);

	if (!name.isEmpty()) {
		bool result = nspace->addItem(unionType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes(unionType, unionType, pos);
	return unionType;
}

ClassType*
Parser::createClassType(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	const sl::Array<Type*>& baseTypeArray,
	uint_t flags
) {
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	ClassType* classType = m_module->m_typeMgr.createClassType(name, m_pragmaConfig.m_fieldAlignment, flags);

	size_t count = baseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		result = classType->addBaseType(baseTypeArray[i]) != NULL;
		if (!result)
			return NULL;
	}

	result = nspace->addItem(classType);
	if (!result)
		return NULL;

	assignDeclarationAttributes(classType, classType, pos);
	return classType;
}

DynamicLibClassType*
Parser::createDynamicLibType(
	const lex::LineCol& pos,
	const sl::StringRef& name
) {
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	DynamicLibClassType* classType = m_module->m_typeMgr.createClassType<DynamicLibClassType>(name);
	Type* baseType = m_module->m_typeMgr.getStdType(StdType_DynamicLib);

	result =
		classType->addBaseType(baseType) != NULL &&
		nspace->addItem(classType);

	if (!result)
		return NULL;

	assignDeclarationAttributes(classType, classType, pos);

	DynamicLibNamespace* dynamicLibNamespace = classType->createLibNamespace();
	dynamicLibNamespace->m_parentUnit = classType->getParentUnit();
	return classType;
}

bool
Parser::finalizeEnumType(EnumType* type) {
	m_lastDeclaredItem = type;

	if (m_mode == Mode_Compile) {
		bool result = type->ensureLayout();
		if (!result)
			return false;
	}

	return (type->getFlags() & EnumTypeFlag_Exposed) ?
		m_module->m_namespaceMgr.getCurrentNamespace()->exposeEnumConsts(type) :
		true;
}

bool
Parser::finalizeDerivableType(DerivableType* type) {
	m_lastDeclaredItem = type;

	if (m_mode == Mode_Compile) {
		bool result = type->ensureLayout();
		if (!result)
			return false;
	}

	if (!type->getName().isEmpty())
		return true;

	// unroll unnamed type

	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;

	Declarator declarator;
	declarator.m_declaratorKind = DeclaratorKind_Name;
	declarator.m_pos = type->getPos();
	return declareData(&declarator, type, 0);
}

bool
Parser::callBaseTypeMemberConstructor(
	const QualifiedName& name,
	sl::BoxList<Value>* argList
) {
	ASSERT(m_constructorType || m_constructorProperty);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	FindModuleItemResult findResult = function->getParentNamespace()->findItemTraverse(*function, name);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item) {
		err::setFormatStringError("name '%s' is not found", name.getFullName().sz());
		return false;
	}

	Type* type = NULL;
	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();

	switch (itemKind) {
	case ModuleItemKind_Type:
		return callBaseTypeConstructor((Type*)item, argList);

	case ModuleItemKind_Typedef:
		return callBaseTypeConstructor(((Typedef*)item)->getType(), argList);

	case ModuleItemKind_Property:
		err::setError("property construction is not yet implemented");
		return false;

	case ModuleItemKind_Field:
		return callFieldConstructor((Field*)item, argList);

	case ModuleItemKind_Variable:
		err::setError("static field construction is not yet implemented");
		return false;

	default:
		err::setFormatStringError("'%s' cannot be used in base-type-member construct list", name.getFullName().sz());
		return false;
	}
}

DerivableType*
Parser::findBaseType(size_t baseTypeIdx) {
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function); // should not be called at pass

	DerivableType* parentType = function->getParentType();
	if (!parentType || !parentType->ensureNoImports())
		return NULL;

	BaseTypeSlot* slot = parentType->getBaseTypeByIndex(baseTypeIdx);
	if (!slot)
		return NULL;

	return slot->getType();
}

DerivableType*
Parser::getBaseType(size_t baseTypeIdx) {
	DerivableType* type = findBaseType(baseTypeIdx);
	if (!type) {
		err::setFormatStringError("'basetype%d' is not found", baseTypeIdx + 1);
		return NULL;
	}

	return type;
}

bool
Parser::getBaseType(
	size_t baseTypeIdx,
	Value* resultValue
) {
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
) {
	ASSERT(m_constructorType || m_constructorProperty);

	if (m_constructorProperty) {
		err::setFormatStringError("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->getItemName().sz());
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
) {
	ASSERT(m_constructorType || m_constructorProperty);

	if (m_constructorProperty) {
		err::setFormatStringError("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->getItemName().sz());
		return false;
	}

	BaseTypeSlot* baseTypeSlot = m_constructorType->findBaseType(type);
	if (!baseTypeSlot) {
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
) {
	DerivableType* type = baseTypeSlot->getType();

	if (baseTypeSlot->m_flags & ModuleItemFlag_Constructed) {
		err::setFormatStringError("'%s' is already constructed", type->getTypeString().sz());
		return false;
	}

	OverloadableFunction constructor = type->getConstructor();
	if (!constructor) {
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
) {
	ASSERT(m_constructorType || m_constructorProperty);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	bool result;

	if (m_constructorProperty) {
		err::setError("property field construction is not yet implemented");
		return false;
	}

	if (field->getParentNamespace() != m_constructorType) {
		err::setFormatStringError(
			"'%s' is not an immediate field of '%s'",
			field->getName().sz(),
			m_constructorType->getTypeString().sz()
		);
		return false;
	}

	if (field->getFlags() & ModuleItemFlag_Constructed) {
		err::setFormatStringError("'%s' is already constructed", field->getName().sz());
		return false;
	}

	if (!(field->getType()->getTypeKindFlags() & TypeKindFlag_Derivable) ||
		!((DerivableType*)field->getType())->getConstructor()) {
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
Parser::finalizeBaseTypeMemberConstructBlock() {
	ASSERT(m_constructorType || m_constructorProperty);

	Function* constructor = m_module->m_functionMgr.getCurrentFunction();
	FunctionKind functionKind = constructor->getFunctionKind();

	ASSERT(functionKind == FunctionKind_Constructor || functionKind == FunctionKind_StaticConstructor);

	if (functionKind == FunctionKind_StaticConstructor) {
		MemberBlock* memberBlock = m_constructorProperty ?
			(MemberBlock*)m_constructorProperty :
			(MemberBlock*)m_constructorType;

		memberBlock->primeStaticVariables();

		return
			memberBlock->initializeStaticVariables() &&
			memberBlock->callPropertyStaticConstructors();
	} else {
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

ModuleItem*
Parser::lookupIdentifier(
	const Token& token,
	MemberCoord* coord
) {
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace();
	FindModuleItemResult findResult = nspace->findDirectChildItemTraverse(token.m_data.m_string, coord);
	if (!findResult.m_result)
		return NULL;

	if (!findResult.m_item) {
		err::setFormatStringError("undeclared identifier '%s'", token.m_data.m_string.sz());
		pushSrcPosError(token.m_pos);
		return NULL;
	}

	return findResult.m_item;
}

bool
Parser::lookupIdentifier(
	const Token& token,
	Value* value
) {
	bool result;

	MemberCoord coord;
	ModuleItem* item = lookupIdentifier(token, &coord);
	if (!item)
		return false;

	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Namespace:
		value->setNamespace((GlobalNamespace*)item);
		break;

	case ModuleItemKind_Template:
		value->setTemplate((Template*)item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*)item)->getType();
		// and fall through

	case ModuleItemKind_Type:
		if (((Type*)item)->getTypeKind() == TypeKind_String)
			item = m_module->m_typeMgr.getStdType(StdType_StringStruct);
		else if (!(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named)) {
			err::setFormatStringError("'%s' cannot be used as expression", ((Type*)item)->getTypeString().sz());
			return false;
		}

		value->setNamespace((NamedType*)item);
		break;

	case ModuleItemKind_Variable:
		value->setVariable((Variable*)item);
		break;

	case ModuleItemKind_Function:
		result = value->trySetFunction((Function*)item);
		if (!result)
			return false;

		if (((Function*)item)->isMember()) {
			result = m_module->m_operatorMgr.createMemberClosure(value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_FunctionOverload:
		value->setFunctionOverload((FunctionOverload*)item);
		if (((FunctionOverload*)item)->getFlags() & FunctionOverloadFlag_HasMembers) {
			result = m_module->m_operatorMgr.createMemberClosure(value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_Property:
		value->setProperty((Property*)item);
		if (((Property*)item)->isMember()) {
			result = m_module->m_operatorMgr.createMemberClosure(value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_EnumConst:
		result = value->trySetEnumConst((EnumConst*)item);
		if (!result)
			return false;
		break;

	case ModuleItemKind_Const:
		*value = ((Const*)item)->getValue();
		break;

	case ModuleItemKind_Field: {
		Field* field = (Field*)item;
		StorageKind storageKind = field->getStorageKind();
		if (storageKind != StorageKind_DynamicField) { // regular (or reactor) field
			Value thisValue;
			result = m_module->m_operatorMgr.getThisValue(&thisValue);
			if (!result)
				return false;

			result = m_module->m_operatorMgr.getField(thisValue, field, &coord, value);
			if (!result)
				return false;

			m_module->m_operatorMgr.finalizeDualType(
				thisValue,
				field,
				m_module->m_namespaceMgr.getCurrentNamespace(),
				value
			);

			break;
		}

		// field in a dynamic layout

		uint_t ptrTypeFlags = field->getPtrTypeFlags() | PtrTypeFlag_Const;
		ASSERT(!(ptrTypeFlags & PtrTypeFlag_BitField)); // bitfields produce dynamic sections

		DataPtrType* ptrType = m_module->m_typeMgr.getDataPtrType(
			field->getType(),
			TypeKind_DataRef,
			DataPtrTypeKind_Normal,
			ptrTypeFlags
		);

		if (m_module->m_controlFlowMgr.isEmissionLocked()) { // sizeof/countof/typeof
			value->setType(ptrType);
			break;
		}

		DynamicLayoutStmt* stmt = findDynamicLayoutStmt();
		ASSERT(stmt);

		StructType* structType = (StructType*)field->getParentNamespace();
		result = structType->ensureLayoutTo(field);
		if (!result)
			return false;

		ASSERT(structType->m_dynamicStructSectionId != -1);
		Value offsetValue = stmt->m_offsetValueArray[structType->m_dynamicStructSectionId];
		Value fieldOffsetValue(field->getOffset(), m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));
		Value ptrValue;

		result =
			m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "m_p", &ptrValue) &&
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Add, &offsetValue, fieldOffsetValue) &&
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Add, &ptrValue, offsetValue) &&
			m_module->m_operatorMgr.castOperator(
				ptrValue,
				field->getType()->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const),
				value
			);

		if (!result)
			return false;

		value->overrideType(ptrType);
		break;
		}

	case ModuleItemKind_DynamicSection: {
		DynamicSectionKind sectionKind = ((DynamicSection*)item)->getSectionKind();
		switch (sectionKind) {
		case DynamicSectionKind_Array:
		case DynamicSectionKind_Field:
			break;

		default:
			ASSERT(sectionKind == DynamicSectionKind_Group); // dylayout structs have no names
			err::setFormatStringError("dyfield '%s' cannot be used as expression", token.m_data.m_string.sz());
			return false;
		}

		DynamicDataSection* section = (DynamicDataSection*)item;
		DynamicLayoutStmt* stmt = findDynamicLayoutStmt();
		ASSERT(stmt);

		Type* ptrType = section->getType()->getDataPtrType(DataPtrTypeKind_Normal, PtrTypeFlag_Const);
		Value ptrValue;
		size_t bitCount = section->getBitCount();

		result =
			m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "m_p", &ptrValue) &&
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Add, &ptrValue, section->getOffsetValue()) &&
			m_module->m_operatorMgr.castOperator(ptrValue, ptrType, value) && (
				sectionKind == DynamicSectionKind_Array ||
				m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, value)
			) && (
				bitCount == 0 ||
				!m_module->hasCodeGen() ||
				m_module->m_operatorMgr.loadDataRef(value) &&
				m_module->m_operatorMgr.extractBitField(
					*value,
					section->getType(),
					section->getBitOffsetValue(),
					bitCount,
					value
				)
			);

		if (!result)
			return false;

		break;
		}

	default:
		err::setFormatStringError(
			"%s '%s' cannot be used as expression",
			getModuleItemKindString(item->getItemKind()),
			token.m_data.m_string.sz()
		);
		return false;
	};

	if (m_module->m_codeAssistMgr.getCodeAssistKind() == CodeAssistKind_QuickInfoTip &&
		(token.m_data.m_codeAssistFlags & TokenCodeAssistFlag_At)
	)
		m_module->m_codeAssistMgr.createQuickInfoTip(token.m_pos.m_offset, item);

	return true;
}

ModuleItem*
Parser::lookupMember(
	ModuleItem* item,
	const Token& token
) {
	Namespace* nspace = item->getNamespace();
	if (!nspace) {
		err::setFormatStringError("member operator cannot be applied to '%s'", item->getItemName().sz());
		return NULL;
	}

	FindModuleItemResult result = nspace->findDirectChildItemTraverse(
		token.m_data.m_string,
		NULL,
		TraverseFlag_NoParentNamespace |
		TraverseFlag_NoUsingNamespaces |
		TraverseFlag_NoExtensionNamespaces
	);

	if (!result.m_result)
		return NULL;

	if (!result.m_item) {
		err::setFormatStringError("'%s' is not a member of '%s'", token.m_data.m_string.sz(), item->getItemName().sz());
		return NULL;
	}

	return result.m_item;}

bool
Parser::prepareCurlyInitializerNamedItem(
	CurlyInitializer* initializer,
	const sl::StringRef& name
) {
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
Parser::prepareCurlyInitializerIndexedItem(CurlyInitializer* initializer) {
	if (initializer->m_index == -1) {
		err::setError("indexed-based initializer cannot be used after named-based initializer");
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
Parser::skipCurlyInitializerItem(CurlyInitializer* initializer) {
	if (initializer->m_index == -1)
		return true; // allow finishing comma(s)

	initializer->m_index++;
	return true;
}

bool
Parser::assignCurlyInitializerItem(
	CurlyInitializer* initializer,
	const Value& value
) {
	if (initializer->m_index == -1 ||
		value.getValueKind() != ValueKind_Const ||
		!isCharArrayType(value.getType()) ||
		!isCharArrayRefType(initializer->m_targetValue.getType())) {
		if (initializer->m_index != -1)
			initializer->m_index++;

		initializer->m_count++;
		return m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, initializer->m_memberValue, value);
	}

	ArrayType* srcType = (ArrayType*)value.getType();
	ArrayType* dstType = (ArrayType*)((DataPtrType*)initializer->m_targetValue.getType())->getTargetType();

	size_t length = srcType->getElementCount();

	if (dstType->getElementCount() < initializer->m_index + length) {
		err::setError("literal initializer is too big to fit inside the target array");
		return false;
	}

	initializer->m_index += length;
	initializer->m_count++;

	Value memberPtrValue;

	return
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Addr, initializer->m_memberValue, &memberPtrValue) &&
		m_module->m_operatorMgr.memCpy(memberPtrValue, value, length);
}

void
Parser::finalizeFmtMlLiteral(
	Literal* literal,
	const sl::StringRef& string,
	uint_t indent
) {
	if (!indent) {
		literal->m_binData.append(string.cp(), string.getLength());
		literal->m_isZeroTerminated = true;
		literal->m_fmtMlFirstIt = NULL;
		return;
	}

	// remove common indent and adjust fmt site offsets

	sl::Iterator<FmtSite> it = literal->m_fmtMlFirstIt;
	size_t srcOffset = literal->m_fmtMlOffset;
	size_t dstOffset = srcOffset;
	bool isFirstChunk = true;
	for (; it; it++) {
		ASSERT(srcOffset <= it->m_offset);
		size_t length = it->m_offset - srcOffset;
		sl::StringRef chunk(literal->m_binData + srcOffset, length);
		if (isFirstChunk) {
			chunk = chunk.getSubString(indent);
			isFirstChunk = false;
		}

		sl::String unindentedString = Lexer::unindentMlLiteral(chunk, indent);
		memcpy(literal->m_binData.p() + dstOffset, unindentedString.cp(), unindentedString.getLength());
		dstOffset += unindentedString.getLength();
		srcOffset += length;

		ASSERT(it->m_offset == srcOffset && dstOffset <= srcOffset);
		it->m_offset = dstOffset; // update
	}

	// unindent and append the final chunk

	sl::String unindentedString = Lexer::unindentMlLiteral(
		isFirstChunk ? string.getSubString(indent) : string,
		indent
	);

	literal->m_binData.setCount(dstOffset);
	literal->m_binData.append(unindentedString.cp(), unindentedString.getLength());
	literal->m_isZeroTerminated = true;
	literal->m_fmtMlFirstIt = NULL;
}

bool
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	const Value& value,
	const sl::StringRef& fmtSpecifierString,
	uint_t flags
) {
	FmtSite* site = literal->addFmtSite(string, flags);
	site->m_fmtSpecifierString = fmtSpecifierString;

	if (!(flags & FmtLiteralTokenFlag_Index)) {
		site->m_value = value;
		return true;
	}

	if (value.getValueKind() != ValueKind_Const ||
		!(value.getType()->getTypeKindFlags() & TypeKindFlag_Integer)) {
		err::setError("expression is not integer constant");
		return false;
	}

	site->m_index = 0;
	memcpy(&site->m_index, value.getConstData(), value.getType()->getSize());

	literal->m_fmtIndex = site->m_index;
	return true;
}

void
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	size_t index,
	uint_t flags
) {
	FmtSite* site = literal->addFmtSite(string, flags);
	site->m_index = index;
	literal->m_fmtIndex = index;
}

void
Parser::addFmtSite(
	Literal* literal,
	const sl::StringRef& string,
	const sl::StringRef& fmtSpecifierString,
	uint_t flags
) {
	FmtSite* site = literal->addFmtSite(string, flags);
	site->m_index = ++literal->m_fmtIndex;
	site->m_fmtSpecifierString = fmtSpecifierString;
}

bool
Parser::finalizeLiteral(
	Literal* literal,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	bool result;

	if (literal->m_fmtSiteList.isEmpty()) {
		if (literal->m_isZeroTerminated)
			literal->m_binData.append(0);

		resultValue->setCharArray(literal->m_binData, literal->m_binData.getCount(), m_module);
		return true;
	}

	char buffer[256];
	sl::Array<Value*> argValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	size_t argCount = 0;

	if (argValueList) {
		argCount = argValueList->getCount();
		argValueArray.setCount(argCount);
		sl::Array<Value*>::Rwi rwi = argValueArray;

		sl::BoxIterator<Value> it = argValueList->getHead();
		for (size_t i = 0; i < argCount; i++, it++) {
			ASSERT(it);
			rwi[i] = it.p();
		}
	}

	Type* fmtLiteralType = m_module->m_typeMgr.getStdType(StdType_FmtLiteral);
	Variable* fmtLiteralVariable = m_module->m_variableMgr.createSimpleStackVariable("fmtLiteral", fmtLiteralType);
	result = m_module->m_variableMgr.initializeVariable(fmtLiteralVariable);
	ASSERT(result);

	Value fmtLiteralValue = fmtLiteralVariable;

	size_t offset = 0;

	sl::BitMap argUsageMap;
	argUsageMap.setBitCount(argCount);

	sl::Iterator<FmtSite> siteIt = literal->m_fmtSiteList.getHead();
	for (; siteIt; siteIt++) {
		FmtSite* site = *siteIt;
		Value* value;

		if (site->m_index == -1) {
			value = &site->m_value;
		} else {
			size_t i = site->m_index - 1;
			if (i >= argCount) {
				err::setFormatStringError("formatting literal doesn't have argument %%%d", site->m_index);
				return false;
			}

			value = argValueArray[i];
			argUsageMap.setBit(i);
		}

		if (site->m_offset > offset) {
			size_t length = site->m_offset - offset;
			appendFmtLiteralRawData(
				fmtLiteralValue,
				literal->m_binData + offset,
				length
			);

			offset += length;
		}

		if (value->isEmpty()) {
			err::setError("formatting literals arguments cannot be skipped");
			return false;
		}

		result = appendFmtLiteralValue(fmtLiteralValue, *value, site->m_fmtSpecifierString);
		if (!result)
			return false;
	}

	size_t unusedArgIdx = argUsageMap.findZeroBit(0);
	if (unusedArgIdx < argCount) {
		err::setFormatStringError("formatting literal argument %%%d is not used", unusedArgIdx + 1);
		return false;
	}

	size_t endOffset = literal->m_binData.getCount();
	if (endOffset > offset) {
		size_t length = endOffset - offset;
		appendFmtLiteralRawData(
			fmtLiteralValue,
			literal->m_binData + offset,
			length
		);
	}

	DataPtrType* resultType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getDataPtrType(DataPtrTypeKind_Lean);

	if (!m_module->hasCodeGen()) {
		resultValue->setType(resultType);
		return true;
	}

	Value fatPtrValue;
	Value thinPtrValue;
	Value validatorValue;
	Type* fatPtrType = m_module->m_typeMgr.getStdType(StdType_DataPtrStruct);
	Type* validatorType = m_module->m_typeMgr.getStdType(StdType_DataPtrValidatorPtr);

	m_module->m_llvmIrBuilder.createGep2(fmtLiteralValue, fmtLiteralType, 0, NULL, &fatPtrValue);
	m_module->m_llvmIrBuilder.createLoad(fatPtrValue, fatPtrType, &fatPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue(fatPtrValue, 0, NULL, &thinPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue(fatPtrValue, 1, validatorType, &validatorValue);
	resultValue->setLeanDataPtr(thinPtrValue.getLlvmValue(), resultType, validatorValue);
	return true;
}

void
Parser::appendFmtLiteralRawData(
	const Value& fmtLiteralValue,
	const void* p,
	size_t length
) {
	if (!m_module->hasCodeGen())
		return;

	Function* append = m_module->m_functionMgr.getStdFunction(StdFunc_AppendFmtLiteral_a);

	Value literalValue;
	literalValue.setCharArray(p, length, m_module);
	bool result = m_module->m_operatorMgr.castOperator(&literalValue, m_module->m_typeMgr.getStdType(StdType_CharConstThinPtr));
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
) {
	if (fmtSpecifierString.isSuffix('B')) // binary format
		return appendFmtLiteralBinValue(fmtLiteralValue, rawSrcValue);

	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand(rawSrcValue, &srcValue, OpFlag_KeepDerivableRef);
	if (!result)
		return false;

	StdFunc appendFunc;

	Type* type = srcValue.getType();
	TypeKind typeKind = type->getTypeKind();
	uint_t typeKindFlags = type->getTypeKindFlags();

	if (typeKindFlags & TypeKindFlag_Integer) {
		static StdFunc funcTable[2][2] = {
			{ StdFunc_AppendFmtLiteral_i32, StdFunc_AppendFmtLiteral_ui32 },
			{ StdFunc_AppendFmtLiteral_i64, StdFunc_AppendFmtLiteral_ui64 },
		};

		size_t i1 = type->getSize() > 4;
		size_t i2 = (typeKindFlags & TypeKindFlag_Unsigned) != 0;

		appendFunc = funcTable[i1][i2];
	} else if (typeKindFlags & TypeKindFlag_Fp)
		appendFunc = StdFunc_AppendFmtLiteral_f;
	else if (typeKind == TypeKind_Variant)
		appendFunc = StdFunc_AppendFmtLiteral_v;
	else if (typeKind == TypeKind_String || isStringableType(type))
		appendFunc = StdFunc_AppendFmtLiteral_s;
	else if (isCharArrayType(type) || isCharArrayRefType(type) || isCharPtrType(type))
		appendFunc = StdFunc_AppendFmtLiteral_p;
	else {
		err::setFormatStringError("don't know how to format '%s'", type->getTypeString().sz());
		return false;
	}

	Function* append = m_module->m_functionMgr.getStdFunction(appendFunc);
	Type* argType = append->getType()->getArgArray()[2]->getType();

	Value argValue;
	result = m_module->m_operatorMgr.castOperator(srcValue, argType, &argValue);
	if (!result)
		return false;

	Value fmtSpecifierValue;
	if (!fmtSpecifierString.isEmpty()) {
		fmtSpecifierValue.setCharArray(fmtSpecifierString, m_module);
		m_module->m_operatorMgr.castOperator(&fmtSpecifierValue, m_module->m_typeMgr.getStdType(StdType_CharConstThinPtr));
	} else {
		fmtSpecifierValue = m_module->m_typeMgr.getStdType(StdType_CharConstThinPtr)->getZeroValue();
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
) {
	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand(rawSrcValue, &srcValue);
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	Type* type = srcValue.getType();
	Function* append = m_module->m_functionMgr.getStdFunction(StdFunc_AppendFmtLiteral_a);
	Type* argType = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);

	Value sizeValue(
		type->getSize(),
		m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)
	);

	Value tmpValue;
	Value resultValue;

	m_module->m_llvmIrBuilder.createAlloca(type, NULL, &tmpValue);
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
Parser::finalizeRegexSwitchCaseLiteral(
	sl::StringRef* data,
	const Value& value,
	bool isZeroTerminated
) {
	if (value.getValueKind() != ValueKind_Const) {
		err::setError("not a constant literal expression");
		return false;
	}

	size_t length = value.m_type->getSize();
	if (isZeroTerminated) {
		ASSERT(length);
		length--;
	}

	*data = sl::StringRef(value.m_constData.getHdr(), value.m_constData.cp(), length);
	return true;
}

bool
Parser::assertStmt(
	sl::List<Token>* conditionTokenList,
	const sl::StringRef& message
) {
	if (!(m_module->getCompileFlags() & ModuleCompileFlag_Assert))
		return true;

	lex::LineCol pos = conditionTokenList->getHead()->m_pos;
	const sl::StringRef conditionText = Token::getText(*conditionTokenList);

	Value conditionValue;
	bool result = m_module->m_operatorMgr.parseExpression(conditionTokenList, &conditionValue);
	if (!result)
		return false;

	if (!m_module->hasCodeGen())
		return true;

	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock("assert_fail");
	BasicBlock* continueBlock = m_module->m_controlFlowMgr.createBlock("assert_continue");

	result = m_module->m_controlFlowMgr.conditionalJump(conditionValue, continueBlock, failBlock, failBlock);
	if (!result)
		return false;

	Function* assertionFailure = m_module->m_functionMgr.getStdFunction(StdFunc_AssertionFailure);
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();

	sl::BoxList<Value> argValueList;
	argValueList.insertTail(m_module->m_variableMgr.getStaticLiteralVariable(unit->getFilePath()));
	argValueList.insertTail(Value(pos.m_line, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int)));
	argValueList.insertTail(m_module->m_variableMgr.getStaticLiteralVariable(conditionText));

	if (!message.isEmpty())
		argValueList.insertTail(m_module->m_variableMgr.getStaticLiteralVariable(message));
	else {
		Value nullValue;
		nullValue.setNull(m_module);
		argValueList.insertTail(nullValue);
	}

	result = m_module->m_operatorMgr.callOperator(assertionFailure, &argValueList);
	if (!result)
		return false;

	m_module->m_controlFlowMgr.follow(continueBlock);
	return true;
}

DynamicLayoutStmt*
Parser::initializeDynamicLayoutStmt(
	const Value& layoutValue0,
	const lex::LineCol& pos,
	uint_t flags
) {
	ClassType* layoutType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_DynamicLayout);

	Value layoutValue;

	bool result =
		m_module->ensureDynamicLayoutRequired() &&
		m_module->m_operatorMgr.castOperator(
			layoutValue0,
			layoutType->getClassPtrType(ClassPtrTypeKind_Normal, PtrTypeFlag_Safe),
			&layoutValue
		);

	if (!result)
		return NULL;

	DynamicLayoutStmt* stmt = m_module->m_namespaceMgr.createScopeExtension<DynamicLayoutStmt>();
	stmt->m_layoutValue = layoutValue;
	stmt->m_structType = NULL;
	stmt->m_structBlock = NULL;
#if (_JNC_DYLAYOUT_FINALIZE_STRUCT_SECTIONS_ON_CALLS)
	stmt->m_callCount = 0;
#endif

	Scope* scope = m_module->m_namespaceMgr.openScope(pos, flags);
	scope->m_dynamicLayoutStmt = stmt;
	return stmt;
}

bool
Parser::finalizeDynamicStructSection(DynamicLayoutStmt* stmt) {
	if (stmt->m_structType) {
		bool result = stmt->m_structType->ensureLayout();
		if (!result)
			return false;
	}

	stmt->m_structType = NULL;
	stmt->m_structBlock = NULL;
	return true;
}

bool
Parser::finalizeDynamicLayoutStmt(DynamicLayoutStmt* stmt) {
	if (!m_module->hasCodeGen()) {
		m_module->m_namespaceMgr.closeScope();
		return true;
	}

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (scope->m_dynamicLayoutStmt != stmt) {
		err::setError("invalid scope structure due to previous errors");
		return false;
	}

	bool result = finalizeDynamicStructSection(stmt); // finalize inside the scope
	m_module->m_namespaceMgr.closeScope();
	return result;
}

bool
Parser::openDynamicGroup(
	const lex::LineCol& pos,
	const sl::StringRef& name,
	uint_t scopeFlags
) {
	DynamicLayoutStmt* stmt = findDynamicLayoutStmt();
	if (!stmt) {
		err::setError("dynamic groups are only allowed inside dynamic layouts");
		return false;
	}

	bool result = finalizeDynamicStructSection(stmt);
	if (!result)
		return false;

	sl::BoxList<Value> dynamicAttributeArgList;
	AttributeBlock* attributeBlock = popAttributeBlock();
	if (attributeBlock) {
		result = attributeBlock->prepareAttributeValues(true);
		if (!result)
			return false;

		if (attributeBlock->m_flags & AttributeBlockFlag_DynamicValues) {
			result = prepareDynamicAttributeArgs(&dynamicAttributeArgList, attributeBlock) != -1;
			if (!result)
				return false;
		}
	}

	m_module->m_namespaceMgr.openScope(pos, scopeFlags | ScopeFlag_DynamicGroup);

	m_storageKind = StorageKind_Undefined;
	DynamicSection* group = m_module->m_dynamicLayoutMgr.createGroup(name);
	assignDeclarationAttributes(group, group, pos, attributeBlock);

	Value funcValue;
	Value declValue((int64_t)(ModuleItemDecl*)group, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr));

	m_module->disableAccessChecks();

	result =
		group->ensureAttributeValuesReady() &&
		m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "openGroup", &funcValue) &&
		m_module->m_operatorMgr.callOperator(funcValue, declValue) && (
			dynamicAttributeArgList.isEmpty() ||
			m_module->m_operatorMgr.memberOperator(stmt->m_layoutValue, "setDynamicAttributes", &funcValue) &&
			m_module->m_operatorMgr.callOperator(funcValue, &dynamicAttributeArgList)
		);

	m_module->enableAccessChecks();
	return result;
}

bool
Parser::closeDynamicGroup() {
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (!(scope->m_flags & ScopeFlag_DynamicGroup)) {
		err::setError("invalid scope structure due to previous errors");
		return false;
	}

	DynamicLayoutStmt* stmt = findDynamicLayoutStmt();
	ASSERT(stmt);

	bool result =
		finalizeDynamicStructSection(stmt) && // finalize inside the scope
		m_module->m_operatorMgr.closeDynamicGroup(stmt->m_layoutValue);

	m_module->m_namespaceMgr.closeScope();
	return result;
}

void
Parser::addScopeAnchorToken(
	StmtPass1* stmt,
	const Token& srcToken
) {
	Token* token = m_tokenPool->get(srcToken);
	token->m_data.m_integer = 0; // tokens can be reused, ensure 0
	stmt->m_tokenList.insertTail(token);
	stmt->m_scopeAnchorToken = token;
}

bool
Parser::finalizeOnEventStmt(
	const lex::LineCol& pos,
	DeclFunctionSuffix* functionSuffix,
	const sl::BoxList<Value>& valueList,
	sl::List<Token>* bodyTokenList
) {
	Function* handler = m_module->m_controlFlowMgr.createOnEventHandler(pos, functionSuffix->getArgArray());
	bool result = setBody(handler, bodyTokenList);
	ASSERT(result);

	result = m_module->m_controlFlowMgr.addOnEventBindings(handler, valueList);
	if (!result)
		handler->ensureSrcPosError();

	return result;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
Parser::createMemberCodeAssist(
	const Token& token,
	const Value& value
) {
	Namespace* nspace = m_module->m_operatorMgr.getValueNamespace(value);
	if (!nspace) {
		m_module->m_codeAssistMgr.createEmptyCodeAssist(token.m_pos.m_offset);
		return;
	}

	CodeAssistKind codeAssistKind = m_module->m_codeAssistMgr.getCodeAssistKind();
	if (codeAssistKind == CodeAssistKind_QuickInfoTip) {
		if (token.m_tokenKind == TokenKind_Identifier) {
			FindModuleItemResult result = nspace->findDirectChildItemTraverse(token.m_data.m_string, NULL, TraverseFlag_NoParentNamespace);
			if (result.m_item)
				m_module->m_codeAssistMgr.createQuickInfoTip(token.m_pos.m_offset, result.m_item);
		}
	} else if (codeAssistKind == CodeAssistKind_AutoComplete) {
		size_t offset = token.m_pos.m_offset;
		if (token.m_tokenKind != TokenKind_Identifier)
			if (token.m_data.m_codeAssistFlags & TokenCodeAssistFlag_Right)
				offset += token.m_pos.m_length;
			else
				return;

		m_module->m_codeAssistMgr.createAutoComplete(offset, nspace);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
