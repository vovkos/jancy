#include "pch.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

sl::String
ModuleItemInitializer::getInitializerString ()
{
	if (m_initializer.isEmpty ())
		return sl::String ();

	if (m_initializerString.isEmpty ())
		m_initializerString = Token::getTokenListString (m_initializer);

	return m_initializerString;
}

//.............................................................................

ModuleItemDecl::ModuleItemDecl ()
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Public; // public by default
	m_parentNamespace = NULL;
	m_attributeBlock = NULL;
}

//.............................................................................

ModuleItem::ModuleItem ()
{
	m_module = NULL;
	m_itemKind = ModuleItemKind_Undefined;
	m_flags = 0;
}

bool
ModuleItem::ensureLayout ()
{
	bool result;

	if (m_flags & ModuleItemFlag_LayoutReady)
		return true;

	if (m_flags & ModuleItemFlag_InCalcLayout)
	{
		err::setFormatStringError ("can't calculate layout of '%s' due to recursion", m_tag.cc ());
		return false;
	}

	m_flags |= ModuleItemFlag_InCalcLayout;

	result = calcLayout ();

	m_flags &= ~ModuleItemFlag_InCalcLayout;

	if (!result)
		return false;

	m_flags |= ModuleItemFlag_LayoutReady;
	return true;
}

//.............................................................................

ModuleItem*
verifyModuleItemKind (
	ModuleItem* item, 
	ModuleItemKind itemKind,
	const char* name
	)
{
	if (item->getItemKind () != itemKind)
	{
		err::setFormatStringError ("'%s' is not a %s", name, getModuleItemKindString (itemKind));
		return NULL;
	}

	return item;
}

DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	Type* type = (Type*) verifyModuleItemKind (item, ModuleItemKind_Type, name);
	if (!type)
		return NULL;

	if (!(type->getTypeKindFlags () & TypeKindFlag_Derivable))
	{
		err::setFormatStringError ("'%s' is not a derivable type", type->getTypeString ().cc ());
		return NULL;
	}

	return (DerivableType*) item;
}

ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	Type* type = (Type*) verifyModuleItemKind (item, ModuleItemKind_Type, name);
	if (!type)
		return NULL;

	if (type->getTypeKind () != TypeKind_Class)
	{
		err::setFormatStringError ("'%s' is not a class type", type->getTypeString ().cc ());
		return NULL;
	}

	return (ClassType*) item;
}

//.............................................................................

} // namespace ct
} // namespace jnc
