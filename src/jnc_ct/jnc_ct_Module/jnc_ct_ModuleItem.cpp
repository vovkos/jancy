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
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ModuleItemDecl::ModuleItemDecl()
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Public; // public by default
	m_parentNamespace = NULL;
	m_attributeBlock = NULL;
	m_doxyBlock = NULL;
}

void
ModuleItemDecl::prepareQualifiedName()
{
	ASSERT(m_qualifiedName.isEmpty());
	m_qualifiedName = m_parentNamespace ? m_parentNamespace->createQualifiedName(m_name) : m_name;
}

sl::String
ModuleItemDecl::getDoxyLocationString()
{
	if (!m_parentUnit)
		return sl::String();

	sl::String string;

	string.format("<location file='%s' line='%d' col='%d'/>\n",
		m_parentUnit->getFileName().sz(),
		m_pos.m_line + 1,
		m_pos.m_col + 1
		);

	return string;
}

//..............................................................................

bool
ModuleItemBodyDecl::setBody(
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	if (!canSetBody())
		return false;

	m_bodyPos = pos;
	m_body = body;
	return true;
}

bool
ModuleItemBodyDecl::setBody(sl::BoxList<Token>* tokenList)
{
	if (!canSetBody())
		return false;

	m_bodyPos = tokenList->getHead()->m_pos;
	sl::takeOver(&m_bodyTokenList, tokenList);
	return true;
}

bool
ModuleItemBodyDecl::canSetBody()
{
	if (!m_body.isEmpty() || !m_bodyTokenList.isEmpty())
	{
		err::setFormatStringError("'%s' already has a body", getQualifiedName().sz());
		return false;
	}

	if (m_storageKind == StorageKind_Abstract)
	{
		err::setFormatStringError("'%s' is abstract and hence cannot have a body", getQualifiedName().sz());
		return false;
	}

	return true;
}

//..............................................................................

ModuleItemDecl*
getNullDecl(ModuleItem* item)
{
	return NULL;
}

ModuleItemDecl*
getNamespaceDecl(ModuleItem* item)
{
	return (GlobalNamespace*)item;
}

ModuleItemDecl*
getScopeDecl(ModuleItem* item)
{
	return (Scope*)item;
}

ModuleItemDecl*
getAttributeDecl(ModuleItem* item)
{
	return (Attribute*)item;
}

ModuleItemDecl*
getAttributeBlockDecl(ModuleItem* item)
{
	return (AttributeBlock*)item;
}

ModuleItemDecl*
getTypeDecl(ModuleItem* item)
{
	return (((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named) ? (NamedType*)item : NULL;
}

ModuleItemDecl*
getTypedefDecl(ModuleItem* item)
{
	return (Typedef*)item;
}

ModuleItemDecl*
getAliasDecl(ModuleItem* item)
{
	return (Alias*)item;
}

ModuleItemDecl*
getConstDecl(ModuleItem* item)
{
	return (Const*)item;
}

ModuleItemDecl*
getVariableDecl(ModuleItem* item)
{
	return (Variable*)item;
}

ModuleItemDecl*
getFunctionDecl(ModuleItem* item)
{
	return (Function*)item;
}

ModuleItemDecl*
getFunctionArgDecl(ModuleItem* item)
{
	return (FunctionArg*)item;
}

ModuleItemDecl*
getFunctionOverloadDecl(ModuleItem* item)
{
	return (FunctionOverload*)item;
}

ModuleItemDecl*
getPropertyDecl(ModuleItem* item)
{
	return (Property*)item;
}

ModuleItemDecl*
getPropertyTemplateDecl(ModuleItem* item)
{
	return (PropertyTemplate*)item;
}

ModuleItemDecl*
getEnumConstDecl(ModuleItem* item)
{
	return (EnumConst*)item;
}

ModuleItemDecl*
getFieldDecl(ModuleItem* item)
{
	return (Field*)item;
}

ModuleItemDecl*
getBaseTypeSlotDecl(ModuleItem* item)
{
	return (BaseTypeSlot*)item;
}

ModuleItemDecl*
getOrphanDecl(ModuleItem* item)
{
	return (Orphan*)item;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Namespace*
getNullNamespace(ModuleItem* item)
{
	return NULL;
}

Namespace*
getNamespaceNamespace(ModuleItem* item)
{
	return (GlobalNamespace*)item;
}

Namespace*
getScopeNamespace(ModuleItem* item)
{
	return (Scope*)item;
}

Namespace*
getTypeNamespace(ModuleItem* item)
{
	return (((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named) ? (NamedType*)item : NULL;
}

Namespace*
getTypedefNamespace(ModuleItem* item)
{
	return getTypeNamespace(((Typedef*)item)->getType());
}

Namespace*
getPropertyNamespace(ModuleItem* item)
{
	return (Property*)item;
}

Namespace*
getPropertyTemplateNamespace(ModuleItem* item)
{
	return (PropertyTemplate*)item;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Type*
getNullType(ModuleItem* item)
{
	return NULL;
}

Type*
getTypeType(ModuleItem* item)
{
	return (Type*)item;
}

Type*
getTypedefType(ModuleItem* item)
{
	return ((Typedef*)item)->getType();
}

Type*
getAliasType(ModuleItem* item)
{
	return ((Alias*)item)->getTargetItem()->getType();
}

Type*
getConstType(ModuleItem* item)
{
	return ((Const*)item)->getValue().getType();
}

Type*
getVariableType(ModuleItem* item)
{
	return ((Variable*)item)->getType();
}

Type*
getFunctionType(ModuleItem* item)
{
	return ((Function*)item)->getType();
}

Type*
getFunctionArgType(ModuleItem* item)
{
	return ((FunctionArg*)item)->getType();
}

Type*
getPropertyType(ModuleItem* item)
{
	return ((Property*)item)->getType();
}

Type*
getPropertyTemplateType(ModuleItem* item)
{
	return ((PropertyTemplate*)item)->calcType();
}

Type*
getEnumConstType(ModuleItem* item)
{
	return ((EnumConst*)item)->getParentEnumType();
}

Type*
getFieldType(ModuleItem* item)
{
	return ((Field*)item)->getType();
}

Type*
getBaseTypeSlotType(ModuleItem* item)
{
	return ((BaseTypeSlot*)item)->getType();
}

Type*
getOrphanType(ModuleItem* item)
{
	return ((Orphan*)item)->getFunctionType();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ModuleItem::ModuleItem()
{
	m_module = NULL;
	m_itemKind = ModuleItemKind_Undefined;
	m_flags = 0;
}

ModuleItemDecl*
ModuleItem::getDecl()
{
	typedef
	ModuleItemDecl*
	GetDeclFunc(ModuleItem* item);

	GetDeclFunc* getDeclFuncTable[ModuleItemKind__Count] =
	{
		getNullDecl,             // ModuleItemKind_Undefined = 0,
		getNamespaceDecl,        // ModuleItemKind_Namespace,
		getScopeDecl,            // ModuleItemKind_Scope,
		getAttributeDecl,        // ModuleItemKind_Attribute,
		getAttributeBlockDecl,   // ModuleItemKind_AttributeBlock,
		getTypeDecl,             // ModuleItemKind_Type,
		getTypedefDecl,          // ModuleItemKind_Typedef,
		getAliasDecl,            // ModuleItemKind_Alias,
		getConstDecl,            // ModuleItemKind_Const,
		getVariableDecl,         // ModuleItemKind_Variable,
		getFunctionDecl,         // ModuleItemKind_Function,
		getFunctionArgDecl,      // ModuleItemKind_FunctionArg,
		getFunctionOverloadDecl, // ModuleItemKind_FunctionOverload,
		getPropertyDecl,         // ModuleItemKind_Property,
		getPropertyTemplateDecl, // ModuleItemKind_PropertyTemplate,
		getEnumConstDecl,        // ModuleItemKind_EnumConst,
		getFieldDecl,            // ModuleItemKind_Field,
		getBaseTypeSlotDecl,     // ModuleItemKind_BaseTypeSlot,
		getOrphanDecl,           // ModuleItemKind_Orphan,
		getNullDecl,             // ModuleItemKind_LazyImport,
	};

	ASSERT((size_t)m_itemKind);
	return getDeclFuncTable[(size_t)m_itemKind](this);
}

Namespace*
ModuleItem::getNamespace()
{
	typedef
	Namespace*
	GetNamespaceFunc(ModuleItem* item);

	GetNamespaceFunc* getNamespaceFuncTable[ModuleItemKind__Count] =
	{
		getNullNamespace,             // ModuleItemKind_Undefined = 0,
		getNamespaceNamespace,        // ModuleItemKind_Namespace,
		getScopeNamespace,            // ModuleItemKind_Scope,
		getNullNamespace,             // ModuleItemKind_Attribute,
		getNullNamespace,             // ModuleItemKind_AttributeBlock,
		getTypeNamespace,             // ModuleItemKind_Type,
		getTypedefNamespace,          // ModuleItemKind_Typedef,
		getNullNamespace,             // ModuleItemKind_Alias,
		getNullNamespace,             // ModuleItemKind_Const,
		getNullNamespace,             // ModuleItemKind_Variable,
		getNullNamespace,             // ModuleItemKind_Function,
		getNullNamespace,             // ModuleItemKind_FunctionArg,
		getNullNamespace,             // ModuleItemKind_FunctionOverload,
		getPropertyNamespace,         // ModuleItemKind_Property,
		getPropertyTemplateNamespace, // ModuleItemKind_PropertyTemplate,
		getNullNamespace,             // ModuleItemKind_EnumConst,
		getNullNamespace,             // ModuleItemKind_Field,
		getNullNamespace,             // ModuleItemKind_BaseTypeSlot,
		getNullNamespace,             // ModuleItemKind_Orphan,
		getNullNamespace,             // ModuleItemKind_LazyImport,
	};

	ASSERT((size_t)m_itemKind);
	return getNamespaceFuncTable[(size_t)m_itemKind](this);
}

Type*
ModuleItem::getType()
{
	typedef
	Type*
	GetTypeFunc(ModuleItem* item);

	GetTypeFunc* getTypeFuncTable[ModuleItemKind__Count] =
	{
		getNullType,             // ModuleItemKind_Undefined = 0,
		getNullType,             // ModuleItemKind_Namespace,
		getNullType,             // ModuleItemKind_Scope,
		getNullType,             // ModuleItemKind_Attribute,
		getNullType,             // ModuleItemKind_AttributeBlock,
		getTypeType,             // ModuleItemKind_Type,
		getTypedefType,          // ModuleItemKind_Typedef,
		getAliasType,            // ModuleItemKind_Alias,
		getConstType,            // ModuleItemKind_Const,
		getVariableType,         // ModuleItemKind_Variable,
		getFunctionType,         // ModuleItemKind_Function,
		getFunctionArgType,      // ModuleItemKind_FunctionArg,
		getNullType,             // ModuleItemKind_FunctionOverload,
		getPropertyType,         // ModuleItemKind_Property,
		getPropertyTemplateType, // ModuleItemKind_PropertyTemplate,
		getEnumConstType,        // ModuleItemKind_EnumConst,
		getFieldType,            // ModuleItemKind_Field,
		getBaseTypeSlotType,     // ModuleItemKind_BaseTypeSlot,
		getOrphanType,           // ModuleItemKind_Orphan,
		getNullType,             // ModuleItemKind_LazyImport,
	};

	ASSERT((size_t)m_itemKind);
	return getTypeFuncTable[(size_t)m_itemKind](this);
}

sl::String
ModuleItem::createDoxyRefId()
{
	sl::String refId = getModuleItemKindString(m_itemKind);
	refId.replace('-', '_');

	ModuleItemDecl* decl = getDecl();
	ASSERT(decl);

	sl::String name = decl->getQualifiedName();
	if (!name.isEmpty())
	{
		refId.appendFormat("_%s", name.sz());
		refId.replace('.', '_');
	}

	refId.makeLowerCase();

	return m_module->m_doxyModule.adjustRefId(refId);
}

//..............................................................................

} // namespace ct
} // namespace jnc
