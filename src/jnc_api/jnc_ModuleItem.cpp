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
#include "jnc_ModuleItem.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

JNC_EXTERN_C
const char*
jnc_getModuleItemKindString (jnc_ModuleItemKind itemKind)
{
	static const char* stringTable [jnc_ModuleItemKind__Count] =
	{
		"undefined-module-item-kind",  // jnc_ModuleItemKind_Undefined = 0,
		"namespace",                   // jnc_ModuleItemKind_Namespace,
		"scope",                       // jnc_ModuleItemKind_Scope,
		"type",                        // jnc_ModuleItemKind_Type,
		"typedef",                     // jnc_ModuleItemKind_Typedef,
		"alias",                       // jnc_ModuleItemKind_Alias,
		"const",                       // jnc_ModuleItemKind_Const,
		"variable",                    // jnc_ModuleItemKind_Variable,
		"function-arg",                // jnc_ModuleItemKind_FunctionArg,
		"function",                    // jnc_ModuleItemKind_Function,
		"property",                    // jnc_ModuleItemKind_Property,
		"property-template",           // jnc_ModuleItemKind_PropertyTemplate,
		"enum-member",                 // jnc_ModuleItemKind_EnumConst,
		"struct-member",               // jnc_ModuleItemKind_StructField,
		"base-type-slot",              // jnc_ModuleItemKind_BaseTypeSlot,
		"orphan",                      // jnc_ModuleItemKind_Orphan,
		"lazy",                        // jnc_ModuleItemKind_Lazy,
	};

	return (size_t) itemKind < jnc_ModuleItemKind__Count ?
		stringTable [itemKind] :
		stringTable [jnc_ModuleItemKind_Undefined];
}

JNC_EXTERN_C
const char*
jnc_getStorageKindString (jnc_StorageKind storageKind)
{
	static const char* stringTable [jnc_StorageKind__Count] =
	{
		"undefined-storage-class",  // StorageKind_Undefined = 0,
		"alias",                    // StorageKind_Alias,
		"typedef",                  // StorageKind_Typedef,
		"static",                   // StorageKind_Static,
		"threadlocal",              // StorageKind_Tls,
		"stack",                    // StorageKind_Stack,
		"heap",                     // StorageKind_Heap,
		"member",                   // StorageKind_Member,
		"abstract",                 // StorageKind_Abstract,
		"virtual",                  // StorageKind_Virtual,
		"override",                 // StorageKind_Override,
		"mutable",                  // StorageKind_Mutable,
		"disposable",               // StorageKind_Disposable,
		"this",                     // StorageKind_This,
	};

	return (size_t) storageKind < jnc_StorageKind__Count ?
		stringTable [storageKind] :
		stringTable [jnc_StorageKind_Undefined];
}

JNC_EXTERN_C
const char*
jnc_getAccessKindString (jnc_AccessKind accessKind)
{
	static const char* stringTable [jnc_AccessKind__Count] =
	{
		"undefined-access-kind", // AccessKind_Undefined = 0,
		"public",                // AccessKind_Public,
		"protected",             // AccessKind_Protected,
	};

	return (size_t) accessKind < jnc_AccessKind__Count ?
		stringTable [accessKind] :
		stringTable [jnc_AccessKind_Undefined];
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getName (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getNameFunc (decl);
}

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getQualifiedName (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getQualifiedNameFunc (decl);
}

JNC_EXTERN_C
jnc_StorageKind
jnc_ModuleItemDecl_getStorageKind (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getStorageKindFunc (decl);
}

JNC_EXTERN_C
jnc_AccessKind
jnc_ModuleItemDecl_getAccessKind (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getAccessKindFunc (decl);
}

JNC_EXTERN_C
jnc_AttributeBlock*
jnc_ModuleItemDecl_getAttributeBlock (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getAttributeBlockFunc (decl);
}

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItemDecl_getParentNamespace (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getParentNamespaceFunc (decl);
}

JNC_EXTERN_C
jnc_Unit*
jnc_ModuleItemDecl_getParentUnit (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getParentUnitFunc (decl);
}

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getLine (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getLineFunc (decl);
}

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getCol (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getColFunc (decl);
}

JNC_EXTERN_C
size_t
jnc_ModuleItemDecl_getOffset (jnc_ModuleItemDecl* decl)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemDeclFuncTable->m_getOffsetFunc (decl);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Module*
jnc_ModuleItem_getModule (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getModuleFunc (item);
}

JNC_EXTERN_C
jnc_ModuleItemKind
jnc_ModuleItem_getItemKind (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getItemKindFunc (item);
}

JNC_EXTERN_C
uint_t
jnc_ModuleItem_getFlags (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getFlagsFunc (item);
}

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_ModuleItem_getDecl (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getDeclFunc (item);
}

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItem_getNamespace (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getNamespaceFunc (item);
}

JNC_EXTERN_C
jnc_Type*
jnc_ModuleItem_getType (jnc_ModuleItem* item)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_getTypeFunc (item);
}

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_verifyModuleItemIsDerivableTypeFunc (item, name);
}

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleItemFuncTable->m_verifyModuleItemIsClassTypeFunc (item, name);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getName (jnc_ModuleItemDecl* decl)
{
	return decl->getName ();
}

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getQualifiedName (jnc_ModuleItemDecl* decl)
{
	return decl->getQualifiedName ();
}

JNC_EXTERN_C
jnc_StorageKind
jnc_ModuleItemDecl_getStorageKind (jnc_ModuleItemDecl* decl)
{
	return decl->getStorageKind ();
}

JNC_EXTERN_C
jnc_AccessKind
jnc_ModuleItemDecl_getAccessKind (jnc_ModuleItemDecl* decl)
{
	return decl->getAccessKind ();
}

JNC_EXTERN_C
jnc_AttributeBlock*
jnc_ModuleItemDecl_getAttributeBlock (jnc_ModuleItemDecl* decl)
{
	return decl->getAttributeBlock ();
}

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItemDecl_getParentNamespace (jnc_ModuleItemDecl* decl)
{
	return decl->getParentNamespace ();
}

JNC_EXTERN_C
jnc_Unit*
jnc_ModuleItemDecl_getParentUnit (jnc_ModuleItemDecl* decl)
{
	return decl->getParentUnit ();
}

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getLine (jnc_ModuleItemDecl* decl)
{
	return decl->getPos ()->m_line;
}

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getCol (jnc_ModuleItemDecl* decl)
{
	return decl->getPos ()->m_col;
}

JNC_EXTERN_C
size_t
jnc_ModuleItemDecl_getOffset (jnc_ModuleItemDecl* decl)
{
	return decl->getPos ()->m_offset;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Module*
jnc_ModuleItem_getModule (jnc_ModuleItem* item)
{
	return item->getModule ();
}

JNC_EXTERN_C
jnc_ModuleItemKind
jnc_ModuleItem_getItemKind (jnc_ModuleItem* item)
{
	return item->getItemKind ();
}

JNC_EXTERN_C
uint_t
jnc_ModuleItem_getFlags (jnc_ModuleItem* item)
{
	return item->getFlags ();
}

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_ModuleItem_getDecl (jnc_ModuleItem* item)
{
	return item->getDecl ();
}

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItem_getNamespace (jnc_ModuleItem* item)
{
	return item->getNamespace ();
}

JNC_EXTERN_C
jnc_Type*
jnc_ModuleItem_getType (jnc_ModuleItem* item)
{
	return item->getType ();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc::ct::verifyModuleItemIsDerivableType (item, name);
}

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc::ct::verifyModuleItemIsClassType (item, name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
