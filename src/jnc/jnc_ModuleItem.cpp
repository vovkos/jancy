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
		"undefined-module-item-kind",  // EModuleItem_Undefined = 0,
		"namespace",                   // EModuleItem_Namespace,
		"scope",                       // EModuleItem_Scope,
		"type",                        // EModuleItem_Type,
		"typedef",                     // EModuleItem_Typedef,
		"alias",                       // EModuleItem_Alias,
		"const",                       // EModuleItem_Const,
		"variable",                    // EModuleItem_Variable,
		"function-arg",                // EModuleItem_FunctionArg,
		"function",                    // EModuleItem_Function,
		"property",                    // EModuleItem_Property,
		"property-template",           // EModuleItem_PropertyTemplate,
		"enum-member",                 // EModuleItem_EnumConst,
		"struct-member",               // EModuleItem_StructField,
		"base-type-slot",              // EModuleItem_BaseTypeSlot,
		"orphan",                      // EModuleItem_Orphan,
		"lazy",                        // EModuleItem_Lazy,
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
		"undefined-storage-class",  // EStorage_Undefined = 0,
		"typedef",                  // EStorage_Typedef,
		"alias",                    // EStorage_Alias,
		"static",                   // EStorage_Static,
		"thread",                   // EStorage_Thread,
		"stack",                    // EStorage_Stack,
		"heap",                     // EStorage_Heap,
		"uheap",                    // EStorage_UHeap,
		"member",                   // EStorage_Member,
		"abstract",                 // EStorage_Abstract,
		"virtual",                  // EStorage_Virtual,
		"override",                 // EStorage_Override,
		"mutable",                  // EStorage_Mutable,
		"this",                     // EStorage_This,
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
		"undefined-access-kind", // EAccess_Undefined = 0,
		"public",                // EAccess_Public,
		"protected",             // EAccess_Protected,
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

ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	if (!item)
		return NULL;

	if (item->getItemKind () != ModuleItemKind_Type || ((Type*) item)->getTypeKind () != TypeKind_Class)
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
