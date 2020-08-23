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

#define _JNC_DYNAMICEXTENSIONLIBHOST_H

#include "jnc_RuntimeStructs.h"
#include "jnc_Namespace.h"
#include "jnc_Function.h"
#include "jnc_ClassType.h"
#include "jnc_Variant.h"
#include "jnc_Promise.h"
#include "jnc_OpKind.h"
#include "jnc_StdHashTable.h"
#include "jnc_StdRbTree.h"

typedef struct jnc_ErrorFuncTable jnc_ErrorFuncTable;
typedef struct jnc_ModuleItemDeclFuncTable jnc_ModuleItemDeclFuncTable;
typedef struct jnc_ModuleItemFuncTable jnc_ModuleItemFuncTable;
typedef struct jnc_AttributeFuncTable jnc_AttributeFuncTable;
typedef struct jnc_AttributeBlockFuncTable jnc_AttributeBlockFuncTable;
typedef struct jnc_NamespaceFuncTable jnc_NamespaceFuncTable;
typedef struct jnc_GlobalNamespaceFuncTable jnc_GlobalNamespaceFuncTable;
typedef struct jnc_VariableFuncTable jnc_VariableFuncTable;
typedef struct jnc_FunctionFuncTable jnc_FunctionFuncTable;
typedef struct jnc_FunctionOverloadFuncTable jnc_FunctionOverloadFuncTable;
typedef struct jnc_PropertyFuncTable jnc_PropertyFuncTable;
typedef struct jnc_TypedefFuncTable jnc_TypedefFuncTable;
typedef struct jnc_TypeFuncTable jnc_TypeFuncTable;
typedef struct jnc_NamedTypeFuncTable jnc_NamedTypeFuncTable;
typedef struct jnc_BaseTypeSlotFuncTable jnc_BaseTypeSlotFuncTable;
typedef struct jnc_DerivableTypeFuncTable jnc_DerivableTypeFuncTable;
typedef struct jnc_ArrayTypeFuncTable jnc_ArrayTypeFuncTable;
typedef struct jnc_BitFieldTypeFuncTable jnc_BitFieldTypeFuncTable;
typedef struct jnc_FunctionArgFuncTable jnc_FunctionArgFuncTable;
typedef struct jnc_FunctionTypeFuncTable jnc_FunctionTypeFuncTable;
typedef struct jnc_PropertyTypeFuncTable jnc_PropertyTypeFuncTable;
typedef struct jnc_EnumConstFuncTable jnc_EnumConstFuncTable;
typedef struct jnc_EnumTypeFuncTable jnc_EnumTypeFuncTable;
typedef struct jnc_FieldFuncTable jnc_FieldFuncTable;
typedef struct jnc_StructTypeFuncTable jnc_StructTypeFuncTable;
typedef struct jnc_UnionTypeFuncTable jnc_UnionTypeFuncTable;
typedef struct jnc_ClassTypeFuncTable jnc_ClassTypeFuncTable;
typedef struct jnc_MulticastClassTypeFuncTable jnc_MulticastClassTypeFuncTable;
typedef struct jnc_McSnapshotClassTypeFuncTable jnc_McSnapshotClassTypeFuncTable;
typedef struct jnc_DataPtrTypeFuncTable jnc_DataPtrTypeFuncTable;
typedef struct jnc_ClassPtrTypeFuncTable jnc_ClassPtrTypeFuncTable;
typedef struct jnc_FunctionPtrTypeFuncTable jnc_FunctionPtrTypeFuncTable;
typedef struct jnc_PropertyPtrTypeFuncTable jnc_PropertyPtrTypeFuncTable;
typedef struct jnc_VariantFuncTable jnc_VariantFuncTable;
typedef struct jnc_PromiseFuncTable jnc_PromiseFuncTable;
typedef struct jnc_UnitFuncTable jnc_UnitFuncTable;
typedef struct jnc_ModuleFuncTable jnc_ModuleFuncTable;
typedef struct jnc_RuntimeFuncTable jnc_RuntimeFuncTable;
typedef struct jnc_GcHeapFuncTable jnc_GcHeapFuncTable;
typedef struct jnc_StdHashTableFuncTable jnc_StdHashTableFuncTable;
typedef struct jnc_StdRbTreeFuncTable jnc_StdRbTreeFuncTable;
typedef struct jnc_DynamicExtensionLibHost jnc_DynamicExtensionLibHost;

//..............................................................................

// Error

typedef
const jnc_Error*
jnc_GetLastErrorFunc();

typedef
void
jnc_SetErrorFunc(const jnc_Error* error);

typedef
void
jnc_SetErrnoFunc(int code);

typedef
void
jnc_SetStringErrorFunc(const char* string);

typedef
const char*
jnc_GetErrorDescriptionFunc(const jnc_Error* error);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ErrorFuncTable
{
	size_t m_size;
	jnc_GetLastErrorFunc* m_getLastErrorFunc;
	jnc_SetErrorFunc* m_setErrorFunc;
	jnc_SetErrnoFunc* m_setErrnoFunc;
	jnc_SetStringErrorFunc* m_setStringErrorFunc;
	jnc_GetErrorDescriptionFunc* m_getErrorDescriptionFunc;
};

//..............................................................................

// ModuleItemDecl

typedef
const char*
jnc_ModuleItemDecl_GetNameFunc(jnc_ModuleItemDecl* decl);

typedef
const char*
jnc_ModuleItemDecl_GetQualifiedNameFunc(jnc_ModuleItemDecl* decl);

typedef
jnc_StorageKind
jnc_ModuleItemDecl_GetStorageKindFunc(jnc_ModuleItemDecl* decl);

typedef
jnc_AccessKind
jnc_ModuleItemDecl_GetAccessKindFunc(jnc_ModuleItemDecl* decl);

typedef
jnc_AttributeBlock*
jnc_ModuleItemDecl_GetAttributeBlockFunc(jnc_ModuleItemDecl* decl);

typedef
jnc_Namespace*
jnc_ModuleItemDecl_GetParentNamespaceFunc(jnc_ModuleItemDecl* decl);

typedef
jnc_Unit*
jnc_ModuleItemDecl_GetParentUnitFunc(jnc_ModuleItemDecl* decl);

typedef
int
jnc_ModuleItemDecl_GetLineFunc(jnc_ModuleItemDecl* decl);

typedef
int
jnc_ModuleItemDecl_GetColFunc(jnc_ModuleItemDecl* decl);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleItemDeclFuncTable
{
	size_t m_size;
	jnc_ModuleItemDecl_GetNameFunc* m_getNameFunc;
	jnc_ModuleItemDecl_GetQualifiedNameFunc* m_getQualifiedNameFunc;
	jnc_ModuleItemDecl_GetStorageKindFunc* m_getStorageKindFunc;
	jnc_ModuleItemDecl_GetAccessKindFunc* m_getAccessKindFunc;
	jnc_ModuleItemDecl_GetAttributeBlockFunc* m_getAttributeBlockFunc;
	jnc_ModuleItemDecl_GetParentNamespaceFunc* m_getParentNamespaceFunc;
	jnc_ModuleItemDecl_GetParentUnitFunc* m_getParentUnitFunc;
	jnc_ModuleItemDecl_GetLineFunc* m_getLineFunc;
	jnc_ModuleItemDecl_GetColFunc* m_getColFunc;
};

//..............................................................................

// ModuleItem

typedef
jnc_Module*
jnc_ModuleItem_GetModuleFunc(jnc_ModuleItem* item);

typedef
jnc_ModuleItemKind
jnc_ModuleItem_GetItemKindFunc(jnc_ModuleItem* item);

typedef
uint_t
jnc_ModuleItem_GetFlagsFunc(jnc_ModuleItem* item);

typedef
jnc_ModuleItemDecl*
jnc_ModuleItem_GetDeclFunc(jnc_ModuleItem* item);

typedef
jnc_Namespace*
jnc_ModuleItem_GetNamespaceFunc(jnc_ModuleItem* item);

typedef
jnc_Type*
jnc_ModuleItem_GetTypeFunc(jnc_ModuleItem* item);

typedef
bool_t
jnc_ModuleItem_RequireFunc(jnc_ModuleItem* item);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleItemFuncTable
{
	size_t m_size;
	jnc_ModuleItem_GetModuleFunc* m_getModuleFunc;
	jnc_ModuleItem_GetItemKindFunc* m_getItemKindFunc;
	jnc_ModuleItem_GetFlagsFunc* m_getFlagsFunc;
	jnc_ModuleItem_GetDeclFunc* m_getDeclFunc;
	jnc_ModuleItem_GetNamespaceFunc* m_getNamespaceFunc;
	jnc_ModuleItem_GetTypeFunc* m_getTypeFunc;
	jnc_ModuleItem_RequireFunc* m_requireFunc;
};

//..............................................................................

// Attribute

struct jnc_AttributeFuncTable
{
	size_t m_size;
};

//..............................................................................

// AttributeBlock

typedef
size_t
jnc_AttributeBlock_GetAttributeCountFunc(jnc_AttributeBlock* block);

typedef
jnc_Attribute*
jnc_AttributeBlock_GetAttributeFunc(
	jnc_AttributeBlock* block,
	size_t index
	);

typedef
jnc_Attribute*
jnc_AttributeBlock_FindAttributeFunc(
	jnc_AttributeBlock* block,
	const char* name
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_AttributeBlockFuncTable
{
	size_t m_size;
	jnc_AttributeBlock_GetAttributeCountFunc* m_getAttributeCountFunc;
	jnc_AttributeBlock_GetAttributeFunc* m_getAttributeFunc;
	jnc_AttributeBlock_FindAttributeFunc* m_findAttributeFunc;
};

//..............................................................................

// Namespace

typedef
bool_t
jnc_Namespace_IsReadyFunc(jnc_Namespace* nspace);

typedef
jnc_NamespaceKind
jnc_Namespace_GetNamespaceKindFunc(jnc_Namespace* nspace);

typedef
jnc_Namespace*
jnc_Namespace_GetParentNamespaceFunc(jnc_Namespace* nspace);

typedef
jnc_ModuleItem*
jnc_Namespace_GetParentItemFunc(jnc_Namespace* nspace);

typedef
size_t
jnc_Namespace_GetItemCountFunc(jnc_Namespace* nspace);

typedef
jnc_ModuleItem*
jnc_Namespace_GetItemFunc(
	jnc_Namespace* nspace,
	size_t index
	);

typedef
jnc_FindModuleItemResult
jnc_Namespace_FindItemFunc(
	jnc_Namespace* nspace,
	const char* name
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_NamespaceFuncTable
{
	size_t m_size;
	jnc_Namespace_IsReadyFunc* m_isReadyFunc;
	jnc_Namespace_GetNamespaceKindFunc* m_getNamespaceKindFunc;
	jnc_Namespace_GetParentNamespaceFunc* m_getParentNamespaceFunc;
	jnc_Namespace_GetParentItemFunc* m_getParentItemFunc;
	jnc_Namespace_GetItemCountFunc* m_getItemCountFunc;
	jnc_Namespace_GetItemFunc* m_getItemFunc;
	jnc_Namespace_FindItemFunc* m_findItemFunc;
};

//..............................................................................

// GlobalNamespace

struct jnc_GlobalNamespaceFuncTable
{
	size_t m_size;
};

//..............................................................................

// Variable

struct jnc_VariableFuncTable
{
	size_t m_size;
};

//..............................................................................

// Function

typedef
jnc_FunctionKind
jnc_Function_GetFunctionKindFunc(jnc_Function* function);

typedef
bool_t
jnc_Function_IsMemberFunc(jnc_Function* function);

typedef
void*
jnc_Function_GetMachineCodeFunc(jnc_Function* function);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionFuncTable
{
	size_t m_size;
	jnc_Function_GetFunctionKindFunc* m_getFunctionKindFunc;
	jnc_Function_IsMemberFunc* m_isMemberFunc;
	jnc_Function_GetMachineCodeFunc* m_getMachineCodeFunc;
};

//..............................................................................

// FunctionOverload

typedef
jnc_FunctionKind
jnc_FunctionOverload_GetFunctionKindFunc(jnc_FunctionOverload* function);

typedef
size_t
jnc_FunctionOverload_GetOverloadCountFunc(jnc_FunctionOverload* function);

typedef
jnc_Function*
jnc_FunctionOverload_GetOverloadFunc(
	jnc_FunctionOverload* function,
	size_t index
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionOverloadFuncTable
{
	size_t m_size;
	jnc_FunctionOverload_GetFunctionKindFunc* m_getFunctionKindFunc;
	jnc_FunctionOverload_GetOverloadCountFunc* m_getOverloadCountFunc;
	jnc_FunctionOverload_GetOverloadFunc* m_getOverloadFunc;
};

//..............................................................................

// Property

typedef
jnc_Function*
jnc_Property_GetGetterFunc(jnc_Property* prop);

typedef
jnc_OverloadableFunction
jnc_Property_GetSetterFunc(jnc_Property* prop);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyFuncTable
{
	size_t m_size;
	jnc_Property_GetGetterFunc* m_getGetterFunc;
	jnc_Property_GetSetterFunc* m_getSetterFunc;
};

//..............................................................................

// Typedef

struct jnc_TypedefFuncTable
{
	size_t m_size;
};

//..............................................................................

// Type

typedef
jnc_TypeKind
jnc_Type_GetTypeKindFunc(jnc_Type* type);

typedef
size_t
jnc_Type_GetSizeFunc(jnc_Type* type);

typedef
const char*
jnc_Type_GetTypeStringFunc(jnc_Type* type);

typedef
int
jnc_Type_CmpFunc(
	jnc_Type* type,
	jnc_Type* type2
	);

typedef
jnc_DataPtrType*
jnc_Type_GetDataPtrTypeFunc(
	jnc_Type* type,
	jnc_TypeKind typeKind,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
	);

typedef
bool_t
jnc_Type_ensureLayoutFunc(jnc_Type* type);

typedef
void
jnc_Type_MarkGcRootsFunc(
	jnc_Type* type,
	const void* p,
	jnc_GcHeap* gcHeap
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_TypeFuncTable
{
	size_t m_size;
	jnc_Type_GetTypeKindFunc* m_getTypeKindFunc;
	jnc_Type_GetSizeFunc* m_getSizeFunc;
	jnc_Type_GetTypeStringFunc* m_getTypeStringFunc;
	jnc_Type_CmpFunc* m_cmpFunc;
	jnc_Type_GetDataPtrTypeFunc* m_getDataPtrTypeFunc;
	jnc_Type_ensureLayoutFunc* m_ensureLayoutFunc;
	jnc_Type_MarkGcRootsFunc* m_markGcRootsFunc;
};

//..............................................................................

// NamedType

struct jnc_NamedTypeFuncTable
{
	size_t m_size;
};

//..............................................................................

// BaseTypeSlot

typedef
size_t
jnc_BaseTypeSlot_GetOffsetFunc(jnc_BaseTypeSlot* baseType);

typedef
size_t
jnc_BaseTypeSlot_GetVtableIndexFunc(jnc_BaseTypeSlot* baseType);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_BaseTypeSlotFuncTable
{
	size_t m_size;
	jnc_BaseTypeSlot_GetOffsetFunc* m_getOffsetFunc;
	jnc_BaseTypeSlot_GetVtableIndexFunc* m_getVtableIndexFunc;
};

//..............................................................................

// DerivableType

typedef
jnc_Function*
jnc_DerivableType_GetMethodFunc(jnc_DerivableType* type);

typedef
jnc_OverloadableFunction
jnc_DerivableType_GetOverloadableMethodFunc(jnc_DerivableType* type);

typedef
jnc_OverloadableFunction
jnc_DerivableType_GetUnaryOperatorFunc(
	jnc_DerivableType* type,
	jnc_UnOpKind opKind
	);

typedef
jnc_OverloadableFunction
jnc_DerivableType_GetBinaryOperatorFunc(
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	);

typedef
size_t
jnc_DerivableType_GetCastOperatorCountFunc(jnc_DerivableType* type);

typedef
jnc_Function*
jnc_DerivableType_GetCastOperatorFunc(
	jnc_DerivableType* type,
	size_t index
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DerivableTypeFuncTable
{
	size_t m_size;
	jnc_DerivableType_GetMethodFunc* m_getStaticConstructorFunc;
	jnc_DerivableType_GetOverloadableMethodFunc* m_getConstructorFunc;
	jnc_DerivableType_GetMethodFunc* m_getDestructorFunc;
	jnc_DerivableType_GetUnaryOperatorFunc* m_getUnaryOperatorFunc;
	jnc_DerivableType_GetBinaryOperatorFunc* m_getBinaryOperatorFunc;
	jnc_DerivableType_GetOverloadableMethodFunc* m_getCallOperatorFunc;
	jnc_DerivableType_GetCastOperatorCountFunc* m_getCastOperatorCountFunc;
	jnc_DerivableType_GetCastOperatorFunc* m_getCastOperatorFunc;
};

//..............................................................................

// ArrayType

typedef
jnc_Type*
jnc_ArrayType_GetElementTypeFunc(jnc_ArrayType* type);

typedef
size_t
jnc_ArrayType_GetElementCountFunc(jnc_ArrayType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ArrayTypeFuncTable
{
	size_t m_size;
	jnc_ArrayType_GetElementTypeFunc* m_getElementTypeFunc;
	jnc_ArrayType_GetElementCountFunc* m_GetElementCountFunc;
};

//..............................................................................

// BitFieldType

typedef
jnc_Type*
jnc_BitFieldType_GetBaseTypeFunc(jnc_BitFieldType* type);

typedef
size_t
jnc_BitFieldType_GetBitOffsetFunc(jnc_BitFieldType* type);

typedef
size_t
jnc_BitFieldType_GetBitCountFunc(jnc_BitFieldType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_BitFieldTypeFuncTable
{
	size_t m_size;
	jnc_BitFieldType_GetBaseTypeFunc* m_getBaseTypeFunc;
	jnc_BitFieldType_GetBitOffsetFunc* m_getBitOffsetFunc;
	jnc_BitFieldType_GetBitCountFunc* m_getBitCountFunc;
};

//..............................................................................

// FunctionArg

struct jnc_FunctionArgFuncTable
{
	size_t m_size;
};

//..............................................................................

// FunctionType

typedef
jnc_Type*
jnc_FunctionType_GetReturnTypeFunc(jnc_FunctionType* type);

typedef
size_t
jnc_FunctionType_GetArgCountFunc(jnc_FunctionType* type);

typedef
jnc_FunctionArg*
jnc_FunctionType_GetArgFunc(
	jnc_FunctionType* type,
	size_t index
	);

typedef
jnc_FunctionPtrType*
jnc_FunctionType_GetFunctionPtrTypeFunc(
	jnc_FunctionType* type,
	jnc_TypeKind typeKind,
	jnc_FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	);

typedef
jnc_FunctionType*
jnc_FunctionType_GetShortTypeFunc(jnc_FunctionType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


struct jnc_FunctionTypeFuncTable
{
	size_t m_size;
	jnc_FunctionType_GetReturnTypeFunc* m_getReturnTypeFunc;
	jnc_FunctionType_GetArgCountFunc* m_getArgCountFunc;
	jnc_FunctionType_GetArgFunc* m_getArgFunc;
	jnc_FunctionType_GetFunctionPtrTypeFunc* m_getFunctionPtrTypeFunc;
	jnc_FunctionType_GetShortTypeFunc* m_getShortTypeFunc;
};

//..............................................................................

// PropertyType

struct jnc_PropertyTypeFuncTable
{
	size_t m_size;
};

//..............................................................................

// EnumConst

typedef
int64_t
jnc_EnumConst_GetValueFunc(jnc_EnumConst* enumConst);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_EnumConstFuncTable
{
	size_t m_size;
	jnc_EnumConst_GetValueFunc* m_getValueFunc;
};

//..............................................................................

// EnumType

typedef
jnc_Type*
jnc_EnumType_GetBaseTypeFunc(jnc_EnumType* type);

typedef
size_t
jnc_EnumType_GetConstCountFunc(jnc_EnumType* type);

typedef
jnc_EnumConst*
jnc_EnumType_GetConstFunc(
	jnc_EnumType* type,
	size_t index
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_EnumTypeFuncTable
{
	size_t m_size;
	jnc_EnumType_GetBaseTypeFunc* m_getBaseTypeFunc;
	jnc_EnumType_GetConstCountFunc* m_getConstCountFunc;
	jnc_EnumType_GetConstFunc* m_getConstFunc;
};

//..............................................................................

// FieldFuncTable

typedef
size_t
jnc_Field_GetOffsetFunc(jnc_Field* field);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FieldFuncTable
{
	size_t m_size;
	jnc_Field_GetOffsetFunc* m_getOffsetFunc;
};

//..............................................................................

// StructType

struct jnc_StructTypeFuncTable
{
	size_t m_size;
};

//..............................................................................

// UnionType

struct jnc_UnionTypeFuncTable
{
	size_t m_size;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ClassType

typedef
jnc_ClassTypeKind
jnc_ClassType_GetClassTypeKindFunc(jnc_ClassType* type);

typedef
jnc_StructType*
jnc_ClassType_GetIfaceStructTypeFunc(jnc_ClassType* type);

typedef
jnc_ClassPtrType*
jnc_ClassType_GetClassPtrTypeFunc(
	jnc_ClassType* type,
	jnc_TypeKind typeKind,
	jnc_ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ClassTypeFuncTable
{
	size_t m_size;
	jnc_ClassType_GetClassTypeKindFunc* m_getClassTypeKindFunc;
	jnc_ClassType_GetIfaceStructTypeFunc* m_getIfaceStructTypeFunc;
	jnc_ClassType_GetClassPtrTypeFunc* m_getClassPtrTypeFunc;
};

//..............................................................................

// MulticastClassType

typedef
jnc_FunctionPtrType*
jnc_MulticastClassType_GetTargetTypeFunc(jnc_MulticastClassType* type);

typedef
jnc_Function*
jnc_MulticastClassType_GetMethodFunc(
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_MulticastClassTypeFuncTable
{
	size_t m_size;
	jnc_MulticastClassType_GetTargetTypeFunc* m_getTargetTypeFunc;
	jnc_MulticastClassType_GetMethodFunc* m_getMethodFunc;
};

//..............................................................................

// McSnapshotClassType

typedef
jnc_FunctionPtrType*
jnc_McSnapshotClassType_GetTargetTypeFunc(jnc_McSnapshotClassType* type);

typedef
jnc_Function*
jnc_McSnapshotClassType_GetMethodFunc(
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_McSnapshotClassTypeFuncTable
{
	size_t m_size;
	jnc_McSnapshotClassType_GetTargetTypeFunc* m_getTargetTypeFunc;
	jnc_McSnapshotClassType_GetMethodFunc* m_getMethodFunc;
};

//..............................................................................

// DataPtrType

typedef
jnc_DataPtrTypeKind
jnc_DataPtrType_GetPtrTypeKindFunc(jnc_DataPtrType* type);

typedef
jnc_Type*
jnc_DataPtrType_GetTargetTypeFunc(jnc_DataPtrType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DataPtrTypeFuncTable
{
	size_t m_size;
	jnc_DataPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_DataPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//..............................................................................

// ClassPtrType

struct jnc_ClassPtrTypeFuncTable
{
	size_t m_size;
};

//..............................................................................

// FunctionPtrType

typedef
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_GetPtrTypeKindFunc(jnc_FunctionPtrType* type);

typedef
jnc_FunctionType*
jnc_FunctionPtrType_GetTargetTypeFunc(jnc_FunctionPtrType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionPtrTypeFuncTable
{
	size_t m_size;
	jnc_FunctionPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_FunctionPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//..............................................................................

// PropertyPtrType

struct jnc_PropertyPtrTypeFuncTable
{
	size_t m_size;
};

//..............................................................................

// Variant

typedef
bool_t
jnc_Variant_CastFunc(
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	);

typedef
bool_t
jnc_Variant_UnaryOperatorFunc(
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	);

typedef
bool_t
jnc_Variant_BinaryOperatorFunc(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	);

typedef
bool_t
jnc_Variant_RelationalOperatorFunc(
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	bool_t* result
	);

typedef
bool_t
jnc_Variant_GetMemberFunc(
	const jnc_Variant* variant,
	const char* name,
	jnc_Variant* result
	);

typedef
bool_t
jnc_Variant_SetMemberFunc(
	jnc_Variant* variant,
	const char* name,
	jnc_Variant value
	);

typedef
bool_t
jnc_Variant_GetElementFunc(
	const jnc_Variant* variant,
	size_t index,
	jnc_Variant* result
	);

typedef
bool_t
jnc_Variant_SetElementFunc(
	jnc_Variant* variant,
	size_t index,
	jnc_Variant value
	);

typedef
size_t
jnc_Variant_HashFunc(const jnc_Variant* variant);

typedef
const char*
jnc_Variant_FormatFunc(
	const jnc_Variant* variant,
	const char* fmtSpecifier
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_VariantFuncTable
{
	size_t m_size;
	jnc_Variant_CastFunc* m_castFunc;
	jnc_Variant_UnaryOperatorFunc* m_unaryOperatorFunc;
	jnc_Variant_BinaryOperatorFunc* m_binaryOperatorFunc;
	jnc_Variant_RelationalOperatorFunc* m_relationalOperatorFunc;
	jnc_Variant_GetMemberFunc* m_getMemberFunc;
	jnc_Variant_SetMemberFunc* m_setMemberFunc;
	jnc_Variant_GetElementFunc* m_getElementFunc;
	jnc_Variant_SetElementFunc* m_setElementFunc;
	jnc_Variant_HashFunc* m_hashFunc;
	jnc_Variant_FormatFunc* m_formatFunc;
};

//..............................................................................

typedef
jnc_Promise*
jnc_Promise_CreateFunc(jnc_Runtime* runtime);

typedef
void
jnc_Promise_CompleteFunc(
	jnc_Promise* promise,
	jnc_Variant result,
	jnc_DataPtr errorPtr
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PromiseFuncTable
{
	size_t m_size;
	jnc_Promise_CreateFunc* m_createFunc;
	jnc_Promise_CompleteFunc* m_completeFunc;
};

//..............................................................................

// Unit

struct jnc_UnitFuncTable
{
	size_t m_size;
};

//..............................................................................

// Module

typedef
jnc_Module*
jnc_Module_CreateFunc();

typedef
void
jnc_Module_DestroyFunc(jnc_Module* module);

typedef
void
jnc_Module_ClearFunc(jnc_Module* module);

typedef
void
jnc_Module_InitializeFunc(
	jnc_Module* module,
	const char* tag,
	uint_t compileFlags
	);

typedef
jnc_GlobalNamespace*
jnc_Module_GetGlobalNamespaceFunc(jnc_Module* module);

typedef
jnc_Type*
jnc_Module_GetPrimitiveTypeFunc(
	jnc_Module* module,
	jnc_TypeKind typeKind
	);

typedef
jnc_Type*
jnc_Module_GetStdTypeFunc(
	jnc_Module* module,
	jnc_StdType stdType
	);

typedef
handle_t
jnc_Module_GetExtensionSourceFileIteratorFunc(jnc_Module* module);

typedef
const char*
jnc_Module_GetNextExtensionSourceFileFunc(
	jnc_Module* module,
	handle_t* iterator
	);

typedef
jnc_FindModuleItemResult
jnc_Module_FindExtensionLibItemFunc(
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

typedef
bool_t
jnc_Module_MapVariableFunc(
	jnc_Module* module,
	jnc_Variable* variable,
	void* p
	);

typedef
bool_t
jnc_Module_MapFunctionFunc(
	jnc_Module* module,
	jnc_Function* function,
	void* p
	);

typedef
void
jnc_Module_AddSourceFunc(
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	);

typedef
handle_t
jnc_Module_GetImportDirIteratorFunc(jnc_Module* module);

typedef
const char*
jnc_Module_GetNextImportDirFunc(
	jnc_Module* module,
	handle_t* iterator
	);

typedef
void
jnc_Module_AddImportDirFunc(
	jnc_Module* module,
	const char* dir
	);

typedef
bool_t
jnc_Module_AddImportFunc(
	jnc_Module* module,
	const char* fileName
	);

typedef
void
jnc_Module_AddOpaqueClassTypeInfoFunc(
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

typedef
void
jnc_Module_AddStaticLibFunc(
	jnc_Module* module,
	jnc_ExtensionLib* lib
	);

typedef
void
jnc_Module_RequireFunc(
	jnc_Module* module,
	jnc_ModuleItemKind itemKind,
	const char* name,
	bool_t isEssential
	);

typedef
void
jnc_Module_RequireTypeFunc(
	jnc_Module* module,
	jnc_TypeKind typeKind,
	const char* name,
	bool_t isEssential
	);

typedef
bool_t
jnc_Module_ParseFunc(
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
	);

typedef
bool_t
jnc_Module_ParseFileFunc(
	jnc_Module* module,
	const char* fileName
	);

typedef
bool_t
jnc_Module_ParseImportsFunc(jnc_Module* module);

typedef
bool_t
jnc_Module_CompileFunc(jnc_Module* module);

typedef
bool_t
jnc_Module_JitFunc(jnc_Module* module);

typedef
const char*
jnc_Module_getLlvmIrStringFunc(jnc_Module* module);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleFuncTable
{
	size_t m_size;
	jnc_Module_CreateFunc* m_createFunc;
	jnc_Module_DestroyFunc* m_destroyFunc;
	jnc_Module_ClearFunc* m_clearFunc;
	jnc_Module_InitializeFunc* m_initializeFunc;
	jnc_Module_GetGlobalNamespaceFunc* m_getGlobalNamespaceFunc;
	jnc_Module_GetPrimitiveTypeFunc* m_getPrimitiveTypeFunc;
	jnc_Module_GetStdTypeFunc* m_getStdTypeFunc;
	jnc_Module_GetExtensionSourceFileIteratorFunc* m_getExtensionSourceFileIteratorFunc;
	jnc_Module_GetNextExtensionSourceFileFunc* m_getNextExtensionSourceFileFunc;
	jnc_Module_FindExtensionLibItemFunc* m_findExtensionLibItemFunc;
	jnc_Module_MapVariableFunc* m_mapVariableFunc;
	jnc_Module_MapFunctionFunc* m_mapFunctionFunc;
	jnc_Module_AddSourceFunc* m_addSourceFunc;
	jnc_Module_GetImportDirIteratorFunc* m_getImportDirIteratorFunc;
	jnc_Module_GetNextImportDirFunc* m_getNextImportDirFunc;
	jnc_Module_AddImportDirFunc* m_addImportDirFunc;
	jnc_Module_AddImportFunc* m_addImportFunc;
	jnc_Module_AddOpaqueClassTypeInfoFunc* m_addOpaqueClassTypeInfoFunc;
	jnc_Module_AddStaticLibFunc* m_addStaticLibFunc;
	jnc_Module_RequireFunc* m_requireFunc;
	jnc_Module_RequireTypeFunc* m_requireTypeFunc;
	jnc_Module_ParseFunc* m_parseFunc;
	jnc_Module_ParseFileFunc* m_parseFileFunc;
	jnc_Module_ParseImportsFunc* m_parseImportsFunc;
	jnc_Module_CompileFunc* m_compileFunc;
	jnc_Module_JitFunc* m_jitFunc;
	jnc_Module_getLlvmIrStringFunc* m_getLlvmIrStringFunc;
};

//..............................................................................

// Runtime

typedef
jnc_Runtime*
jnc_Runtime_CreateFunc();

typedef
void
jnc_Runtime_DestroyFunc(jnc_Runtime* runtime);

typedef
jnc_Module*
jnc_Runtime_GetModuleFunc(jnc_Runtime* runtime);

typedef
jnc_GcHeap*
jnc_Runtime_GetGcHeapFunc(jnc_Runtime* runtime);

typedef
bool_t
jnc_Runtime_IsAbortedFunc(jnc_Runtime* runtime);

typedef
bool_t
jnc_Runtime_StartupFunc(
	jnc_Runtime* runtime,
	jnc_Module* module
	);

typedef
void
jnc_Runtime_ShutdownFunc(jnc_Runtime* runtime);

typedef
void
jnc_Runtime_AbortFunc(jnc_Runtime* runtime);

typedef
void
jnc_Runtime_InitializeCallSiteFunc(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	);

typedef
void
jnc_Runtime_UninitializeCallSiteFunc(
	jnc_Runtime* runtime,
	jnc_CallSite* callSite
	);

typedef
jnc_SjljFrame*
jnc_Runtime_SetSjljFrameFunc(
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
	);

typedef
void*
jnc_Runtime_GetUserDataFunc(jnc_Runtime* runtime);

typedef
void*
jnc_Runtime_SetUserDataFunc(
	jnc_Runtime* runtime,
	void* data
	);

typedef
jnc_Runtime*
jnc_GetCurrentThreadRuntimeFunc();

typedef
jnc_Tls*
jnc_GetCurrentThreadTlsFunc();

typedef
void
jnc_DynamicThrowFunc();

typedef
void
jnc_PrimeClassFunc(
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_RuntimeFuncTable
{
	size_t m_size;
	jnc_Runtime_CreateFunc* m_createFunc;
	jnc_Runtime_DestroyFunc* m_destroyFunc;
	jnc_Runtime_GetModuleFunc* m_getModuleFunc;
	jnc_Runtime_GetGcHeapFunc* m_getGcHeapFunc;
	jnc_Runtime_IsAbortedFunc* m_isAbortedFunc;
	jnc_Runtime_StartupFunc* m_startupFunc;
	jnc_Runtime_ShutdownFunc* m_shutdownFunc;
	jnc_Runtime_AbortFunc* m_abortFunc;
	jnc_Runtime_InitializeCallSiteFunc* m_initializeCallSiteFunc;
	jnc_Runtime_UninitializeCallSiteFunc* m_uninitializeCallSiteFunc;
	jnc_Runtime_SetSjljFrameFunc* m_setSjljFrameFunc;
	jnc_Runtime_GetUserDataFunc* m_getUserDataFunc;
	jnc_Runtime_SetUserDataFunc* m_setUserDataFunc;
	jnc_GetCurrentThreadRuntimeFunc* m_getCurrentThreadRuntimeFunc;
	jnc_GetCurrentThreadTlsFunc* m_getCurrentThreadTlsFunc;
	jnc_DynamicThrowFunc* m_dynamicThrowFunc;
	jnc_PrimeClassFunc* m_primeClassFunc;
};

//..............................................................................

// GcHeap

typedef
jnc_Runtime*
jnc_GcHeap_GetRuntimeFunc(jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_GetStatsFunc(
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

typedef
void
jnc_GcHeap_GetSizeTriggersFunc(
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	);

typedef
void
jnc_GcHeap_SetSizeTriggersFunc(
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	);

typedef
void
jnc_GcHeap_CollectFunc(jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_EnterNoCollectRegionFunc(jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_LeaveNoCollectRegionFunc(
	jnc_GcHeap* gcHeap,
	bool_t canCollectNow
	);

typedef
void
jnc_GcHeap_EnterWaitRegionFunc(jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_LeaveWaitRegionFunc(jnc_GcHeap* gcHeap);

typedef
jnc_IfaceHdr*
jnc_GcHeap_AllocateClassFunc(
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateDataFunc(
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateArrayFunc(
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateBufferFunc(
	jnc_GcHeap* gcHeap,
	size_t size
	);

typedef
jnc_DataPtrValidator*
jnc_GcHeap_CreateDataPtrValidatorFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box,
	const void* rangeBegin,
	size_t rangeLength
	);

typedef
jnc_DetachedDataBox*
jnc_GcHeap_CreateForeignDataBoxFunc(
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t elementCount,
	const void* p,
	bool_t isCallSiteLocal
	);

typedef
jnc_DataPtr
jnc_GcHeap_CreateForeignBufferPtrFunc(
	jnc_GcHeap* gcHeap,
	const void* p,
	size_t size,
	bool_t isCallSiteLocal
	);

typedef
void
jnc_GcHeap_InvalidateDataPtrValidatorFunc(
	jnc_GcHeap* gcHeap,
	jnc_DataPtrValidator* validator
	);

typedef
jnc_IfaceHdr*
jnc_GcHeap_GetDynamicLayoutFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_ResetDynamicLayoutFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_WeakMarkFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_MarkDataFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_MarkClassFunc(
	jnc_GcHeap* gcHeap,
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_AddBoxToCallSiteFunc(jnc_Box* box);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcHeapFuncTable
{
	size_t m_size;
	jnc_GcHeap_GetRuntimeFunc* m_getRuntimeFunc;
	jnc_GcHeap_GetStatsFunc* m_getStatsFunc;
	jnc_GcHeap_GetSizeTriggersFunc* m_getSizeTriggersFunc;
	jnc_GcHeap_SetSizeTriggersFunc* m_setSizeTriggersFunc;
	jnc_GcHeap_CollectFunc* m_collectFunc;
	jnc_GcHeap_EnterNoCollectRegionFunc* m_enterNoCollectRegionFunc;
	jnc_GcHeap_LeaveNoCollectRegionFunc* m_leaveNoCollectRegionFunc;
	jnc_GcHeap_EnterWaitRegionFunc* m_enterWaitRegionFunc;
	jnc_GcHeap_LeaveWaitRegionFunc* m_leaveWaitRegionFunc;
	jnc_GcHeap_AllocateClassFunc* m_allocateClassFunc;
	jnc_GcHeap_AllocateClassFunc* m_tryAllocateClassFunc;
	jnc_GcHeap_AllocateDataFunc* m_allocateDataFunc;
	jnc_GcHeap_AllocateDataFunc* m_tryAllocateDataFunc;
	jnc_GcHeap_AllocateArrayFunc* m_allocateArrayFunc;
	jnc_GcHeap_AllocateArrayFunc* m_tryAllocateArrayFunc;
	jnc_GcHeap_AllocateBufferFunc* m_allocateBufferFunc;
	jnc_GcHeap_AllocateBufferFunc* m_tryAllocateBufferFunc;
	jnc_GcHeap_CreateDataPtrValidatorFunc* m_createDataPtrValidatorFunc;
	jnc_GcHeap_CreateForeignDataBoxFunc* m_createForeignDataBoxFunc;
	jnc_GcHeap_CreateForeignBufferPtrFunc* m_createForeignBufferPtrFunc;
	jnc_GcHeap_InvalidateDataPtrValidatorFunc* m_invalidateDataPtrValidatorFunc;
	jnc_GcHeap_GetDynamicLayoutFunc* m_getDynamicLayoutFunc;
	jnc_GcHeap_ResetDynamicLayoutFunc* m_resetDynamicLayoutFunc;
	jnc_GcHeap_WeakMarkFunc* m_weakMarkFunc;
	jnc_GcHeap_MarkDataFunc* m_markDataFunc;
	jnc_GcHeap_MarkClassFunc* m_markClassFunc;
	jnc_GcHeap_AddBoxToCallSiteFunc* m_addBoxToCallSiteFunc;
};

//..............................................................................

// std.HashTable

typedef
jnc_StdHashTable*
jnc_CreateStdHashTableFunc(
	jnc_Runtime* runtime,
	jnc_StdHashFunc* hashFunc,
	jnc_StdIsEqualFunc* isEqualFunc
	);

typedef
void
jnc_StdHashTable_ClearFunc(jnc_StdHashTable* hashTable);

typedef
jnc_StdMapEntry*
jnc_StdHashTable_FindFunc(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	);

typedef
jnc_StdMapEntry*
jnc_StdHashTable_AddFunc(
	jnc_StdHashTable* hashTable,
	jnc_Variant key,
	jnc_Variant value
	);

typedef
void
jnc_StdHashTable_RemoveFunc(
	jnc_StdHashTable* hashTable,
	jnc_StdMapEntry* entry
	);

typedef
bool_t
jnc_StdHashTable_RemoveKeyFunc(
	jnc_StdHashTable* hashTable,
	jnc_Variant key
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdHashTableFuncTable
{
	size_t m_size;
	jnc_CreateStdHashTableFunc* m_createStdHashTableFunc;
	jnc_StdHashTable_ClearFunc* m_clearFunc;
	jnc_StdHashTable_FindFunc* m_findFunc;
	jnc_StdHashTable_AddFunc* m_addFunc;
	jnc_StdHashTable_RemoveFunc* m_removeFunc;
	jnc_StdHashTable_RemoveKeyFunc* m_removeKeyFunc;
};

//..............................................................................

// std.RbTree

typedef
jnc_StdRbTree*
jnc_CreateStdRbTreeFunc(
	jnc_Runtime* runtime,
	jnc_StdCmpFunc* cmpFunc
	);

typedef
void
jnc_StdRbTree_ClearFunc(jnc_StdRbTree* RbTree);

typedef
jnc_StdMapEntry*
jnc_StdRbTree_FindFunc(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
	);

typedef
jnc_StdMapEntry*
jnc_StdRbTree_AddFunc(
	jnc_StdRbTree* RbTree,
	jnc_Variant key,
	jnc_Variant value
	);

typedef
void
jnc_StdRbTree_RemoveFunc(
	jnc_StdRbTree* RbTree,
	jnc_StdMapEntry* entry
	);

typedef
bool_t
jnc_StdRbTree_RemoveKeyFunc(
	jnc_StdRbTree* RbTree,
	jnc_Variant key
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdRbTreeFuncTable
{
	size_t m_size;
	jnc_CreateStdRbTreeFunc* m_createStdRbTreeFunc;
	jnc_StdRbTree_ClearFunc* m_clearFunc;
	jnc_StdRbTree_FindFunc* m_findFunc;
	jnc_StdRbTree_AddFunc* m_addFunc;
	jnc_StdRbTree_RemoveFunc* m_removeFunc;
	jnc_StdRbTree_RemoveKeyFunc* m_removeKeyFunc;
};

//..............................................................................

// all-in-one

struct jnc_DynamicExtensionLibHost
{
	size_t m_size;
	jnc_ErrorRouter* m_errorRouter;
	jnc_ErrorFuncTable* m_errorFuncTable;
	jnc_ModuleItemDeclFuncTable* m_moduleItemDeclFuncTable;
	jnc_ModuleItemFuncTable* m_moduleItemFuncTable;
	jnc_AttributeFuncTable* m_attributeFuncTable;
	jnc_AttributeBlockFuncTable* m_attributeBlockFuncTable;
	jnc_NamespaceFuncTable* m_namespaceFuncTable;
	jnc_GlobalNamespaceFuncTable* m_globalNamespaceFuncTable;
	jnc_VariableFuncTable* m_variableFuncTable;
	jnc_FunctionFuncTable* m_functionFuncTable;
	jnc_FunctionOverloadFuncTable* m_functionOverloadFuncTable;
	jnc_PropertyFuncTable* m_propertyFuncTable;
	jnc_TypedefFuncTable* m_typedefFuncTable;
	jnc_TypeFuncTable* m_typeFuncTable;
	jnc_NamedTypeFuncTable* m_namedTypeFuncTable;
	jnc_BaseTypeSlotFuncTable* m_baseTypeSlotFuncTable;
	jnc_FieldFuncTable* m_fieldFuncTable;
	jnc_DerivableTypeFuncTable* m_derivableTypeFuncTable;
	jnc_ArrayTypeFuncTable* m_arrayTypeFuncTable;
	jnc_BitFieldTypeFuncTable* m_bitFieldTypeFuncTable;
	jnc_FunctionArgFuncTable* m_functionArgFuncTable;
	jnc_FunctionTypeFuncTable* m_functionTypeFuncTable;
	jnc_PropertyTypeFuncTable* m_propertyTypeFuncTable;
	jnc_EnumConstFuncTable* m_enumConstFuncTable;
	jnc_EnumTypeFuncTable* m_enumTypeFuncTable;
	jnc_StructTypeFuncTable* m_structTypeFuncTable;
	jnc_UnionTypeFuncTable* m_unionTypeFuncTable;
	jnc_ClassTypeFuncTable* m_classTypeFuncTable;
	jnc_MulticastClassTypeFuncTable* m_multicastClassTypeFuncTable;
	jnc_McSnapshotClassTypeFuncTable* m_mcSnapshotClassTypeFuncTable;
	jnc_DataPtrTypeFuncTable* m_dataPtrTypeFuncTable;
	jnc_ClassPtrTypeFuncTable* m_classPtrTypeFuncTable;
	jnc_FunctionPtrTypeFuncTable* m_functionPtrTypeFuncTable;
	jnc_PropertyPtrTypeFuncTable* m_propertyPtrTypeFuncTable;
	jnc_VariantFuncTable* m_variantFuncTable;
	jnc_PromiseFuncTable* m_promiseFuncTable;
	jnc_UnitFuncTable* m_unitFuncTable;
	jnc_ModuleFuncTable* m_moduleFuncTable;
	jnc_RuntimeFuncTable* m_runtimeFuncTable;
	jnc_GcHeapFuncTable* m_gcHeapFuncTable;
	jnc_StdHashTableFuncTable* m_stdHashTableFuncTable;
	jnc_StdRbTreeFuncTable* m_stdRbTreeFuncTable;
};

//..............................................................................
