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

#define _JNC_MODULEITEM_H

#include "jnc_Def.h"

/// \addtogroup module-item
/// @{

//..............................................................................

enum jnc_ModuleItemKind
{
	jnc_ModuleItemKind_Undefined = 0,
	jnc_ModuleItemKind_Namespace,
	jnc_ModuleItemKind_Scope,
	jnc_ModuleItemKind_Type,
	jnc_ModuleItemKind_Typedef,
	jnc_ModuleItemKind_Alias,
	jnc_ModuleItemKind_Const,
	jnc_ModuleItemKind_Variable,
	jnc_ModuleItemKind_FunctionArg,
	jnc_ModuleItemKind_Function,
	jnc_ModuleItemKind_Property,
	jnc_ModuleItemKind_PropertyTemplate,
	jnc_ModuleItemKind_EnumConst,
	jnc_ModuleItemKind_StructField,
	jnc_ModuleItemKind_BaseTypeSlot,
	jnc_ModuleItemKind_Orphan,
	jnc_ModuleItemKind_Lazy,
	jnc_ModuleItemKind__Count,
};

typedef enum jnc_ModuleItemKind jnc_ModuleItemKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getModuleItemKindString (jnc_ModuleItemKind itemKind);

//..............................................................................

enum jnc_ModuleItemFlag
{
	jnc_ModuleItemFlag_User         = 0x01,
	jnc_ModuleItemFlag_NeedLayout   = 0x02,
	jnc_ModuleItemFlag_NeedCompile  = 0x04,
	jnc_ModuleItemFlag_InCalcLayout = 0x10,
	jnc_ModuleItemFlag_LayoutReady  = 0x20,
	jnc_ModuleItemFlag_Constructed  = 0x40, // fields, properties, base type slots
	jnc_ModuleItemFlag_Sealed       = 0x80,
};

typedef enum jnc_ModuleItemFlag jnc_ModuleItemFlag;

//..............................................................................

enum jnc_StorageKind
{
	jnc_StorageKind_Undefined = 0,
	jnc_StorageKind_Alias,
	jnc_StorageKind_Typedef,
	jnc_StorageKind_Static,
	jnc_StorageKind_Tls,
	jnc_StorageKind_Stack,
	jnc_StorageKind_Heap,
	jnc_StorageKind_Member,
	jnc_StorageKind_Abstract,
	jnc_StorageKind_Virtual,
	jnc_StorageKind_Override,
	jnc_StorageKind_Mutable,
	jnc_StorageKind_Disposable,
	jnc_StorageKind_This,
	jnc_StorageKind__Count,
};

typedef enum jnc_StorageKind jnc_StorageKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getStorageKindString (jnc_StorageKind storageKind);

//..............................................................................

enum jnc_AccessKind
{
	jnc_AccessKind_Undefined = 0,
	jnc_AccessKind_Public,
	jnc_AccessKind_Protected,
	jnc_AccessKind__Count,
};

typedef enum jnc_AccessKind jnc_AccessKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getAccessKindString (jnc_AccessKind accessKind);

//..............................................................................

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getName (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
const char*
jnc_ModuleItemDecl_getQualifiedName (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
jnc_StorageKind
jnc_ModuleItemDecl_getStorageKind (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
jnc_AccessKind
jnc_ModuleItemDecl_getAccessKind (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
jnc_AttributeBlock*
jnc_ModuleItemDecl_getAttributeBlock (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItemDecl_getParentNamespace (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
jnc_Unit*
jnc_ModuleItemDecl_getParentUnit (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getLine (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
int
jnc_ModuleItemDecl_getCol (jnc_ModuleItemDecl* decl);

JNC_EXTERN_C
size_t
jnc_ModuleItemDecl_getOffset (jnc_ModuleItemDecl* decl);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_ModuleItemDecl
{
	const char*
	getName ()
	{
		return jnc_ModuleItemDecl_getName (this);
	}

	const char*
	getQualifiedName ()
	{
		return jnc_ModuleItemDecl_getQualifiedName (this);
	}

	jnc_StorageKind
	getStorageKind (jnc_ModuleItemDecl* decl)
	{
		return jnc_ModuleItemDecl_getStorageKind (this);
	}

	jnc_AccessKind
	getAccessKind ()
	{
		return jnc_ModuleItemDecl_getAccessKind (this);
	}

	jnc_AttributeBlock*
	getAttributeBlock ()
	{
		return jnc_ModuleItemDecl_getAttributeBlock (this);
	}

	jnc_Namespace*
	getParentNamespace ()
	{
		return jnc_ModuleItemDecl_getParentNamespace (this);
	}

	jnc_Unit*
	getParentUnit ()
	{
		return jnc_ModuleItemDecl_getParentUnit (this);
	}

	int
	getLine ()
	{
		return jnc_ModuleItemDecl_getLine (this);
	}

	int
	getCol ()
	{
		return jnc_ModuleItemDecl_getCol (this);
	}

	size_t
	getOffset ()
	{
		return jnc_ModuleItemDecl_getOffset (this);
	}

};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Module*
jnc_ModuleItem_getModule (jnc_ModuleItem* item);

JNC_EXTERN_C
jnc_ModuleItemKind
jnc_ModuleItem_getItemKind (jnc_ModuleItem* item);

JNC_EXTERN_C
uint_t
jnc_ModuleItem_getFlags (jnc_ModuleItem* item);

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_ModuleItem_getDecl (jnc_ModuleItem* item);

JNC_EXTERN_C
jnc_Namespace*
jnc_ModuleItem_getNamespace (jnc_ModuleItem* item);

JNC_EXTERN_C
jnc_Type*
jnc_ModuleItem_getType (jnc_ModuleItem* item);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_ModuleItem
{
	jnc_Module*
	getModule ()
	{
		return jnc_ModuleItem_getModule (this);
	}

	jnc_ModuleItemKind
	getItemKind ()
	{
		return jnc_ModuleItem_getItemKind (this);
	}

	uint_t
	getFlags ()
	{
		return jnc_ModuleItem_getFlags (this);
	}

	jnc_ModuleItemDecl*
	getDecl ()
	{
		return jnc_ModuleItem_getDecl (this);
	}

	jnc_Namespace*
	getNamespace ()
	{
		return jnc_ModuleItem_getNamespace (this);
	}

	jnc_Type*
	getType ()
	{
		return jnc_ModuleItem_getType (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	);

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_ModuleItemKind ModuleItemKind;

const ModuleItemKind
	ModuleItemKind_Undefined        = jnc_ModuleItemKind_Undefined,
	ModuleItemKind_Namespace        = jnc_ModuleItemKind_Namespace,
	ModuleItemKind_Scope            = jnc_ModuleItemKind_Scope,
	ModuleItemKind_Type             = jnc_ModuleItemKind_Type,
	ModuleItemKind_Typedef          = jnc_ModuleItemKind_Typedef,
	ModuleItemKind_Alias            = jnc_ModuleItemKind_Alias,
	ModuleItemKind_Const            = jnc_ModuleItemKind_Const,
	ModuleItemKind_Variable         = jnc_ModuleItemKind_Variable,
	ModuleItemKind_FunctionArg      = jnc_ModuleItemKind_FunctionArg,
	ModuleItemKind_Function         = jnc_ModuleItemKind_Function,
	ModuleItemKind_Property         = jnc_ModuleItemKind_Property,
	ModuleItemKind_PropertyTemplate = jnc_ModuleItemKind_PropertyTemplate,
	ModuleItemKind_EnumConst        = jnc_ModuleItemKind_EnumConst,
	ModuleItemKind_StructField      = jnc_ModuleItemKind_StructField,
	ModuleItemKind_BaseTypeSlot     = jnc_ModuleItemKind_BaseTypeSlot,
	ModuleItemKind_Orphan           = jnc_ModuleItemKind_Orphan,
	ModuleItemKind_Lazy             = jnc_ModuleItemKind_Lazy,
	ModuleItemKind__Count           = jnc_ModuleItemKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
const char*
getModuleItemKindString (ModuleItemKind itemKind)
{
	return jnc_getModuleItemKindString (itemKind);
}

//..............................................................................

typedef jnc_ModuleItemFlag ModuleItemFlag;

const ModuleItemFlag
	ModuleItemFlag_User         = jnc_ModuleItemFlag_User,
	ModuleItemFlag_NeedLayout   = jnc_ModuleItemFlag_NeedLayout,
	ModuleItemFlag_NeedCompile  = jnc_ModuleItemFlag_NeedCompile,
	ModuleItemFlag_InCalcLayout = jnc_ModuleItemFlag_InCalcLayout,
	ModuleItemFlag_LayoutReady  = jnc_ModuleItemFlag_LayoutReady,
	ModuleItemFlag_Constructed  = jnc_ModuleItemFlag_Constructed,
	ModuleItemFlag_Sealed       = jnc_ModuleItemFlag_Sealed;

//..............................................................................

typedef jnc_StorageKind StorageKind;

const StorageKind
	StorageKind_Undefined  = jnc_StorageKind_Undefined,
	StorageKind_Alias      = jnc_StorageKind_Alias,
	StorageKind_Typedef    = jnc_StorageKind_Typedef,
	StorageKind_Static     = jnc_StorageKind_Static,
	StorageKind_Tls        = jnc_StorageKind_Tls,
	StorageKind_Stack      = jnc_StorageKind_Stack,
	StorageKind_Heap       = jnc_StorageKind_Heap,
	StorageKind_Member     = jnc_StorageKind_Member,
	StorageKind_Abstract   = jnc_StorageKind_Abstract,
	StorageKind_Virtual    = jnc_StorageKind_Virtual,
	StorageKind_Override   = jnc_StorageKind_Override,
	StorageKind_Mutable    = jnc_StorageKind_Mutable,
	StorageKind_Disposable = jnc_StorageKind_Disposable,
	StorageKind_This       = jnc_StorageKind_This,
	StorageKind__Count     = jnc_StorageKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
const char*
getStorageKindString (StorageKind storageKind)
{
	return jnc_getStorageKindString (storageKind);
}

//..............................................................................

typedef jnc_AccessKind AccessKind;

const AccessKind
	AccessKind_Undefined = jnc_AccessKind_Undefined,
	AccessKind_Public    = jnc_AccessKind_Public,
	AccessKind_Protected = jnc_AccessKind_Protected,
	AccessKind__Count    = jnc_AccessKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
const char*
getAccessKindString (AccessKind accessKind)
{
	return jnc_getAccessKindString (accessKind);
}

//..............................................................................

JNC_INLINE
jnc_DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return jnc_verifyModuleItemIsDerivableType (item, name);
}

JNC_INLINE
jnc_ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	return jnc_verifyModuleItemIsClassType (item, name);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
