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

sl::String
ModuleItemDecl::createDoxyLocationString ()
{
	if (!m_parentUnit)
		return sl::String ();

	sl::String string;

	string.format ("<location file='%s' line='%d' col='%d'/>",
		m_parentUnit->getFileName ().cc (),
		m_pos.m_line + 1,
		m_pos.m_col + 1
		);

	return string;
}

//.............................................................................

ModuleItem::ModuleItem ()
{
	m_module = NULL;
	m_itemKind = ModuleItemKind_Undefined;
	m_doxyBlock = NULL;
	m_flags = 0;
}

ModuleItemDecl*
ModuleItem::getDecl ()
{
	switch (m_itemKind)
	{
	case ModuleItemKind_Namespace:
		return (GlobalNamespace*) this;

	case ModuleItemKind_Scope:
		return (Scope*) this;

	case ModuleItemKind_Type:
		return (((Type*) this)->getTypeKindFlags () & TypeKindFlag_Named) ? 
			(NamedType*) this :
			NULL;

	case ModuleItemKind_Typedef:
		return (Typedef*) this;

	case ModuleItemKind_Alias:
		return (Alias*) this;

	case ModuleItemKind_Const:
		return (Const*) this;

	case ModuleItemKind_Variable:
		return (Variable*) this;

	case ModuleItemKind_FunctionArg:
		return (FunctionArg*) this;

	case ModuleItemKind_Function:
		return (Function*) this;

	case ModuleItemKind_Property:
		return (Property*) this;

	case ModuleItemKind_PropertyTemplate:
		return (PropertyTemplate*) this;

	case ModuleItemKind_EnumConst:
		return (EnumConst*) this;

	case ModuleItemKind_StructField:
		return (StructField*) this;

	case ModuleItemKind_BaseTypeSlot:
		return (BaseTypeSlot*) this;

	case ModuleItemKind_Orphan:
		return (Orphan*) this;

	default:
		return NULL;
	}
}

Namespace*
ModuleItem::getNamespace ()
{
	switch (m_itemKind)
	{
	case ModuleItemKind_Namespace:
		return (GlobalNamespace*) this;

	case ModuleItemKind_Scope:
		return (Scope*) this;

	case ModuleItemKind_Typedef:
		return ((Typedef*) this)->getType ()->getNamespace ();

	case ModuleItemKind_Type:
		return (((Type*) this)->getTypeKindFlags () & TypeKindFlag_Named) ? 
			(NamedType*) this :
			NULL;

	case ModuleItemKind_Property:
		return (Property*) this;

	case ModuleItemKind_PropertyTemplate:
		return (PropertyTemplate*) this;

	default:
		return NULL;
	}
}

Type*
ModuleItem::getType ()
{
	using namespace jnc;

	switch (m_itemKind)
	{
	case ModuleItemKind_Type:
		return (ct::Type*) this;

	case ModuleItemKind_Typedef:
		return ((ct::Typedef*) this)->getType ();

	case ModuleItemKind_Alias:
		return ((ct::Alias*) this)->getType ();

	case ModuleItemKind_Const:
		return ((ct::Const*) this)->getValue ().getType ();

	case ModuleItemKind_Variable:
		return ((ct::Variable*) this)->getType ();

	case ModuleItemKind_FunctionArg:
		return ((ct::FunctionArg*) this)->getType ();

	case ModuleItemKind_Function:
		return ((ct::Function*) this)->getType ();

	case ModuleItemKind_Property:
		return ((ct::Property*) this)->getType ();

	case ModuleItemKind_PropertyTemplate:
		return ((ct::PropertyTemplate*) this)->calcType ();

	case ModuleItemKind_EnumConst:
		return ((ct::EnumConst*) this)->getParentEnumType ();

	case ModuleItemKind_StructField:
		return ((ct::StructField*) this)->getType ();

	case ModuleItemKind_BaseTypeSlot:
		return ((ct::BaseTypeSlot*) this)->getType ();

	case ModuleItemKind_Orphan:
		return ((ct::Orphan*) this)->getFunctionType ();

	case ModuleItemKind_Lazy:
		return jnc_ModuleItem_getType (((ct::LazyModuleItem*) this)->getActualItem ());

	default:
		return NULL;
	}
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

DoxyBlock* 
ModuleItem::getDoxyBlock ()
{
	if (!m_doxyBlock)
		m_doxyBlock = m_module->m_doxyMgr.createDoxyBlock ();

	if (m_doxyBlock->m_refId.isEmpty ())
		m_doxyBlock->m_refId = createDoxyRefId ();

	return m_doxyBlock;
}

sl::String
ModuleItem::createDoxyRefId ()
{
	#pragma AXL_TODO ("generate more meaningful doxygen refid")

	sl::String refId = getModuleItemKindString (m_itemKind);
	refId.replace ('-', '_');
	
	return m_module->m_doxyMgr.adjustDoxyRefId (refId);
}

sl::String
ModuleItem::createDoxyDescriptionString ()
{
	sl::String string;
	DoxyBlock* doxyBlock = getDoxyBlock ();

	sl::String description = doxyBlock->getBriefDescription ();
	if (!description.isEmpty ())
	{
		string.append ("<briefdescription><para>\n");
		string.append (description);
		string.append ("</para></briefdescription>\n");
	}

	description = doxyBlock->getDetailedDescription ();
	if (!description.isEmpty ())
	{
		string.append ("<detaileddescription><para>\n");
		string.append (description);
		string.append ("</para></detaileddescription>\n");
	}

	return string;
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
