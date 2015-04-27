#include "pch.h"
#include "jnc_ModuleItem.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char*
getModuleItemKindString (ModuleItemKind itemKind)
{
	static const char* stringTable [ModuleItemKind__Count] =
	{
		"undefined-module-item-kind",  // ModuleItemKind_Undefined = 0,
		"namespace",                   // ModuleItemKind_Namespace,
		"scope",                       // ModuleItemKind_Scope,
		"type",                        // ModuleItemKind_Type,
		"typedef",                     // ModuleItemKind_Typedef,
		"alias",                       // ModuleItemKind_Alias,
		"const",                       // ModuleItemKind_Const,
		"variable",                    // ModuleItemKind_Variable,
		"function-arg",                // ModuleItemKind_FunctionArg,
		"function",                    // ModuleItemKind_Function,
		"property",                    // ModuleItemKind_Property,
		"property-template",           // ModuleItemKind_PropertyTemplate,
		"enum-member",                 // ModuleItemKind_EnumConst,
		"struct-member",               // ModuleItemKind_StructField,
		"base-type-slot",              // ModuleItemKind_BaseTypeSlot,
		"orphan",                      // ModuleItemKind_Orphan,
		"lazy",                        // ModuleItemKind_Lazy,
	};

	return (size_t) itemKind < ModuleItemKind__Count ?
		stringTable [itemKind] :
		stringTable [ModuleItemKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getStorageKindString (StorageKind storageKind)
{
	static const char* stringTable [StorageKind__Count] =
	{
		"undefined-storage-class",  // StorageKind_Undefined = 0,
		"typedef",                  // StorageKind_Typedef,
		"alias",                    // StorageKind_Alias,
		"static",                   // StorageKind_Static,
		"thread",                   // StorageKind_Thread,
		"stack",                    // StorageKind_Stack,
		"heap",                     // StorageKind_Heap,
		"member",                   // StorageKind_Member,
		"abstract",                 // StorageKind_Abstract,
		"virtual",                  // StorageKind_Virtual,
		"override",                 // StorageKind_Override,
		"mutable",                  // StorageKind_Mutable,
		"this",                     // StorageKind_This,
	};

	return (size_t) storageKind < StorageKind__Count ?
		stringTable [storageKind] :
		stringTable [StorageKind_Undefined];
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getAccessKindString (AccessKind accessKind)
{
	static const char* stringTable [AccessKind__Count] =
	{
		"undefined-access-kind", // AccessKind_Undefined = 0,
		"public",                // AccessKind_Public,
		"protected",             // AccessKind_Protected,
	};

	return (size_t) accessKind < AccessKind__Count ?
		stringTable [accessKind] :
		stringTable [AccessKind_Undefined];
}

//.............................................................................

rtl::String
ModuleItemInitializer::getInitializerString ()
{
	if (m_initializer.isEmpty ())
		return rtl::String ();

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
	m_itemDecl = NULL;
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

Type*
verifyModuleItemIsType (
	ModuleItem* item,
	const char* name
	)
{
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Type)
	{
		err::setFormatStringError ("'%s' is not a type", name);
		return NULL;
	}

	return (Type*) item;
}

DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Type || 
		!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Derivable))
	{
		err::setFormatStringError ("'%s' is not a derivable type", name);
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
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Type || 
		((Type*) item)->getTypeKind () != TypeKind_Class)
	{
		err::setFormatStringError ("'%s' is not a class", name);
		return NULL;
	}

	return (ClassType*) item;
}

Function*
verifyModuleItemIsFunction (
	ModuleItem* item,
	const char* name
	)
{
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Function)
	{
		err::setFormatStringError ("'%s' is not a function", name);
		return NULL;
	}

	return (Function*) item;
}

Property*
verifyModuleItemIsProperty (
	ModuleItem* item,
	const char* name
	)
{
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Property)
	{
		err::setFormatStringError ("'%s' is not a property", name);
		return NULL;
	}

	return (Property*) item;
}

//.............................................................................

} // namespace jnc {
