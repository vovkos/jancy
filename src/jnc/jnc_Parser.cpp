#include "pch.h"
#include "jnc_Parser.llk.h"
#include "jnc_Parser.llk.cpp"
#include "jnc_Closure.h"
#include "jnc_DeclTypeCalc.h"
#include "jnc_Recognizer.h"

namespace jnc {

//.............................................................................

Parser::Parser (Module* module)
{
	m_module = module;
	m_stage = StageKind_Pass1;
	m_flags = 0;
	m_fieldAlignment = 8;
	m_defaultFieldAlignment = 8;
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_attributeBlock = NULL;
	m_lastDeclaredItem = NULL;
	m_lastPropertyGetterType = NULL;
	m_reactorType = NULL;
	m_reactionBindSiteCount = 0;
	m_reactorTotalBindSiteCount = 0;
	m_reactionIndex = 0;
	m_automatonState = AutomatonState_Idle;
	m_automatonSwitchBlock = NULL;
	m_automatonReturnBlock = NULL;
	m_libraryFunctionCount = 0;
	m_constructorType = NULL;
	m_constructorProperty = NULL;
	m_namedType = NULL;
}

bool
Parser::parseTokenList (
	SymbolKind symbol,
	const rtl::ConstBoxList <Token>& tokenList,
	bool isBuildingAst
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	ASSERT (!tokenList.isEmpty ());

	bool result;

	create (symbol, isBuildingAst);

	rtl::BoxIterator <Token> token = tokenList.getHead ();
	for (; token; token++)
	{
		result = parseToken (&*token);
		if (!result)
		{
			err::ensureSrcPosError (unit->getFilePath (), token->m_pos.m_line, token->m_pos.m_col);
			return false;
		}
	}

	// important: process EOF token, it might actually trigger actions!

	Token::Pos pos = tokenList.getTail ()->m_pos;

	Token eofToken;
	eofToken.m_token = 0;
	eofToken.m_pos = pos;
	eofToken.m_pos.m_p += pos.m_length;
	eofToken.m_pos.m_offset += pos.m_length;
	eofToken.m_pos.m_col += pos.m_length;
	eofToken.m_pos.m_length = 0;

	result = parseToken (&eofToken);
	if (!result)
	{
		err::ensureSrcPosError (unit->getFilePath (), eofToken.m_pos.m_line, eofToken.m_pos.m_col);
		return false;
	}

	return true;
}

bool
Parser::isTypeSpecified ()
{
	if (m_typeSpecifierStack.isEmpty ())
		return false;

	// if we've seen 'unsigned', assume 'int' is implied.
	// checking for 'property' is required for full property syntax e.g.:
	// property foo { int get (); }
	// here 'foo' should be a declarator, not import-type-specifier

	TypeSpecifier* typeSpecifier = m_typeSpecifierStack.getBack ();
	return
		typeSpecifier->getType () != NULL ||
		typeSpecifier->getTypeModifiers () & (TypeModifier_Unsigned | TypeModifier_Property);
}

NamedImportType*
Parser::getNamedImportType (
	const QualifiedName& name,
	const Token::Pos& pos
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	NamedImportType* type = m_module->m_typeMgr.getNamedImportType (name, nspace);

	if (!type->m_parentUnit)
	{
		type->m_parentUnit = m_module->m_unitMgr.getCurrentUnit ();
		type->m_pos = pos;
	}

	return type;
}

Type*
Parser::findType (
	size_t baseTypeIdx,
	const QualifiedName& name,
	const Token::Pos& pos
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();

	ModuleItem* item;

	if (m_stage == StageKind_Pass1)
	{
		if (baseTypeIdx != -1)
			return NULL;

		if (!name.isSimple ())
			return getNamedImportType (name, pos);

		rtl::String shortName = name.getShortName ();
		item = nspace->findItem (shortName);
		if (!item)
			return getNamedImportType (name, pos);
	}
	else
	{
		if (baseTypeIdx != -1)
		{
			DerivableType* baseType = findBaseType (baseTypeIdx);
			if (!baseType)
				return NULL;

			nspace = baseType;

			if (name.isEmpty ())
				return baseType;
		}

		item = nspace->findItemTraverse (name);
		if (!item)
			return NULL;
	}

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Type:
		return (Type*) item;

	case ModuleItemKind_Typedef:
		return ((Typedef*) item)->getType ();

	default:
		return NULL;
	}
}

Type*
Parser::getType (
	size_t baseTypeIdx,
	const QualifiedName& name,
	const Token::Pos& pos
	)
{
	Type* type = findType (baseTypeIdx, name, pos);
	if (!type)
	{
		if (baseTypeIdx == -1)
			err::setFormatStringError ("'%s' is not found or not a type", name.getFullName ().cc ());
		else if (name.isEmpty ())
			err::setFormatStringError ("'basetype%d' is not found", baseTypeIdx + 1);
		else
			err::setFormatStringError ("'basetype%d.%s' is not found or not a type", baseTypeIdx + 1, name.getFullName ().cc ());

		return NULL;
	}

	return type;
}

bool
Parser::setSetAsType (Type* type)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	if (nspace->getNamespaceKind () != NamespaceKind_Type)
	{
		err::setFormatStringError ("invalid setas in '%s'", nspace->getQualifiedName ().cc ());
		return false;
	}

	DerivableType* derivableType = (DerivableType*) (NamedType*) nspace;
	if (derivableType->m_setAsType)
	{
		err::setFormatStringError ("setas redefinition for '%s'", derivableType->getTypeString ().cc ());
		return false;
	}	

	derivableType->m_setAsType = type;
	
	if (type->getTypeKindFlags () & TypeKindFlag_Import)
		derivableType->m_setAsType_i = (ImportType*) type;

	return true;
}

void
Parser::preDeclaration ()
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;
	m_lastDeclaredItem = NULL;
}

bool
Parser::emptyDeclarationTerminator (TypeSpecifier* typeSpecifier)
{
	if (!m_lastDeclaredItem)
	{
		ASSERT (typeSpecifier);
		Type* type = typeSpecifier->getType ();
		if (!type || !(type->getFlags () & TypeFlag_Named))
		{
			err::setFormatStringError ("invalid declaration (no declarator, no named type)");
			return false;
		}

		if (typeSpecifier->getTypeModifiers ())
		{
			err::setFormatStringError ("unused modifier '%s'", getTypeModifierString (typeSpecifier->getTypeModifiers ()).cc ());
			return false;
		}

		return true;
	}

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Property:
		return finalizeLastProperty (false);

	case ModuleItemKind_Orphan:
		err::setFormatStringError ("orphan '%s' without a body", m_lastDeclaredItem->m_tag.cc ());
		return false;
	}

	return true;
}

bool
Parser::setFunctionBody (
	Function* function,
	rtl::BoxList <Token>* tokenList
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	if (nspace->getNamespaceKind () == NamespaceKind_Library)
	{
		err::setFormatStringError ("library function cannot have a body");
		return false;
	}

	bool result = function->setBody (tokenList);
	if (!result)
		return false;

	while (nspace)
	{
		function->m_usingSet.append (nspace->getUsingSet ());
		nspace = nspace->getParentNamespace ();
	}

	return true;
}

bool
Parser::setDeclarationBody (rtl::BoxList <Token>* tokenList)
{
	if (!m_lastDeclaredItem)
	{
		err::setFormatStringError ("declaration without declarator cannot have a body");
		return false;
	}

	Type* type;

	ModuleItemKind itemKind = m_lastDeclaredItem->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Function:
		return setFunctionBody ((Function*) m_lastDeclaredItem, tokenList);

	case ModuleItemKind_Property:
		return parseLastPropertyBody (*tokenList);

	case ModuleItemKind_Typedef:
		type = ((Typedef*) m_lastDeclaredItem)->getType ();
		break;

	case ModuleItemKind_Type:
		type = (Type*) m_lastDeclaredItem;
		break;

	case ModuleItemKind_Variable:
		type = ((Variable*) m_lastDeclaredItem)->getType ();
		break;

	case ModuleItemKind_StructField:
		type = ((StructField*) m_lastDeclaredItem)->getType ();
		break;

	case ModuleItemKind_Orphan:
		return ((Orphan*) m_lastDeclaredItem)->setBody (tokenList);

	default:
		err::setFormatStringError ("'%s' cannot have a body", getModuleItemKindString (m_lastDeclaredItem->getItemKind ()));
		return false;
	}

	if (!isClassType (type, ClassTypeKind_Reactor))
	{
		err::setFormatStringError ("only functions and reactors can have bodies, not '%s'", type->getTypeString ().cc ());
		return false;
	}

	return ((ReactorClassType*) type)->setBody (tokenList);
}

bool
Parser::setStorageKind (StorageKind storageKind)
{
	if (m_storageKind)
	{
		err::setFormatStringError (
			"more than one storage specifier specifiers ('%s' and '%s')",
			getStorageKindString (m_storageKind),
			getStorageKindString (storageKind)
			);
		return false;
	}

	m_storageKind = storageKind;
	return true;
}

bool
Parser::setAccessKind (AccessKind accessKind)
{
	if (m_accessKind)
	{
		err::setFormatStringError (
			"more than one access specifiers ('%s' and '%s')",
			getAccessKindString (m_accessKind),
			getAccessKindString (accessKind)
			);
		return false;
	}

	m_accessKind = accessKind;
	return true;
}

GlobalNamespace*
Parser::getGlobalNamespace (
	GlobalNamespace* parentNamespace,
	const rtl::String& name,
	const Token::Pos& pos
	)
{
	GlobalNamespace* nspace;

	ModuleItem* item = parentNamespace->findItem (name);
	if (!item)
	{
		nspace = m_module->m_namespaceMgr.createGlobalNamespace (name, parentNamespace);
		nspace->m_pos = pos;
		parentNamespace->addItem (nspace);
	}
	else
	{
		if (item->getItemKind () != ModuleItemKind_Namespace)
		{
			err::setFormatStringError ("'%s' exists and is not a namespace", parentNamespace->createQualifiedName (name).cc ());
			return NULL;
		}

		nspace = (GlobalNamespace*) item;
	}

	return nspace;
}

GlobalNamespace*
Parser::openGlobalNamespace (
	const QualifiedName& name,
	const Token::Pos& pos
	)
{
	Namespace* currentNamespace = m_module->m_namespaceMgr.getCurrentNamespace ();
	if (currentNamespace->getNamespaceKind () != NamespaceKind_Global)
	{
		err::setFormatStringError ("cannot open global namespace in '%s'", getNamespaceKindString (currentNamespace->getNamespaceKind ()));
		return NULL;
	}

	GlobalNamespace* nspace = getGlobalNamespace ((GlobalNamespace*) currentNamespace, name.getFirstName (), pos);
	if (!nspace)
		return NULL;

	if (nspace->getFlags () & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError ("cannot extend sealed namespace '%s'", nspace->getQualifiedName ().cc ());
		return NULL;
	}

	rtl::BoxIterator <rtl::String> it = name.getNameList ().getHead ();
	for (; it; it++)
	{
		nspace = getGlobalNamespace (nspace, *it, pos);
		if (!nspace)
			return NULL;
	}

	m_module->m_namespaceMgr.openNamespace (nspace);
	return nspace;
}

ExtensionNamespace*
Parser::openExtensionNamespace (
	const rtl::String& name,
	Type* type,
	const Token::Pos& pos
	)
{
	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
	case TypeKind_Class:
		break;

	case TypeKind_NamedImport:
		err::setFormatStringError ("type extensions for imports not supported yet");
		return NULL;

	default:
		err::setFormatStringError ("'%s' cannot have a type extension", type->getTypeString ().cc ());
		return NULL;
	}

	DerivableType* derivableType = (DerivableType*) type;

	Namespace* currentNamespace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ExtensionNamespace* extensionNamespace = m_module->m_namespaceMgr.createExtensionNamespace (
		name, 
		derivableType, 
		currentNamespace
		);

	extensionNamespace->m_pos = pos;
	derivableType->m_extensionNamespaceArray.append (extensionNamespace);

	bool result = currentNamespace->addItem (extensionNamespace);
	if (!result)
		return NULL;

	m_module->m_namespaceMgr.openNamespace (extensionNamespace);
	return extensionNamespace;
}

bool
Parser::useNamespace (
	const rtl::BoxList <QualifiedName>& nameList,
	NamespaceKind namespaceKind,
	const Token::Pos& pos
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	
	rtl::BoxIterator <QualifiedName> it = nameList.getHead ();
	for (; it; it++)
	{
		ModuleItem* item = nspace->findItemTraverse (*it);
		if (!item)
		{
			err::setFormatStringError ("undeclared identifier '%s'", it->getFullName ().cc ());
			return false;
		}

		if (item->getItemKind () != ModuleItemKind_Namespace)
		{
			err::setFormatStringError ("'%s' is not a namespace", it->getFullName ().cc ());
			return false;
		}
		
		GlobalNamespace* usedNamespace = (GlobalNamespace*) item;
		if (usedNamespace->getNamespaceKind () != namespaceKind)
		{
			err::setFormatStringError ("'%s' is not %s", it->getFullName ().cc (), getNamespaceKindString (namespaceKind));
			return false;
		}

		switch (namespaceKind)
		{
		case NamespaceKind_Global:
			nspace->m_usingSet.addGlobalNamespace (usedNamespace);
			break;

		case NamespaceKind_Extension:
			nspace->m_usingSet.addExtensionNamespace ((ExtensionNamespace*) usedNamespace);
			break;

		default:
			err::setFormatStringError ("invalid using: %s", getNamespaceKindString (namespaceKind));
			return false;
		}
	}

	return true;
}

bool
Parser::preDeclare ()
{
	if (!m_lastDeclaredItem || m_lastDeclaredItem->getItemKind () != ModuleItemKind_Property)
		return true;

	Property* prop = (Property*) m_lastDeclaredItem;
	return true;
}

bool
Parser::declare (Declarator* declarator)
{
	m_lastDeclaredItem = NULL;

	bool isLibrary = m_module->m_namespaceMgr.getCurrentNamespace ()->getNamespaceKind () == NamespaceKind_Library;

	if ((declarator->getTypeModifiers () & TypeModifier_Property) && m_storageKind != StorageKind_Typedef)
	{
		if (isLibrary)
		{
			err::setFormatStringError ("only functions can be part of library");
			return false;			
		}

		// too early to calctype cause maybe this property has a body
		// declare a typeless property for now

		return declareProperty (declarator, NULL, 0);
	}

	uint_t declFlags;
	Type* type = declarator->calcType (&declFlags);
	if (!type)
		return false;

	DeclaratorKind declaratorKind = declarator->getDeclaratorKind ();
	uint_t postModifiers = declarator->getPostDeclaratorModifiers ();
	TypeKind typeKind = type->getTypeKind ();

	if (isLibrary && typeKind != TypeKind_Function)
	{
		err::setFormatStringError ("only functions can be part of library");
		return false;			
	}

	if (postModifiers != 0 && typeKind != TypeKind_Function)
	{
		err::setFormatStringError ("unused post-declarator modifier '%s'", getPostDeclaratorModifierString (postModifiers).cc ());
		return false;
	}

	switch (m_storageKind)
	{
	case StorageKind_Typedef:
		return declareTypedef (declarator, type);

	case StorageKind_Alias:
		return declareAlias (declarator, type, declFlags);

	default:
		switch (typeKind)
		{
		case TypeKind_Void:
			err::setFormatStringError ("illegal use of type 'void'");
			return false;

		case TypeKind_Function:
			return declareFunction (declarator, (FunctionType*) type);

		case TypeKind_Property:
			return declareProperty (declarator, (PropertyType*) type, declFlags);

		default:
			return isClassType (type, ClassTypeKind_ReactorIface) ?
				declareReactor (declarator, (ClassType*) type, declFlags) :
				declareData (declarator, type, declFlags);
		}
	}
}

void
Parser::assignDeclarationAttributes (
	ModuleItem* item,
	const Token::Pos& pos
	)
{
	ModuleItemDecl* decl = item->getItemDecl ();
	ASSERT (decl);

	decl->m_accessKind = m_accessKind ?
		m_accessKind :
		m_module->m_namespaceMgr.getCurrentAccessKind ();

	// don't overwrite storage unless explicit

	if (m_storageKind)
		decl->m_storageKind = m_storageKind;

	decl->m_pos = pos;
	decl->m_parentUnit = m_module->m_unitMgr.getCurrentUnit ();
	decl->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace ();
	decl->m_attributeBlock = m_attributeBlock;

	item->m_flags |= ModuleItemFlag_User;

	m_attributeBlock = NULL;
	m_lastDeclaredItem = item;
}

bool
Parser::declareTypedef (
	Declarator* declarator,
	Type* type
	)
{
	ASSERT (m_storageKind == StorageKind_Typedef);

	bool result;

	if (!declarator->isSimple ())
	{
		err::setFormatStringError ("invalid typedef declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();

	rtl::String name = declarator->getName ()->getShortName ();
	rtl::String qualifiedName = nspace->createQualifiedName (name);

	ModuleItem* item;

	if (isClassType (type, ClassTypeKind_ReactorIface))
	{
		type = m_module->m_typeMgr.createReactorType (name, qualifiedName, (ClassType*) type, NULL);
		item = type;
	}
	else
	{
		Typedef* tdef = m_module->m_typeMgr.createTypedef (name, qualifiedName, type);
		item = tdef;
	}

	if (!item)
		return false;

	assignDeclarationAttributes (item, declarator->getPos ());

	result = nspace->addItem (item, item->getItemDecl ());
	if (!result)
		return false;

	return true;
}

bool
Parser::declareAlias (
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (!declarator->m_constructor.isEmpty ())
	{
		err::setFormatStringError ("alias cannot have constructor");
		return false;
	}

	if (declarator->m_initializer.isEmpty ())
	{
		err::setFormatStringError ("missing alias initializer");
		return false;
	}

	if (!declarator->isSimple ())
	{
		err::setFormatStringError ("invalid alias declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();

	rtl::String name = declarator->getName ()->getShortName ();
	rtl::String qualifiedName = nspace->createQualifiedName (name);
	rtl::BoxList <Token>* initializer = &declarator->m_initializer;

	Alias* alias = m_module->m_variableMgr.createAlias (name, qualifiedName, type, initializer);
	assignDeclarationAttributes (alias, declarator->getPos ());

	result = nspace->addItem (alias);
	if (!result)
		return false;

	if (nspace->getNamespaceKind () == NamespaceKind_Property)
	{
		Property* prop = (Property*) nspace;

		if (ptrTypeFlags & PtrTypeFlag_Bindable)
		{
			result = prop->setOnChanged (alias);
			if (!result)
				return false;
		}
		else if (ptrTypeFlags & PtrTypeFlag_AutoGet)
		{
			result = prop->setAutoGetValue (alias);
			if (!result)
				return false;
		}
	}

	return true;
}

bool
Parser::declareFunction (
	Declarator* declarator,
	FunctionType* type
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	NamespaceKind namespaceKind = nspace->getNamespaceKind ();
	DeclaratorKind declaratorKind = declarator->getDeclaratorKind ();
	uint_t postModifiers = declarator->getPostDeclaratorModifiers ();
	FunctionKind functionKind = declarator->getFunctionKind ();
	bool hasArgs = !type->getArgArray ().isEmpty ();

	if (declaratorKind == DeclaratorKind_UnaryBinaryOperator)
	{
		ASSERT (functionKind == FunctionKind_UnaryOperator || functionKind == FunctionKind_BinaryOperator);
		functionKind = hasArgs ? FunctionKind_BinaryOperator : FunctionKind_UnaryOperator;
	}

	ASSERT (functionKind);
	uint_t functionKindFlags = getFunctionKindFlags (functionKind);

	if ((functionKindFlags & FunctionKindFlag_NoStorage) && m_storageKind)
	{
		err::setFormatStringError ("'%s' cannot have storage specifier", getFunctionKindString (functionKind));
		return false;
	}

	if ((functionKindFlags & FunctionKindFlag_NoArgs) && hasArgs)
	{
		err::setFormatStringError ("'%s' cannot have arguments", getFunctionKindString (functionKind));
		return false;
	}

	if (!m_storageKind)
	{
		m_storageKind =
			functionKind == FunctionKind_StaticConstructor || functionKind == FunctionKind_StaticDestructor ? StorageKind_Static :
			namespaceKind == NamespaceKind_Property ? ((Property*) nspace)->getStorageKind () : StorageKind_Undefined;
	}

	if (namespaceKind == NamespaceKind_PropertyTemplate)
	{
		if (m_storageKind)
		{
			err::setFormatStringError ("invalid storage '%s' in property template", getStorageKindString (m_storageKind));
			return false;
		}

		if (postModifiers)
		{
			err::setFormatStringError ("unused post-declarator modifier '%s'", getPostDeclaratorModifierString (postModifiers).cc ());
			return false;
		}

		bool result = ((PropertyTemplate*) nspace)->addMethod (functionKind, type);
		if (!result)
			return false;

		m_lastDeclaredItem = type;
		return true;
	}

	UserModuleItem* functionItem;
	FunctionName* functionName;

	if (declarator->isQualified ())
	{
		Orphan* orphan = m_module->m_namespaceMgr.createOrphan (OrphanKind_Function, type);
		orphan->m_functionKind = functionKind;
		functionItem = orphan;
		functionName = orphan;
	}
	else
	{
		Function* function = m_module->m_functionMgr.createFunction (functionKind, type);
		functionItem = function;
		functionName = function;
	}

	functionName->m_declaratorName = *declarator->getName ();
	functionItem->m_tag = nspace->createQualifiedName (functionName->m_declaratorName);

	assignDeclarationAttributes (functionItem, declarator->getPos ());

	if (postModifiers & PostDeclaratorModifier_Const)
		functionName->m_thisArgTypeFlags = PtrTypeFlag_Const;

	switch (functionKind)
	{
	case FunctionKind_Named:
		functionItem->m_name = declarator->getName ()->getShortName ();
		functionItem->m_qualifiedName = nspace->createQualifiedName (functionItem->m_name);
		functionItem->m_tag = functionItem->m_qualifiedName;
		break;

	case FunctionKind_UnaryOperator:
		functionName->m_unOpKind = declarator->getUnOpKind ();
		functionItem->m_tag.appendFormat (".unary operator %s", getUnOpKindString (functionName->m_unOpKind));
		break;

	case FunctionKind_BinaryOperator:
		functionName->m_binOpKind = declarator->getBinOpKind ();
		functionItem->m_tag.appendFormat (".binary operator %s", getBinOpKindString (functionName->m_binOpKind));
		break;

	case FunctionKind_CastOperator:
		functionName->m_castOpType = declarator->getCastOpType ();
		functionItem->m_tag.appendFormat (".cast operator %s", functionName->m_castOpType->getTypeString ().cc ());
		break;

	default:
		functionItem->m_tag.appendFormat (".%s", getFunctionKindString (functionKind));
	}

	if (functionItem->getItemKind () == ModuleItemKind_Orphan)
	{
		if (namespaceKind == NamespaceKind_Library)
		{
			err::setFormatStringError ("illegal orphan in library '%s'", nspace->getQualifiedName ().cc ());
			return false;
		}

		return true;
	}

	ASSERT (functionItem->getItemKind () == ModuleItemKind_Function);
	Function* function = (Function*) functionItem;

	ExtensionNamespace* extensionNamespace;
	DerivableType* extensionType;
	TypeKind typeKind;

	switch (namespaceKind)
	{
	case NamespaceKind_Extension:
		if (function->isVirtual ())
		{
			err::setFormatStringError ("invalid storage '%s' in type extension", getStorageKindString (function->m_storageKind));
			return false;
		}

		extensionNamespace = (ExtensionNamespace*) nspace;
		extensionType = extensionNamespace->getType ();

		if (function->m_storageKind != StorageKind_Static)
		{
			function->m_storageKind = StorageKind_Member;
			function->convertToMemberMethod (extensionType);
		}

		function->m_parentNamespace = extensionType;
		function->m_extensionNamespace = extensionNamespace;
		break;

	case NamespaceKind_Type:
		typeKind = ((NamedType*) nspace)->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_Struct:
			return ((StructType*) nspace)->addMethod (function);

		case TypeKind_Union:
			return ((UnionType*) nspace)->addMethod (function);

		case TypeKind_Class:
			return ((ClassType*) nspace)->addMethod (function);

		default:
			err::setFormatStringError ("method members are not allowed in '%s'", ((NamedType*) nspace)->getTypeString ().cc ());
			return false;
		}

	case NamespaceKind_Property:
		return ((Property*) nspace)->addMethod (function);

	case NamespaceKind_Library:
		function->m_libraryTableIndex = m_libraryFunctionCount;
		m_libraryFunctionCount++;

		// and fall through

	default:
		if (postModifiers)
		{
			err::setFormatStringError ("unused post-declarator modifier '%s'", getPostDeclaratorModifierString (postModifiers).cc ());
			return false;
		}

		if (m_storageKind && m_storageKind != StorageKind_Static)
		{
			err::setFormatStringError ("invalid storage specifier '%s' for a global function", getStorageKindString (m_storageKind));
			return false;
		}
	}

	if (!nspace->getParentNamespace ()) // module constructor / destructor
		switch (functionKind)
		{
		case FunctionKind_Constructor:
			return function->getModule ()->setConstructor (function);

		case FunctionKind_Destructor:
			return function->getModule ()->setDestructor (function);
		}

	if (functionKind != FunctionKind_Named)
	{
		err::setFormatStringError (
			"invalid '%s' at '%s' namespace",
			getFunctionKindString (functionKind),
			getNamespaceKindString (namespaceKind)
			);
		return false;
	}

	return nspace->addFunction (function) != -1;
}

bool
Parser::declareProperty (
	Declarator* declarator,
	PropertyType* type,
	uint_t flags
	)
{
	if (!declarator->isSimple ())
	{
		err::setFormatStringError ("invalid property declarator");
		return false;
	}

	Property* prop = createProperty (
		declarator->getName ()->getShortName (),
		declarator->getPos ()
		);

	if (!prop)
		return false;

	if (type)
	{
		prop->m_flags |= flags;
		return prop->create (type);
	}

	if (declarator->getBaseType ()->getTypeKind () != TypeKind_Void ||
		!declarator->getPointerPrefixList ().isEmpty () ||
		!declarator->getSuffixList ().isEmpty ())
	{
		DeclTypeCalc typeCalc;
		m_lastPropertyGetterType = typeCalc.calcPropertyGetterType (declarator);
		if (!m_lastPropertyGetterType)
			return false;
	}
	else
	{
		m_lastPropertyGetterType = NULL;
	}

	if (declarator->getTypeModifiers () & TypeModifier_Const)
		prop->m_flags |= PropertyFlag_Const;

	m_lastPropertyTypeModifiers.takeOver (declarator);
	return true;
}

PropertyTemplate*
Parser::createPropertyTemplate ()
{
	PropertyTemplate* propertyTemplate = m_module->m_functionMgr.createPropertyTemplate ();
	uint_t modifiers = getTypeSpecifier ()->clearTypeModifiers (TypeModifier_Property | TypeModifier_Bindable);

	if (modifiers & TypeModifier_Bindable)
		propertyTemplate->m_typeFlags = PropertyTypeFlag_Bindable;

	return propertyTemplate;
}

Property*
Parser::createProperty (
	const rtl::String& name,
	const Token::Pos& pos
	)
{
	bool result;

	m_lastDeclaredItem = NULL;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	NamespaceKind namespaceKind = nspace->getNamespaceKind ();

	if (namespaceKind == NamespaceKind_PropertyTemplate)
	{
		err::setFormatStringError ("property templates cannot have property members");
		return NULL;
	}

	rtl::String qualifiedName = nspace->createQualifiedName (name);
	Property* prop = m_module->m_functionMgr.createProperty (name, qualifiedName);

	assignDeclarationAttributes (prop, pos);

	ExtensionNamespace* extensionNamespace;
	DerivableType* extensionType;
	TypeKind typeKind;

	switch (namespaceKind)
	{
	case NamespaceKind_Extension:
		if (prop->isVirtual ())
		{
			err::setFormatStringError ("invalid storage '%s' in type extension", getStorageKindString (prop->m_storageKind));
			return NULL;
		}

		extensionNamespace = (ExtensionNamespace*) nspace;
		extensionType = extensionNamespace->getType ();

		if (prop->m_storageKind != StorageKind_Static)
		{
			prop->m_storageKind = StorageKind_Member;
			prop->m_parentType = extensionType;
		}

		result = nspace->addItem (prop);
		if (!result)
			return NULL;

		prop->m_parentNamespace = extensionType;
		prop->m_extensionNamespace = extensionNamespace;
		break;

	case NamespaceKind_Type:
		typeKind = ((NamedType*) nspace)->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_Struct:
			result = ((StructType*) nspace)->addProperty (prop);
			break;

		case TypeKind_Union:
			result = ((UnionType*) nspace)->addProperty (prop);
			break;

		case TypeKind_Class:
			result = ((ClassType*) nspace)->addProperty (prop);
			break;

		default:
			err::setFormatStringError ("property members are not allowed in '%s'", ((NamedType*) nspace)->getTypeString ().cc ());
			return NULL;
		}

		if (!result)
			return NULL;

		break;

	case NamespaceKind_Property:
		result = ((Property*) nspace)->addProperty (prop);
		if (!result)
			return NULL;

		break;

	default:
		if (m_storageKind && m_storageKind != StorageKind_Static)
		{
			err::setFormatStringError ("invalid storage specifier '%s' for a global property", getStorageKindString (m_storageKind));
			return NULL;
		}

		result = nspace->addItem (prop);
		if (!result)
			return NULL;

		prop->m_storageKind = StorageKind_Static;
	}

	return prop;
}

bool
Parser::parseLastPropertyBody (const rtl::ConstBoxList <Token>& body)
{
	ASSERT (m_lastDeclaredItem && m_lastDeclaredItem->getItemKind () == ModuleItemKind_Property);

	bool result;

	Property* prop = (Property*) m_lastDeclaredItem;

	Parser parser (m_module);
	parser.m_stage = Parser::StageKind_Pass1;

	m_module->m_namespaceMgr.openNamespace (prop);

	result = parser.parseTokenList (SymbolKind_named_type_block_impl, body);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace ();

	return finalizeLastProperty (true);
}

bool
Parser::finalizeLastProperty (bool hasBody)
{
	ASSERT (m_lastDeclaredItem && m_lastDeclaredItem->getItemKind () == ModuleItemKind_Property);

	bool result;

	Property* prop = (Property*) m_lastDeclaredItem;
	if (prop->getType ())
		return true;

	// finalize getter

	if (!prop->m_getter)
	{
		if (!m_lastPropertyGetterType)
		{
			err::setFormatStringError ("incomplete property: no 'get' method or autoget field");
			return NULL;
		}

		Function* getter = m_module->m_functionMgr.createFunction (FunctionKind_Getter, m_lastPropertyGetterType);
		getter->m_flags |= ModuleItemFlag_User;

		result = prop->addMethod (getter);
		if (!result)
			return false;
	}
	else if (m_lastPropertyGetterType && m_lastPropertyGetterType->cmp (prop->m_getter->getType ()) != 0)
	{
		err::setFormatStringError ("getter type '%s' does not match property declaration", prop->m_getter->getType ()->getTypeString ().cc ());
		return NULL;
	}

	// finalize setter

	if (!(m_lastPropertyTypeModifiers.getTypeModifiers () & TypeModifier_Const) && !hasBody)
	{
		FunctionType* getterType = prop->m_getter->getType ()->getShortType ();
		rtl::Array <FunctionArg*> argArray = getterType->getArgArray ();

		Type* setterArgType = getterType->getReturnType ();
		if (setterArgType->getTypeKindFlags () & TypeKindFlag_Derivable)
		{
			Type* setAsType = ((DerivableType*) setterArgType)->getSetAsType ();
			if (setAsType)
				setterArgType = setAsType;
		}

		argArray.append (setterArgType->getSimpleFunctionArg ());

		FunctionType* setterType = m_module->m_typeMgr.getFunctionType (argArray);
		Function* setter = m_module->m_functionMgr.createFunction (FunctionKind_Setter, setterType);
		setter->m_flags |= ModuleItemFlag_User;

		result = prop->addMethod (setter);
		if (!result)
			return false;
	}

	// finalize binder

	if (m_lastPropertyTypeModifiers.getTypeModifiers () & TypeModifier_Bindable)
	{
		if (!prop->m_onChanged)
		{
			result = prop->createOnChanged ();
			if (!result)
				return false;
		}
	}

	// finalize auto-get value

	if (m_lastPropertyTypeModifiers.getTypeModifiers () & TypeModifier_AutoGet)
	{
		if (!prop->m_autoGetValue)
		{
			result = prop->createAutoGetValue (prop->m_getter->getType ()->getReturnType ());

			if (!result)
				return false;
		}
	}

	uint_t typeFlags = 0;
	if (prop->m_onChanged)
		typeFlags |= PropertyTypeFlag_Bindable;

	prop->m_type = prop->m_setter ?
		m_module->m_typeMgr.getPropertyType (
			prop->m_getter->getType (),
			*prop->m_setter->getTypeOverload (),
			typeFlags
			) :
		m_module->m_typeMgr.getPropertyType (
			prop->m_getter->getType (),
			NULL,
			typeFlags
			);

	if (prop->m_flags & (PropertyFlag_AutoGet | PropertyFlag_AutoSet))
		m_module->markForCompile (prop);

	if (prop->getStaticConstructor ())
		m_module->m_functionMgr.addStaticConstructor (prop);

	return true;
}

bool
Parser::declareReactor (
	Declarator* declarator,
	ClassType* type,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (declarator->getDeclaratorKind () != DeclaratorKind_Name)
	{
		err::setFormatStringError ("invalid reactor declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	NamespaceKind namespaceKind = nspace->getNamespaceKind ();

	NamedType* parentType = NULL;

	switch (namespaceKind)
	{
	case NamespaceKind_Property:
		parentType = ((Property*) nspace)->getParentType ();
		break;

	case NamespaceKind_Type:
		parentType = (NamedType*) nspace;
		break;
	}

	if (parentType && parentType->getTypeKind () != TypeKind_Class)
	{
		err::setFormatStringError ("'%s' cannot contain reactor members", parentType->getTypeString ().cc ());
		return false;
	}

	rtl::String name = declarator->getName ()->getShortName ();
	rtl::String qualifiedName = nspace->createQualifiedName (name);

	if (declarator->isQualified ())
	{
		Function* start = type->getVirtualMethodArray () [0];
		ASSERT (start->getName () == "start");

		Orphan* oprhan = m_module->m_namespaceMgr.createOrphan (OrphanKind_Reactor, start->getType ());
		oprhan->m_declaratorName = *declarator->getName ();
		assignDeclarationAttributes (oprhan, declarator->getPos ());
	}
	else
	{
		type = m_module->m_typeMgr.createReactorType (name, qualifiedName, (ReactorClassType*) type, (ClassType*) parentType);
		assignDeclarationAttributes (type, declarator->getPos ());
		result = declareData (declarator, type, ptrTypeFlags);
		if (!result)
			return false;
	}

	return true;
}

bool
Parser::declareData (
	Declarator* declarator,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	bool result;

	if (!declarator->isSimple ())
	{
		err::setFormatStringError ("invalid data declarator");
		return false;
	}

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	NamespaceKind namespaceKind = nspace->getNamespaceKind ();

	switch (namespaceKind)
	{
	case NamespaceKind_PropertyTemplate:
	case NamespaceKind_Extension:
		err::setFormatStringError ("'%s' cannot have data fields", getNamespaceKindString (namespaceKind));
		return false;
	}

	rtl::String name = declarator->getName ()->getShortName ();
	size_t bitCount = declarator->getBitCount ();
	rtl::BoxList <Token>* constructor = &declarator->m_constructor;
	rtl::BoxList <Token>* initializer = &declarator->m_initializer;

	if (isAutoSizeArrayType (type))
	{
		if (initializer->isEmpty ())
		{
			err::setFormatStringError ("auto-size array '%s' should have initializer", type->getTypeString ().cc ());
			return false;
		}

		ArrayType* arrayType = (ArrayType*) type;
		result = m_module->m_operatorMgr.parseAutoSizeArrayInitializer (*initializer, &arrayType->m_elementCount);
		if (!result)
			return false;

		if (m_stage == StageKind_Pass2)
		{
			result = arrayType->ensureLayout ();
			if (!result)
				return false;
		}
	}

	ModuleItem* dataItem = NULL;

	if (namespaceKind != NamespaceKind_Property && (ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable)))
	{
		err::setFormatStringError ("'%s' can only be used on property field", getPtrTypeFlagString (ptrTypeFlags & (PtrTypeFlag_AutoGet | PtrTypeFlag_Bindable)).cc ());
		return false;
	}

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	switch (m_storageKind)
	{
	case StorageKind_Undefined:
		switch (namespaceKind)
		{
		case NamespaceKind_Scope:
			m_storageKind = type->getTypeKind () == TypeKind_Class ? StorageKind_Heap : StorageKind_Stack;
			break;

		case NamespaceKind_Type:
			m_storageKind = StorageKind_Member;
			break;

		case NamespaceKind_Property:
			m_storageKind = ((Property*) nspace)->getParentType () ? StorageKind_Member : StorageKind_Static;
			break;

		default:
			m_storageKind = StorageKind_Static;
		}

		break;

	case StorageKind_Static:
		break;

	case StorageKind_Thread:
		if (!scope && (!constructor->isEmpty () || !initializer->isEmpty ()))
		{
			err::setFormatStringError ("global 'thread' variables cannot have initializers");
			return false;
		}

		break;

	case StorageKind_Heap:
	case StorageKind_Stack:
		if (!scope)
		{
			err::setFormatStringError ("can only use '%s' storage specifier for local variables", getStorageKindString (m_storageKind));
			return false;
		}

		break;

	case StorageKind_Mutable:
		switch (namespaceKind)
		{
		case NamespaceKind_Type:
			break;

		case NamespaceKind_Property:
			if (((Property*) nspace)->getParentType ())
				break;

		default:
			err::setFormatStringError ("'mutable' can only be applied to member fields");
			return false;
		}

		break;

	default:
		err::setFormatStringError ("invalid storage specifier '%s' for variable", getStorageKindString (m_storageKind));
		return false;
	}

	if (namespaceKind == NamespaceKind_Property)
	{
		Property* prop = (Property*) nspace;

		if (m_storageKind == StorageKind_Member)
		{
			dataItem = prop->createField (name, type, bitCount, ptrTypeFlags, constructor, initializer);
			if (!dataItem)
				return false;

			assignDeclarationAttributes (dataItem, declarator->getPos ());
		}
		else
		{
			Variable* variable = m_module->m_variableMgr.createVariable (
				m_storageKind,
				name,
				nspace->createQualifiedName (name),
				type,
				ptrTypeFlags,
				constructor,
				initializer
				);

			if (!variable)
				return false;

			assignDeclarationAttributes (variable, declarator->getPos ());

			result = nspace->addItem (variable);
			if (!result)
				return false;

			if (variable->isInitializationNeeded ())
				prop->m_initializedStaticFieldArray.append (variable);

			dataItem = variable;
		}

		if (ptrTypeFlags & PtrTypeFlag_Bindable)
		{
			result = prop->setOnChanged (dataItem);
			if (!result)
				return false;
		}
		else if (ptrTypeFlags & PtrTypeFlag_AutoGet)
		{
			result = prop->setAutoGetValue (dataItem);
			if (!result)
				return false;
		}

	}
	else if (m_storageKind != StorageKind_Member && m_storageKind != StorageKind_Mutable)
	{
		Variable* variable = m_module->m_variableMgr.createVariable (
			m_storageKind,
			name,
			nspace->createQualifiedName (name),
			type,
			ptrTypeFlags,
			constructor,
			initializer
			);

		assignDeclarationAttributes (variable, declarator->getPos ());

		result = nspace->addItem (variable);
		if (!result)
			return false;

		if (nspace->getNamespaceKind () == NamespaceKind_Type)
		{
			NamedType* namedType = (NamedType*) nspace;
			TypeKind namedTypeKind = namedType->getTypeKind ();

			switch (namedTypeKind)
			{
			case TypeKind_Class:
			case TypeKind_Struct:
			case TypeKind_Union:
				if (variable->isInitializationNeeded ())
					((DerivableType*) namedType)->m_initializedStaticFieldArray.append (variable);
				break;

			default:
				err::setFormatStringError ("field members are not allowed in '%s'", namedType->getTypeString ().cc ());
				return false;
			}
		}
		else if (scope)
		{
			switch (m_storageKind)
			{
			case StorageKind_Stack:
			case StorageKind_Heap:
				result = m_module->m_variableMgr.initializeVariable (variable);
				if (!result)
					return NULL;

				break;

			case StorageKind_Static:
			case StorageKind_Thread:
				if (!variable->isInitializationNeeded ())
					break;

				OnceStmt stmt;
				m_module->m_controlFlowMgr.onceStmt_Create (&stmt, variable->m_pos, m_storageKind);

				result = m_module->m_controlFlowMgr.onceStmt_PreBody (&stmt, variable->m_pos);
				if (!result)
					return NULL;

				result = m_module->m_variableMgr.initializeVariable (variable);
				if (!result)
					return NULL;

				Token::Pos pos = 
					!variable->m_initializer.isEmpty () ? variable->m_initializer.getTail ()->m_pos :
					!variable->m_constructor.isEmpty () ? variable->m_constructor.getTail ()->m_pos : 
					variable->m_pos;

				m_module->m_controlFlowMgr.onceStmt_PostBody (&stmt, pos);
			}
		}
	}
	else
	{
		ASSERT (nspace->getNamespaceKind () == NamespaceKind_Type);

		NamedType* namedType = (NamedType*) nspace;
		TypeKind namedTypeKind = namedType->getTypeKind ();

		StructField* field;

		switch (namedTypeKind)
		{
		case TypeKind_Class:
			field = ((ClassType*) namedType)->createField (name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		case TypeKind_Struct:
			field = ((StructType*) namedType)->createField (name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		case TypeKind_Union:
			field = ((UnionType*) namedType)->createField (name, type, bitCount, ptrTypeFlags, constructor, initializer);
			break;

		default:
			err::setFormatStringError ("field members are not allowed in '%s'", namedType->getTypeString ().cc ());
			return false;
		}

		if (!field)
			return false;

		assignDeclarationAttributes (field, declarator->getPos ());
	}

	return true;
}

bool
Parser::declareUnnamedStructOrUnion (DerivableType* type)
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Undefined;

	Declarator declarator;
	declarator.m_declaratorKind = DeclaratorKind_Name;
	declarator.m_pos = *type->getPos ();
	return declareData (&declarator, type, 0);
}

FunctionArg*
Parser::createFormalArg (
	DeclFunctionSuffix* argSuffix,
	Declarator* declarator
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();

	uint_t ptrTypeFlags = 0;
	Type* type = declarator->calcType (&ptrTypeFlags);
	if (!type)
		return NULL;

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Void:
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError (
			"function cannot accept '%s' as an argument",
			type->getTypeString ().cc ()
			);
		return NULL;
	}

	if (m_storageKind)
	{
		err::setFormatStringError ("invalid storage '%s' for argument", getStorageKindString (m_storageKind));
		return NULL;
	}

	m_storageKind = StorageKind_Stack;

	rtl::String name;

	if (declarator->isSimple ())
	{
		name = declarator->getName ()->getShortName ();
	}
	else if (declarator->getDeclaratorKind () != DeclaratorKind_Undefined)
	{
		err::setFormatStringError ("invalid formal argument declarator");
		return NULL;
	}

	rtl::BoxList <Token>* initializer = &declarator->m_initializer;

	FunctionArg* arg = m_module->m_typeMgr.createFunctionArg (name, type, ptrTypeFlags, initializer);
	assignDeclarationAttributes (arg, declarator->getPos ());

	argSuffix->m_argArray.append (arg);

	return arg;
}

bool
Parser::addEnumFlag (
	uint_t* flags,
	EnumTypeFlag flag
	)
{
	if (*flags & flag)
	{
		err::setFormatStringError ("modifier '%s' used more than once", getEnumTypeFlagString (flag));
		return false;
	}

	*flags |= flag;
	return true;
}

EnumType*
Parser::createEnumType (
	const rtl::String& name,
	Type* baseType,
	uint_t flags
	)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	EnumType* enumType = NULL;

	if (name.isEmpty ())
	{
		flags |= EnumTypeFlag_Exposed;
		enumType = m_module->m_typeMgr.createUnnamedEnumType (baseType, flags);
	}
	else
	{
		rtl::String qualifiedName = nspace->createQualifiedName (name);
		enumType = m_module->m_typeMgr.createEnumType (name, qualifiedName, baseType, flags);
		if (!enumType)
			return NULL;

		bool result = nspace->addItem (enumType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes (enumType, m_lastMatchedToken.m_pos);
	return enumType;
}

StructType*
Parser::createStructType (
	const rtl::String& name,
	rtl::BoxList <Type*>* baseTypeList,
	size_t fieldAlignment
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	StructType* structType = NULL;

	if (name.isEmpty ())
	{
		structType = m_module->m_typeMgr.createUnnamedStructType (fieldAlignment);
	}
	else
	{
		rtl::String qualifiedName = nspace->createQualifiedName (name);
		structType = m_module->m_typeMgr.createStructType (name, qualifiedName, fieldAlignment);
		if (!structType)
			return NULL;
	}

	if (baseTypeList)
	{
		rtl::BoxIterator <Type*> baseType = baseTypeList->getHead ();
		for (; baseType; baseType++)
		{
			result = structType->addBaseType (*baseType) != NULL;
			if (!result)
				return NULL;
		}
	}

	if (!name.isEmpty ())
	{
		result = nspace->addItem (structType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes (structType, m_lastMatchedToken.m_pos);
	return structType;
}

UnionType*
Parser::createUnionType (
	const rtl::String& name,
	size_t fieldAlignment
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	UnionType* unionType = NULL;

	if (name.isEmpty ())
	{
		unionType = m_module->m_typeMgr.createUnnamedUnionType (fieldAlignment);
	}
	else
	{
		rtl::String qualifiedName = nspace->createQualifiedName (name);
		unionType = m_module->m_typeMgr.createUnionType (name, qualifiedName, fieldAlignment);
		if (!unionType)
			return NULL;

		result = nspace->addItem (unionType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes (unionType, m_lastMatchedToken.m_pos);
	return unionType;
}

ClassType*
Parser::createClassType (
	const rtl::String& name,
	rtl::BoxList <Type*>* baseTypeList,
	size_t fieldAlignment,
	uint_t flags
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ClassType* classType;

	if (name.isEmpty ())
	{
		classType = m_module->m_typeMgr.createUnnamedClassType (fieldAlignment, flags);
	}
	else
	{
		rtl::String qualifiedName = nspace->createQualifiedName (name);
		classType = m_module->m_typeMgr.createClassType (name, qualifiedName, fieldAlignment, flags);
	}

	if (baseTypeList)
	{
		rtl::BoxIterator <Type*> baseType = baseTypeList->getHead ();
		for (; baseType; baseType++)
		{
			result = classType->addBaseType (*baseType) != NULL;
			if (!result)
				return NULL;
		}
	}

	if (!name.isEmpty ())
	{
		result = nspace->addItem (classType);
		if (!result)
			return NULL;
	}

	assignDeclarationAttributes (classType, m_lastMatchedToken.m_pos);
	return classType;
}

ClassType*
Parser::createLibraryType (const rtl::String& name)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ClassType* classType;

	rtl::String qualifiedName = nspace->createQualifiedName (name);
	classType = m_module->m_typeMgr.createClassType (name, qualifiedName);

	Type* baseType = m_module->m_typeMgr.getStdType (StdType_Library);
	result = classType->addBaseType (baseType) != NULL;
	if (!result)
		return NULL;

	result = nspace->addItem (classType);
	if (!result)
		return NULL;

	LibraryNamespace* libraryNamespace = m_module->m_namespaceMgr.createLibraryNamespace (classType);

	result = classType->addItem (libraryNamespace);
	if (!result)
		return NULL;

	assignDeclarationAttributes (classType, m_lastMatchedToken.m_pos);
	m_module->m_namespaceMgr.openNamespace (libraryNamespace);
	m_libraryFunctionCount = 0;
	return classType;
}

bool
Parser::finalizeLibraryType ()
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ASSERT (nspace->getNamespaceKind () == NamespaceKind_Library);

	LibraryNamespace* libraryNamespace = (LibraryNamespace*) nspace;
	ClassType* libraryType = libraryNamespace->getLibraryType ();

	if (!m_libraryFunctionCount)
	{
		err::setFormatStringError ("library '%s' has no any functions", libraryType->getQualifiedName ().cc ());
		return false;
	}

	ArrayType* functionTableType = m_module->m_typeMgr.getStdType (StdType_BytePtr)->getArrayType (m_libraryFunctionCount);
	libraryType->createField (functionTableType);
	m_module->m_namespaceMgr.closeNamespace ();
	return true;
}

bool
Parser::countReactionBindSites ()
{
	if (!m_reactionBindSiteCount)
	{
		err::setFormatStringError ("no bindable properties found");
		return false;
	}

	m_reactorTotalBindSiteCount += m_reactionBindSiteCount;
	m_reactionBindSiteCount = 0;
	m_reactionIndex++;
	return true;
}

bool 
Parser::addReactorBindSite (const Value& value)
{
	ASSERT (m_stage == StageKind_ReactorStarter);

	bool result;

	Value onChangedValue;
	result = m_module->m_operatorMgr.getPropertyOnChanged (value, &onChangedValue);
	if (!result)
		return false;
	
	// create bind site variable in entry block

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
	Type* type = m_module->m_typeMgr.getStdType (StdType_SimpleEventPtr);
	Variable* variable = m_module->m_variableMgr.createSimpleStackVariable ("onChanged", type);
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
	
	result = m_module->m_operatorMgr.storeDataRef (variable, onChangedValue);
	if (!result)
		return false;

	m_reactionBindSiteList.insertTail (variable);
	return true;
}

bool
Parser::finalizeReactor ()
{
	ASSERT (m_reactorType);
	ASSERT (!m_reactionList.isEmpty ());

	bool result = m_reactorType->subscribe (m_reactionList);
	if (!result)
		return false;

	m_reactionList.clear ();
	return true;
}

void
Parser::reactorOnEventDeclaration (
	rtl::BoxList <Value>* valueList,
	Declarator* declarator,
	rtl::BoxList <Token>* tokenList
	)
{
	ASSERT (m_reactorType);

	DeclFunctionSuffix* suffix = declarator->getFunctionSuffix ();
	ASSERT (suffix);

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType (suffix->getArgArray ());
	Reaction* reaction = AXL_MEM_NEW (Reaction);
	reaction->m_function = m_reactorType->createUnnamedMethod (
		StorageKind_Member, 
		FunctionKind_Internal, 
		functionType
		);

	reaction->m_function->setBody (tokenList);
	reaction->m_bindSiteList.takeOver (valueList);
	m_reactionList.insertTail (reaction);
}

bool
Parser::reactorExpressionStmt (rtl::BoxList <Token>* tokenList)
{
	ASSERT (m_reactorType);
	ASSERT (!tokenList->isEmpty ());

	bool result;

	ASSERT (m_reactorType);

	Parser parser (m_module);
	parser.m_stage = StageKind_ReactorStarter;
	parser.m_reactorType = m_reactorType;

	result = parser.parseTokenList (SymbolKind_expression, *tokenList);
	if (!result)
		return false;

	if (parser.m_reactionBindSiteList.isEmpty ())
	{
		err::setFormatStringError ("no bindable properties found");
		return false;
	}

	FunctionType* functionType = m_module->m_typeMgr.getFunctionType ();
	
	Reaction* reaction = AXL_MEM_NEW (Reaction);
	reaction->m_function = m_reactorType->createUnnamedMethod (StorageKind_Member, FunctionKind_Reaction, functionType);
	reaction->m_function->setBody (tokenList);
	reaction->m_function->m_reactionIndex = m_reactionIndex;
	reaction->m_bindSiteList.takeOver (&parser.m_reactionBindSiteList);
	m_reactionList.insertTail (reaction);

	m_reactionIndex++;
	return true;
}

bool
Parser::beginAutomatonFunction ()
{
	m_automatonState = AutomatonState_Idle;
	m_automatonSwitchBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	m_automatonReturnBlock = m_module->m_controlFlowMgr.createBlock ("automaton_return_block");
	m_automatonRegExpNameMgr.clear ();
	m_automatonRegExp.clear ();
	return true;
}

bool
Parser::finalizeAutomatonFunction ()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function->m_type->getFlags () & FunctionTypeFlag_Automaton);

	ClassType* recognizerType = (ClassType*) m_module->m_typeMgr.getStdType (StdType_Recognizer);
	Type* recognizerPtrType = recognizerType->getClassPtrType (ClassPtrTypeKind_Normal, PtrTypeFlag_Safe);

	if (m_automatonState == AutomatonState_RegExpCase)
	{
		m_module->m_controlFlowMgr.jump (m_automatonReturnBlock);
		m_module->m_namespaceMgr.closeScope ();
	}

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();

	if (function->isMember ())
		llvmArg++;

	Value recognizerValue (llvmArg++, recognizerPtrType);
	Value requestValue (llvmArg++, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int));

	// finalize regexp

	BasicBlock* automatonSetupBlock = m_module->m_controlFlowMgr.createBlock ("automaton_setup_block");

	fsm::RegExpCompiler regExpCompiler (&m_automatonRegExp);
	regExpCompiler.finalize ();

	// build dfa tables

	rtl::Array <fsm::DfaState*> stateArray = m_automatonRegExp.getDfaStateArray ();
	size_t stateCount = stateArray.getCount ();

	Type* stateFlagTableType = m_module->m_typeMgr.getArrayType (m_module->m_typeMgr.getPrimitiveType (TypeKind_Int_u), stateCount);
	Type* transitionTableType = m_module->m_typeMgr.getArrayType (m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT), stateCount * fsm::TransitionTableCharCount);

	Value stateFlagTableValue ((void*) NULL, stateFlagTableType);
	Value transitionTableValue ((void*) NULL, transitionTableType);

	uint_t* stateFlags = (uint_t*) stateFlagTableValue.getConstData ();
	size_t* transitionRow = (size_t*) transitionTableValue.getConstData ();

	rtl::HashTableMap <intptr_t, BasicBlock*, axl::rtl::HashId <intptr_t> > caseMap;
	for (size_t i = 0; i < stateCount; i++)
	{
		fsm::DfaState* state = stateArray [i];

		*stateFlags = 0;
		if (state->m_isAccept)
		{
			*stateFlags |= RecognizerStateFlag_Accept;
			caseMap [state->m_id] = (BasicBlock*) state->m_acceptContext;
		}

		if (state->m_transitionList.isEmpty ())
			*stateFlags |= RecognizerStateFlag_Final;

		memset (transitionRow, -1, sizeof (size_t) * fsm::TransitionTableCharCount);

		rtl::Iterator <fsm::DfaTransition> transitionIt = state->m_transitionList.getHead ();
		for (; transitionIt; transitionIt++)
		{
			fsm::DfaTransition* transition = *transitionIt;
			switch (transition->m_matchCondition.m_conditionKind)
			{
			case fsm::MatchConditionKind_Char:
				ASSERT (transition->m_matchCondition.m_char < fsm::TransitionTableCharCount);
				transitionRow [transition->m_matchCondition.m_char] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_CharSet:
				for (size_t j = 0; j < 256; j++)
					if (transition->m_matchCondition.m_charSet.getBit (j))
						transitionRow [j] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_Any:
				for (size_t j = 0; j < 256; j++)
					transitionRow [j] = transition->m_outState->m_id;
				break;
			}
		}

		stateFlags++;
		transitionRow += fsm::TransitionTableCharCount;
	}

	// generate switch 

	m_module->m_controlFlowMgr.setCurrentBlock (m_automatonSwitchBlock);

	m_module->m_llvmIrBuilder.createSwitch (
		requestValue,
		automatonSetupBlock,
		caseMap.getHead (),
		caseMap.getCount ()
		);
	
	// generate setup

	m_module->m_controlFlowMgr.setCurrentBlock (automatonSetupBlock);
	automatonSetupBlock->markReachable ();
	
	rtl::Array <StructField*> fieldArray = recognizerType->getMemberFieldArray ();
	
	Value fieldValue;

	bool result =
		m_module->m_operatorMgr.getClassField (recognizerValue, fieldArray [RecognizerField_StateCount], NULL, &fieldValue) &&
		m_module->m_operatorMgr.storeDataRef (fieldValue, Value (stateCount, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT))) &&
		m_module->m_operatorMgr.getClassField (recognizerValue, fieldArray [RecognizerField_StateFlagTable], NULL, &fieldValue) &&
		m_module->m_operatorMgr.storeDataRef (fieldValue, stateFlagTableValue) &&
		m_module->m_operatorMgr.getClassField (recognizerValue, fieldArray [RecognizerField_TransitionTable], NULL, &fieldValue) &&
		m_module->m_operatorMgr.storeDataRef (fieldValue, transitionTableValue);

	if (!result)
		return false;

	m_module->m_controlFlowMgr.jump (m_automatonReturnBlock);

	m_module->m_controlFlowMgr.setCurrentBlock (m_automatonReturnBlock);	

	Value retValue (
		AutomatonResult_Continue, 
		m_module->m_typeMgr.getStdType (StdType_AutomatonResult)
		);

	m_module->m_controlFlowMgr.ret (retValue);
	return true;
}

bool
isNameValue (
	const char* source,
	rtl::String* name,
	rtl::String* value
	)
{
	const char* p = source;
	while (isspace ((uchar_t) *p))
		p++;

	if (!isalpha ((uchar_t) *p) && *p != '_')
		return false;

	const char* nameBegin = p;
	while (isalnum ((uchar_t) *p) || *p == '_')
		p++;

	const char* nameEnd = p;

	while (isspace ((uchar_t) *p))
		p++;

	if (*p != '=')
		return false;

	name->copy (nameBegin, nameEnd - nameBegin);
	value->copy (p + 1);
	return true;
}	

bool
Parser::automatonRegExp (
	const rtl::String& source,
	const Token::Pos& pos
	)
{
	if (m_automatonState == AutomatonState_RegExpCase)
	{
		m_module->m_controlFlowMgr.jump (m_automatonReturnBlock);
		m_module->m_namespaceMgr.closeScope ();
	}

	rtl::String name;
	rtl::String value;
	bool isRegExpNameDef = isNameValue (source, &name, &value);
	if (isRegExpNameDef)
	{
		m_automatonRegExpNameMgr.addName (name, value);
		m_automatonState = AutomatonState_Idle;
		return true;
	}

	fsm::RegExpCompiler regExpCompiler (&m_automatonRegExp, &m_automatonRegExpNameMgr);
	
	BasicBlock* block = m_module->m_controlFlowMgr.createBlock ("automaton_regexp_block");
	
	bool result = regExpCompiler.incrementalCompile (source, block);
	if (!result)
		return false;

	m_module->m_namespaceMgr.openScope (pos);
	m_module->m_controlFlowMgr.setCurrentBlock (block);
	block->markReachable ();
	m_automatonState = AutomatonState_RegExpCase;
	return true;
}

bool
Parser::callBaseTypeMemberConstructor (
	const QualifiedName& name,
	rtl::BoxList <Value>* argList
	)
{
	ASSERT (m_constructorType || m_constructorProperty);

	Namespace* nspace = m_module->m_functionMgr.getCurrentFunction ()->getParentNamespace ();
	ModuleItem* item = nspace->findItemTraverse (name);
	if (!item)
	{
		err::setFormatStringError ("name '%s' is not found", name.getFullName ().cc ());
		return false;
	}

	Type* type = NULL;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Type:
		return callBaseTypeConstructor ((Type*) item, argList);

	case ModuleItemKind_Typedef:
		return callBaseTypeConstructor (((Typedef*) item)->getType (), argList);

	case ModuleItemKind_Property:
		err::setFormatStringError ("property construction is not yet implemented");
		return false;

	case ModuleItemKind_StructField:
		return callFieldConstructor ((StructField*) item, argList);

	case ModuleItemKind_Variable:
		err::setFormatStringError ("static field construction is not yet implemented");
		return false;

	default:
		err::setFormatStringError ("'%s' cannot be used in base-type-member construct list");
		return false;
	}
}

DerivableType*
Parser::findBaseType (size_t baseTypeIdx)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function); // should not be called at pass

	DerivableType* parentType = function->getParentType ();
	if (!parentType)
		return NULL;
	
	BaseTypeSlot* slot = parentType->getBaseTypeByIndex (baseTypeIdx);
	if (!slot)
		return NULL;

	return slot->getType ();
}

DerivableType*
Parser::getBaseType (size_t baseTypeIdx)
{
	DerivableType* type = findBaseType (baseTypeIdx);
	if (!type)
	{
		err::setFormatStringError ("'basetype%d' is not found", baseTypeIdx + 1);
		return NULL;
	}

	return type;
}

bool
Parser::getBaseType (
	size_t baseTypeIdx,
	Value* resultValue
	)
{
	DerivableType* type = getBaseType (baseTypeIdx);
	if (!type)
		return false;

	resultValue->setNamespace (type);
	return true;
}

bool
Parser::callBaseTypeConstructor (
	size_t baseTypeIdx,
	rtl::BoxList <Value>* argList
	)
{
	ASSERT (m_constructorType || m_constructorProperty);

	if (m_constructorProperty)
	{
		err::setFormatStringError ("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->m_tag.cc ());
		return false;
	}

	BaseTypeSlot* baseTypeSlot = m_constructorType->getBaseTypeByIndex (baseTypeIdx);
	if (!baseTypeSlot)
		return false;

	return callBaseTypeConstructorImpl (baseTypeSlot, argList);
}

bool
Parser::callBaseTypeConstructor (
	Type* type,
	rtl::BoxList <Value>* argList
	)
{
	ASSERT (m_constructorType || m_constructorProperty);

	if (m_constructorProperty)
	{
		err::setFormatStringError ("'%s.construct' cannot have base-type constructor calls", m_constructorProperty->m_tag.cc ());
		return false;
	}

	BaseTypeSlot* baseTypeSlot = m_constructorType->findBaseType (type);
	if (!baseTypeSlot)
	{
		err::setFormatStringError (
			"'%s' is not a base type of '%s'",
			type->getTypeString ().cc (),
			m_constructorType->getTypeString ().cc ()
			);
		return false;
	}

	return callBaseTypeConstructorImpl (baseTypeSlot, argList);
}

bool
Parser::callBaseTypeConstructorImpl (
	BaseTypeSlot* baseTypeSlot,
	rtl::BoxList <Value>* argList
	)
{
	DerivableType* type = baseTypeSlot->getType ();

	if (baseTypeSlot->m_flags & ModuleItemFlag_Constructed)
	{
		err::setFormatStringError ("'%s' is already constructed", type->getTypeString ().cc ());
		return false;
	}

	Function* constructor = type->getConstructor ();
	if (!constructor)
	{
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ());
		return false;
	}

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	argList->insertHead (thisValue);

	bool result = m_module->m_operatorMgr.callOperator (constructor, argList);
	if (!result)
		return false;

	baseTypeSlot->m_flags |= ModuleItemFlag_Constructed;
	return true;
}

bool
Parser::callFieldConstructor (
	StructField* field,
	rtl::BoxList <Value>* argList
	)
{
	ASSERT (m_constructorType || m_constructorProperty);

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	bool result;

	if (m_constructorProperty)
	{
		err::setFormatStringError ("property field construction is not yet implemented");
		return false;
	}

	if (field->getParentNamespace () != m_constructorType)
	{
		err::setFormatStringError (
			"'%s' is not an immediate field of '%s'",
			field->getName ().cc (),
			m_constructorType->getTypeString ().cc ()
			);
		return false;
	}

	if (field->getFlags () & ModuleItemFlag_Constructed)
	{
		err::setFormatStringError ("'%s' is already constructed", field->getName ().cc ());
		return false;
	}

	if (!(field->getType ()->getTypeKindFlags () & TypeKindFlag_Derivable) ||
		!((DerivableType*) field->getType ())->getConstructor ())
	{
		err::setFormatStringError ("'%s' has no constructor", field->getName ().cc ());
		return false;
	}

	Function* constructor = ((DerivableType*) field->getType ())->getConstructor ();

	Value fieldValue;
	result =
		m_module->m_operatorMgr.getField (thisValue, field, NULL, &fieldValue) &&
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, &fieldValue);

	if (!result)
		return false;

	argList->insertHead (fieldValue);

	result = m_module->m_operatorMgr.callOperator (constructor, argList);
	if (!result)
		return false;

	field->m_flags |= ModuleItemFlag_Constructed;
	return true;
}

bool
Parser::finalizeBaseTypeMemberConstructBlock ()
{
	ASSERT (m_constructorType || m_constructorProperty);

	Value thisValue = m_module->m_functionMgr.getThisValue ();

	bool result;

	if (m_constructorProperty)
		return
			m_constructorProperty->callMemberFieldConstructors (thisValue) &&
			m_constructorProperty->initializeMemberFields (thisValue) &&
			m_constructorProperty->callMemberPropertyConstructors (thisValue);

	ASSERT (thisValue);

	result =
		m_constructorType->callBaseTypeConstructors (thisValue) &&
		m_constructorType->callMemberFieldConstructors (thisValue) &&
		m_constructorType->initializeMemberFields (thisValue) &&
		m_constructorType->callMemberPropertyConstructors (thisValue);

	if (!result)
		return false;

	Function* preconstructor = m_constructorType->getPreConstructor ();
	if (!preconstructor)
		return true;

	return m_module->m_operatorMgr.callOperator (preconstructor, thisValue);
}

bool
Parser::finalizeCompoundStmt ()
{
	bool result;
	
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (scope->m_flags & ScopeFlag_Catch)
	{
		result = m_module->m_controlFlowMgr.closeCatch ();
		if (!result)
			return false;
	}
	else if (scope->m_flags & ScopeFlag_Finally) 
	{
		result = m_module->m_controlFlowMgr.closeFinally ();
		if (!result)
			return false;
	}
	else if (scope->m_flags & ScopeFlag_Try) 
	{
		result = m_module->m_controlFlowMgr.closeTry ();
		if (!result)
			return false;
	}
	else
	{
		if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
			m_module->m_operatorMgr.nullifyGcRootList (scope->getGcStackRootList ());

		m_module->m_namespaceMgr.closeScope ();
	}

	if (scope->m_flags & ScopeFlag_Nested)
	{
		Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
		ASSERT (scope && !(scope->m_flags & ScopeFlag_Nested)); // double-nested

		if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
			m_module->m_operatorMgr.nullifyGcRootList (scope->getGcStackRootList ());

		m_module->m_namespaceMgr.closeScope ();
	}

	return true;
}

bool
Parser::newOperator_0 (
	Type* type,
	Value* resultValue
	)
{
	resultValue->setType (m_module->m_operatorMgr.getNewOperatorResultType (type));
	return true;
}

bool
Parser::lookupIdentifier (
	const rtl::String& name,
	const Token::Pos& pos,
	Value* value
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ModuleItem* item = NULL;

	MemberCoord coord;
	item = nspace->findItemTraverse (name, &coord);
	if (!item)
	{
		err::setFormatStringError ("undeclared identifier '%s'", name.cc ());
		err::pushSrcPosError (m_module->m_unitMgr.getCurrentUnit ()->getFilePath (), pos);
		return false;
	}

	Value thisValue;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		value->setNamespace ((GlobalNamespace*) item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().cc ());
			return false;
		}

		value->setNamespace ((NamedType*) item);
		break;

	case ModuleItemKind_Alias:
		return m_module->m_operatorMgr.evaluateAlias (
			item->getItemDecl (),
			((Alias*) item)->getInitializer (),
			value
			);

	case ModuleItemKind_Const:
		*value = ((Const*) item)->getValue ();
		break;

	case ModuleItemKind_Variable:
		if (m_flags & Flag_ConstExpression)
		{
			err::setFormatStringError ("variable '%s' cannot be used in const expression", name.cc ());
			return false;
		}

		value->setVariable ((Variable*) item);
		break;

	case ModuleItemKind_Function:
		if (m_flags & Flag_ConstExpression)
		{
			err::setFormatStringError ("function '%s' cannot be used in const expression", name.cc ());
			return false;
		}

		value->setFunction ((Function*) item);

		if (((Function*) item)->isMember ())
		{
			result = m_module->m_operatorMgr.createMemberClosure (value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_Property:
		if (m_flags & Flag_ConstExpression)
		{
			err::setFormatStringError ("property '%s' cannot be used in const expression", name.cc ());
			return false;
		}

		value->setProperty ((Property*) item);

		if (((Property*) item)->isMember ())
		{
			result = m_module->m_operatorMgr.createMemberClosure (value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_EnumConst:
		if (!(item->getFlags () & EnumConstFlag_ValueReady))
		{
			result = ((EnumConst*) item)->getParentEnumType ()->ensureLayout ();
			if (!result)
				return false;
		}

		value->setConstInt64 (
			((EnumConst*) item)->getValue (),
			((EnumConst*) item)->getParentEnumType ()
			);
		break;

	case ModuleItemKind_StructField:
		if (m_flags & Flag_ConstExpression)
		{
			err::setFormatStringError ("field '%s' cannot be used in const expression", name.cc ());
			return false;
		}

		result =
			m_module->m_operatorMgr.getThisValue (&thisValue) &&
			m_module->m_operatorMgr.getField (thisValue, (StructField*) item, &coord, value);

		if (!result)
			return false;

		break;

	default:
		err::setFormatStringError (
			"%s '%s' cannot be used as expression",
			getModuleItemKindString (item->getItemKind ()),
			name.cc ()
			);
		return false;
	};

	return true;
}

bool
Parser::lookupIdentifierType (
	const rtl::String& name,
	const Token::Pos& pos,
	Value* value
	)
{
	bool result;

	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	ModuleItem* item = NULL;

	item = nspace->findItemTraverse (name);
	if (!item)
	{
		err::setFormatStringError ("undeclared identifier '%s'", name.cc ());
		err::pushSrcPosError (m_module->m_unitMgr.getCurrentUnit ()->getFilePath (), pos);
		return false;
	}

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		value->setNamespace ((GlobalNamespace*) item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().cc ());
			return false;
		}

		value->setNamespace ((NamedType*) item);
		break;

	case ModuleItemKind_Variable:
		value->setType (((Variable*) item)->getType ()->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean));
		break;

	case ModuleItemKind_Alias:
		value->setType (((Alias*) item)->getType ());
		break;

	case ModuleItemKind_Function:
		{
		Function* function = (Function*) item;
		value->setFunctionTypeOverload (function->getTypeOverload ());

		if (((Function*) item)->isMember ())
		{
			result = m_module->m_operatorMgr.createMemberClosure (value);
			if (!result)
				return false;
		}
		}
		break;

	case ModuleItemKind_Property:
		value->setType (((Property*) item)->getType ()->getPropertyPtrType (TypeKind_PropertyRef, PropertyPtrTypeKind_Thin));

		if (((Property*) item)->isMember ())
		{
			result = m_module->m_operatorMgr.createMemberClosure (value);
			if (!result)
				return false;
		}

		break;

	case ModuleItemKind_EnumConst:
		value->setType (((EnumConst*) item)->getParentEnumType ()->getBaseType ());
		break;

	case ModuleItemKind_StructField:
		value->setType (((StructField*) item)->getType ()->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean));
		break;

	default:
		err::setFormatStringError ("'%s' cannot be used as expression", name.cc ());
		return false;
	};

	return true;
}

bool
Parser::prepareCurlyInitializerNamedItem (
	CurlyInitializer* initializer,
	const char* name
	)
{
	Value memberValue;

	bool result = m_module->m_operatorMgr.memberOperator (
		initializer->m_targetValue,
		name,
		&initializer->m_memberValue
		);

	if (!result)
		return false;

	initializer->m_index = -1;
	initializer->m_count++;
	m_curlyInitializerTargetValue = initializer->m_memberValue;
	return true;
}

bool
Parser::prepareCurlyInitializerIndexedItem (CurlyInitializer* initializer)
{
	if (initializer->m_index == -1)
	{
		err::setFormatStringError ("indexed-baded initializer cannot be used after named-based initializer");
		return false;
	}

	bool result = m_module->m_operatorMgr.memberOperator (
		initializer->m_targetValue,
		initializer->m_index,
		&initializer->m_memberValue
		);

	if (!result)
		return false;

	initializer->m_index++;
	initializer->m_count++;
	m_curlyInitializerTargetValue = initializer->m_memberValue;
	return true;
}

bool
Parser::skipCurlyInitializerItem (CurlyInitializer* initializer)
{
	if (initializer->m_index == -1)
	{
		err::setFormatStringError ("indexed-baded initializer cannot be used after named-based initializer");
		return false;
	}

	initializer->m_index++;
	return true;
}

bool
Parser::addFmtSite (
	Literal* literal,
	const char* p,
	size_t length,
	const Value& value,
	bool isIndex,
	const rtl::String& fmtSpecifierString
	)
{
	literal->m_binData.append (p, length);

	FmtSite* site = AXL_MEM_NEW (FmtSite);
	site->m_offset = literal->m_binData.getCount ();
	site->m_fmtSpecifierString = fmtSpecifierString;
	literal->m_fmtSiteList.insertTail (site);
	literal->m_isZeroTerminated = true;

	if (!isIndex)
	{
		site->m_value = value;
		site->m_index = -1;
	}
	else
	{
		if (value.getValueKind () != ValueKind_Const ||
			!(value.getType ()->getTypeKindFlags () & TypeKindFlag_Integer))
		{
			err::setFormatStringError ("expression is not integer constant");
			return false;
		}

		site->m_index = 0;
		memcpy (&site->m_index, value.getConstData (), value.getType ()->getSize ());
	}

	return true;
}

bool
Parser::finalizeLiteral_0 (
	Literal_0* literal,
	Value* resultValue
	)
{
	Type* type;

	if (literal->m_isFmtLiteral)
	{
		type = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType ();
	}
	else
	{
		if (literal->m_isZeroTerminated)
			literal->m_length++;

		type = m_module->m_typeMgr.getArrayType (m_module->m_typeMgr.getPrimitiveType (TypeKind_Char), literal->m_length);
	}

	resultValue->setType (type);
	return true;
}

bool
Parser::finalizeLiteral (
	Literal* literal,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	bool result;

	if (literal->m_fmtSiteList.isEmpty ())
	{
		if (literal->m_isZeroTerminated)
			literal->m_binData.append (0);

		resultValue->setCharArray (literal->m_binData, literal->m_binData.getCount (), m_module);
		return true;
	}

	char buffer [256];
	rtl::Array <Value*> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	size_t argCount = 0;

	if (argValueList)
	{
		argCount = argValueList->getCount ();
		argValueArray.setCount (argCount);

		rtl::BoxIterator <Value> it = argValueList->getHead ();
		for (size_t i = 0; i < argCount; i++, it++)
		{
			ASSERT (it);
			argValueArray [i] = it.p ();
		}
	}

	Type* type = m_module->m_typeMgr.getStdType (StdType_FmtLiteral);
	Variable* fmtLiteral = m_module->m_variableMgr.createSimpleStackVariable ("fmtLiteral", type);

	result = m_module->m_variableMgr.initializeVariable (fmtLiteral);
	ASSERT (result);

	Value fmtLiteralValue = fmtLiteral;

	size_t offset = 0;

	rtl::BitMap argUsageMap;
	argUsageMap.setBitCount (argCount);

	rtl::Iterator <FmtSite> siteIt = literal->m_fmtSiteList.getHead ();
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
				err::setFormatStringError ("formatting literal doesn't have argument %%%d\n", site->m_index);
				return false;
			}

			value = argValueArray [i];
			argUsageMap.setBit (i);
		}

		if (site->m_offset > offset)
		{
			size_t length = site->m_offset - offset;
			appendFmtLiteralRawData (
				fmtLiteralValue, 
				literal->m_binData + offset,
				length 
				);

			offset += length;
		}

		result = appendFmtLiteralValue (fmtLiteralValue, *value, site->m_fmtSpecifierString);
		if (!result)
			return false;
	}

	size_t unusedArgIdx = argUsageMap.findBit (0, false);
	if (unusedArgIdx < argCount)
	{
		err::setFormatStringError ("formatting literal argument %%%d is not used", unusedArgIdx + 1);
		return false;
	}

	size_t endOffset = literal->m_binData.getCount ();
	if (endOffset > offset)
	{
		size_t length = endOffset - offset;
		appendFmtLiteralRawData (
			fmtLiteralValue, 
			literal->m_binData + offset,
			length 
			);
	}

	Type* validatorType = m_module->m_typeMgr.getStdType (StdType_DataPtrValidatorPtr);

	Value fatPtrValue;
	Value thinPtrValue;
	Value validatorValue;

	m_module->m_llvmIrBuilder.createGep2 (fmtLiteralValue, 0, NULL, &fatPtrValue);
	m_module->m_llvmIrBuilder.createLoad (fatPtrValue, NULL, &fatPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue (fatPtrValue, 0, NULL, &thinPtrValue);
	m_module->m_llvmIrBuilder.createExtractValue (fatPtrValue, 1, validatorType, &validatorValue);

	resultValue->setLeanDataPtr (
		thinPtrValue.getLlvmValue (),
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType (DataPtrTypeKind_Lean),
		validatorValue
		);

	return true;
}

void
Parser::appendFmtLiteralRawData (
	const Value& fmtLiteralValue,
	const void* p,
	size_t length
	)
{
	Function* append = m_module->m_functionMgr.getStdFunction (StdFunc_AppendFmtLiteral_a);

	Value literalValue;
	literalValue.setCharArray (p, length, m_module);
	m_module->m_operatorMgr.castOperator (&literalValue, m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c ());

	Value lengthValue;
	lengthValue.setConstSizeT (length, m_module);

	Value resultValue;
	m_module->m_llvmIrBuilder.createCall3 (
		append,
		append->getType (),
		fmtLiteralValue,
		literalValue,
		lengthValue,
		&resultValue
		);
}

bool
Parser::appendFmtLiteralValue (
	const Value& fmtLiteralValue,
	const Value& rawSrcValue,
	const rtl::String& fmtSpecifierString
	)
{
	if (fmtSpecifierString == 'B') // binary format
		return appendFmtLiteralBinValue (fmtLiteralValue, rawSrcValue);

	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand (rawSrcValue, &srcValue);
	if (!result)
		return false;

	StdFunc appendFunc;

	Type* type = srcValue.getType ();
	TypeKind typeKind = type->getTypeKind ();
	uint_t typeKindFlags = type->getTypeKindFlags ();

	if (typeKindFlags & TypeKindFlag_Integer)
	{
		static StdFunc funcTable [2] [2] =
		{
			{ StdFunc_AppendFmtLiteral_i32, StdFunc_AppendFmtLiteral_ui32 },
			{ StdFunc_AppendFmtLiteral_i64, StdFunc_AppendFmtLiteral_ui64 },
		};

		size_t i1 = type->getSize () > 4;
		size_t i2 = (typeKindFlags & TypeKindFlag_Unsigned) != 0;

		appendFunc = funcTable [i1] [i2];
	}
	else if (typeKindFlags & TypeKindFlag_Fp)
	{
		appendFunc = StdFunc_AppendFmtLiteral_f;
	}
	else if (typeKind == TypeKind_Variant)
	{
		appendFunc = StdFunc_AppendFmtLiteral_v;
	}
	else if (isCharArrayType (type) || isCharArrayRefType (type) || isCharPtrType (type))
	{
		appendFunc = StdFunc_AppendFmtLiteral_p;
	}
	else
	{
		StdType stdType = type->getStdType ();
		switch (stdType)
		{
		case StdType_String:
			appendFunc = StdFunc_AppendFmtLiteral_s;
			break;

		case StdType_StringRef:
			appendFunc = StdFunc_AppendFmtLiteral_sr;
			break;

		case StdType_ConstBuffer:
			appendFunc = StdFunc_AppendFmtLiteral_cb;
			break;

		case StdType_ConstBufferRef:
			appendFunc = StdFunc_AppendFmtLiteral_cbr;
			break;

		case StdType_BufferRef:
			appendFunc = StdFunc_AppendFmtLiteral_br;
			break;

		default:
			err::setFormatStringError ("don't know how to format '%s'", type->getTypeString ().cc ());
			return false;
		}
	}

	Function* append = m_module->m_functionMgr.getStdFunction (appendFunc);
	Type* argType = append->getType ()->getArgArray () [2]->getType ();

	Value argValue;
	result = m_module->m_operatorMgr.castOperator (srcValue, argType, &argValue);
	if (!result)
		return false;

	Value fmtSpecifierValue;
	if (!fmtSpecifierString.isEmpty ())
	{
		fmtSpecifierValue.setCharArray (fmtSpecifierString, fmtSpecifierString.getLength () + 1, m_module);
		m_module->m_operatorMgr.castOperator (&fmtSpecifierValue, m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c ());
	}
	else
	{
		fmtSpecifierValue = m_module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType_c ()->getZeroValue ();
	}

	return m_module->m_operatorMgr.callOperator (
		append, 
		fmtLiteralValue,
		fmtSpecifierValue,
		argValue
		);
}

bool
Parser::appendFmtLiteralBinValue (
	const Value& fmtLiteralValue,
	const Value& rawSrcValue
	)
{
	Value srcValue;
	bool result = m_module->m_operatorMgr.prepareOperand (rawSrcValue, &srcValue);
	if (!result)
		return false;

	Type* type = srcValue.getType ();
	Function* append = m_module->m_functionMgr.getStdFunction (StdFunc_AppendFmtLiteral_a);
	Type* argType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	Value sizeValue (
		type->getSize (), 
		m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)
		);

	Value tmpValue;
	Value resultValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpFmtValue", NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createStore (srcValue, tmpValue);
	m_module->m_llvmIrBuilder.createBitCast (tmpValue, argType, &tmpValue);

	m_module->m_llvmIrBuilder.createCall3 (
		append,
		append->getType (),
		fmtLiteralValue,
		tmpValue,
		sizeValue,
		&resultValue
		);

	return true;
}

BasicBlock*
Parser::assertCondition (const rtl::BoxList <Token>& tokenList)
{
	bool result;

	Value conditionValue;
	result = m_module->m_operatorMgr.parseExpression (NULL, tokenList, &conditionValue);
	if (!result)
		return NULL;

	BasicBlock* failBlock = m_module->m_controlFlowMgr.createBlock ("assert_fail");
	BasicBlock* continueBlock = m_module->m_controlFlowMgr.createBlock ("assert_continue");
	
	result = m_module->m_controlFlowMgr.conditionalJump (conditionValue, continueBlock, failBlock, failBlock); 
	if (!result)
		return NULL;
	
	return continueBlock;
}

bool 
Parser::finalizeAssertStmt (
	const rtl::BoxList <Token>& conditionTokenList,
	const Value& messageValue,
	BasicBlock* continueBlock
	)
{
	ASSERT (!conditionTokenList.isEmpty ());

	rtl::String fileName = m_module->m_unitMgr.getCurrentUnit ()->getFilePath ();
	rtl::String conditionString = Token::getTokenListString (conditionTokenList);
	Token::Pos pos = conditionTokenList.getHead ()->m_pos;

	Value fileNameValue;
	Value lineValue;
	Value conditionValue;
	
	fileNameValue.setCharArray (fileName, fileName.getLength (), m_module);
	lineValue.setConstInt32 (pos.m_line, m_module);
	conditionValue.setCharArray (conditionString, conditionString.getLength (), m_module);

	Function* assertionFailure = m_module->m_functionMgr.getStdFunction (StdFunc_AssertionFailure);

	rtl::BoxList <Value> argValueList;
	argValueList.insertTail (fileNameValue);
	argValueList.insertTail (lineValue);
	argValueList.insertTail (conditionValue);

	if (messageValue)
	{
		argValueList.insertTail (messageValue);
	}
	else
	{
		Value nullValue;
		nullValue.setNull (m_module);
		argValueList.insertTail (nullValue);
	}
	
	bool result = m_module->m_operatorMgr.callOperator (assertionFailure, &argValueList);
	if (!result)
		return false;
		
	m_module->m_controlFlowMgr.follow (continueBlock); 
	return true;
}

void
Parser::addScopeAnchorToken (
	StmtPass1* stmt,
	const Token& token
	)
{
	rtl::BoxIterator <Token> it = stmt->m_tokenList.insertTail (token);
	stmt->m_scopeAnchorToken = &*it;
	stmt->m_scopeAnchorToken->m_data.m_integer = 0; // tokens can be reused, ensure 0
}

//.............................................................................

} // namespace jnc {

/*

bool
Parser::preCreateLandingPads (uint_t flags)
{
	if (!flags)
		return true;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	if (flags & LandingPadFlag_Catch)
	{
	}

	if (flags & LandingPadFlag_Finally)
	{
	}

	return true;
}

 */

