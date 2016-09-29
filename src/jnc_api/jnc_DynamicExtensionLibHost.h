#pragma once

#define _JNC_DYNAMICEXTENSIONLIBHOST_H

#include "jnc_RuntimeStructs.h"
#include "jnc_Function.h"
#include "jnc_ClassType.h"
#include "jnc_Variant.h"
#include "jnc_OpKind.h"

//.............................................................................

// Error

typedef
jnc_Error*
jnc_GetLastErrorFunc ();

typedef
void
jnc_SetErrorFunc (jnc_Error* error);

typedef
const char*
jnc_getErrorDescriptionFunc (jnc_Error* error);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ModuleItemDecl

typedef
const char*
jnc_ModuleItemDecl_GetNameFunc (jnc_ModuleItemDecl* decl);

typedef
const char*
jnc_ModuleItemDecl_GetQualifiedNameFunc (jnc_ModuleItemDecl* decl);

typedef
jnc_StorageKind
jnc_ModuleItemDecl_GetStorageKindFunc (jnc_ModuleItemDecl* decl);

typedef
jnc_AccessKind
jnc_ModuleItemDecl_GetAccessKindFunc (jnc_ModuleItemDecl* decl);

typedef
jnc_AttributeBlock*
jnc_ModuleItemDecl_GetAttributeBlockFunc (jnc_ModuleItemDecl* decl);

typedef
jnc_Namespace*
jnc_ModuleItemDecl_GetParentNamespaceFunc (jnc_ModuleItemDecl* decl);

typedef
jnc_Unit*
jnc_ModuleItemDecl_GetParentUnitFunc (jnc_ModuleItemDecl* decl);

typedef
int
jnc_ModuleItemDecl_GetLineFunc (jnc_ModuleItemDecl* decl);

typedef
int
jnc_ModuleItemDecl_GetColFunc (jnc_ModuleItemDecl* decl);

typedef
size_t
jnc_ModuleItemDecl_GetOffsetFunc (jnc_ModuleItemDecl* decl);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ModuleItem

typedef
jnc_Module*
jnc_ModuleItem_GetModuleFunc (jnc_ModuleItem* item);

typedef
jnc_ModuleItemKind
jnc_ModuleItem_GetItemKindFunc (jnc_ModuleItem* item);

typedef
uint_t
jnc_ModuleItem_GetFlagsFunc (jnc_ModuleItem* item);

typedef
jnc_ModuleItemDecl*
jnc_ModuleItem_GetDeclFunc (jnc_ModuleItem* item);

typedef
jnc_Namespace*
jnc_ModuleItem_GetNamespaceFunc (jnc_ModuleItem* item);

typedef
jnc_Type*
jnc_ModuleItem_GetTypeFunc (jnc_ModuleItem* item);

typedef
jnc_DerivableType*
jnc_VerifyModuleItemIsDerivableTypeFunc (
	jnc_ModuleItem* item,
	const char* name
	);

typedef
jnc_ClassType*
jnc_VerifyModuleItemIsClassTypeFunc (
	jnc_ModuleItem* item,
	const char* name
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Attribute

// AttributeBlock

typedef
size_t
jnc_AttributeBlock_GetAttributeCountFunc (jnc_AttributeBlock* block);

typedef
jnc_Attribute*
jnc_AttributeBlock_GetAttributeFunc (
	jnc_AttributeBlock* block,
	size_t index
	);

typedef
jnc_Attribute*
jnc_AttributeBlock_FindAttributeFunc (
	jnc_AttributeBlock* block,
	const char* name
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Namespace

typedef
size_t
jnc_Namespace_GetItemCountFunc (jnc_Namespace* nspace);

typedef
jnc_ModuleItem*
jnc_Namespace_GetItemFunc (
	jnc_Namespace* nspace,
	size_t index
	);

typedef
jnc_Function*
jnc_Namespace_FindFunctionFunc (
	jnc_Namespace* nspace,
	const char* name
	);

typedef
jnc_Property*
jnc_Namespace_FindPropertyFunc (
	jnc_Namespace* nspace,
	const char* name
	);

typedef
jnc_ClassType*
jnc_Namespace_FindClassTypeFunc (
	jnc_Namespace* nspace,
	const char* name
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// GlobalNamespace

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Variable

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Function

typedef
jnc_FunctionKind
jnc_Function_GetFunctionKindFunc (jnc_Function* function);

typedef
int
jnc_Function_IsMemberFunc (jnc_Function* function);

typedef
int
jnc_Function_IsOverloadedFunc (jnc_Function* function);

typedef
size_t
jnc_Function_GetOverloadCountFunc (jnc_Function* function);

typedef
jnc_Function*
jnc_Function_GetOverloadFunc (
	jnc_Function* function,
	size_t overloadIdx
	);

typedef
void*
jnc_Function_GetMachineCodeFunc (jnc_Function* function);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Property

typedef
jnc_Function*
jnc_Property_GetGetterFunc (jnc_Property* prop);

typedef
jnc_Function*
jnc_Property_GetSetterFunc (jnc_Property* prop);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Typedef

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Type

typedef
jnc_TypeKind
jnc_Type_GetTypeKindFunc (jnc_Type* type);

typedef
size_t
jnc_Type_GetSizeFunc (jnc_Type* type);

typedef
const char*
jnc_Type_GetTypeStringFunc (jnc_Type* type);

typedef
int
jnc_Type_CmpFunc (
	jnc_Type* type,
	jnc_Type* type2
	);

typedef
jnc_DataPtrType*
jnc_Type_GetDataPtrTypeFunc (
	jnc_Type* type,
	jnc_DataPtrTypeKind ptrTypeKind,
	uint_t flags
	);

typedef
void
jnc_Type_MarkGcRootsFunc (
	jnc_Type* type,
	const void* p,
	jnc_GcHeap* gcHeap
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// NamedType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// BaseTypeSlot

typedef
size_t
jnc_BaseTypeSlot_GetOffsetFunc (jnc_BaseTypeSlot* baseType);

typedef
size_t
jnc_BaseTypeSlot_GetVTableIndexFunc (jnc_BaseTypeSlot* baseType);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// DerivableType

typedef
jnc_Function*
jnc_DerivableType_GetMemberMethodFunc (jnc_DerivableType* type);

typedef
jnc_Function*
jnc_DerivableType_GetUnaryOperatorFunc (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	);

typedef
jnc_Function*
jnc_DerivableType_GetBinaryOperatorFunc (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	);

typedef
jnc_Function*
jnc_DerivableType_GetCastOperatorFunc (
	jnc_DerivableType* type,
	size_t idx
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ArrayType

typedef
jnc_Type*
jnc_ArrayType_GetElementTypeFunc (jnc_ArrayType* type);

typedef
size_t
jnc_ArrayType_GetElementCountFunc (jnc_ArrayType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// BitFieldType

typedef
jnc_Type*
jnc_BitFieldType_GetBaseTypeFunc (jnc_BitFieldType* type);

typedef
size_t
jnc_BitFieldType_GetBitOffsetFunc (jnc_BitFieldType* type);

typedef
size_t
jnc_BitFieldType_GetBitCountFunc (jnc_BitFieldType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// FunctionArg

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// FunctionType

typedef
jnc_Type*
jnc_FunctionType_GetReturnTypeFunc (jnc_FunctionType* type);

typedef
size_t
jnc_FunctionType_GetArgCountFunc (jnc_FunctionType* type);

typedef
jnc_FunctionArg*
jnc_FunctionType_GetArgFunc (
	jnc_FunctionType* type,
	size_t index	
	);

typedef
jnc_FunctionPtrType*
jnc_FunctionType_GetFunctionPtrTypeFunc (
	jnc_FunctionType* type,
	jnc_FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	);

typedef
jnc_FunctionType*
jnc_FunctionType_GetShortTypeFunc (jnc_FunctionType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// PropertyType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// EnumConst

typedef
int64_t
jnc_EnumConst_GetValueFunc (jnc_EnumConst* enumConst);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// EnumType

typedef
jnc_Type*
jnc_EnumType_GetBaseTypeFunc (jnc_EnumType* type);

typedef
size_t
jnc_EnumType_GetConstCountFunc (jnc_EnumType* type);

typedef
jnc_EnumConst*
jnc_EnumType_GetConstFunc (
	jnc_EnumType* type,
	size_t index
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// StructFieldFuncTable

typedef
size_t
jnc_StructField_GetOffsetFunc (jnc_StructField* field);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// StructType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// UnionType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ClassType

typedef
jnc_ClassTypeKind
jnc_ClassType_GetClassTypeKindFunc (jnc_ClassType* type);

typedef
jnc_StructType*
jnc_ClassType_GetIfaceStructTypeFunc (jnc_ClassType* type);

typedef
jnc_ClassPtrType*
jnc_ClassType_GetClassPtrTypeFunc (
	jnc_ClassType* type,
	jnc_ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// MulticastClassType

typedef
jnc_FunctionPtrType*
jnc_MulticastClassType_GetTargetTypeFunc (jnc_MulticastClassType* type);

typedef
jnc_Function*
jnc_MulticastClassType_GetMethodFunc (
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// McSnapshotClassType

typedef
jnc_FunctionPtrType*
jnc_McSnapshotClassType_GetTargetTypeFunc (jnc_McSnapshotClassType* type);

typedef
jnc_Function*
jnc_McSnapshotClassType_GetMethodFunc (
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// DataPtrType

typedef
jnc_DataPtrTypeKind
jnc_DataPtrType_GetPtrTypeKindFunc (jnc_DataPtrType* type);

typedef
jnc_Type*
jnc_DataPtrType_GetTargetTypeFunc (jnc_DataPtrType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// ClassPtrType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// FunctionPtrType

typedef
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_GetPtrTypeKindFunc (jnc_FunctionPtrType* type);

typedef
jnc_FunctionType*
jnc_FunctionPtrType_GetTargetTypeFunc (jnc_FunctionPtrType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// PropertyPtrType

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Variant

typedef
int
jnc_Variant_CastFunc (
	const jnc_Variant* variant,
	jnc_Type* type,
	void* buffer
	);

typedef
int
jnc_Variant_UnaryOperatorFunc (
	const jnc_Variant* variant,
	jnc_UnOpKind opKind,
	jnc_Variant* result
	);

typedef
int
jnc_Variant_BinaryOperatorFunc (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	jnc_Variant* result
	);

typedef
int
jnc_Variant_RelationalOperatorFunc (
	const jnc_Variant* variant,
	const jnc_Variant* variant2,
	jnc_BinOpKind opKind,
	int* result
	);

typedef
size_t 
jnc_Variant_GetHashFunc (const jnc_Variant* variant);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// Module

typedef
jnc_Module* 
jnc_Module_CreateFunc ();

typedef
void
jnc_Module_DestroyFunc (jnc_Module* module);

typedef
void
jnc_Module_ClearFunc (jnc_Module* module);

typedef
void
jnc_Module_InitializeFunc (
	jnc_Module* module,
	const char* tag,
	uint_t compileFlags
	);

typedef
jnc_GlobalNamespace*
jnc_Module_GetGlobalNamespaceFunc (jnc_Module* module);

typedef
jnc_Type*
jnc_Module_GetPrimitiveTypeFunc (
	jnc_Module* module,
	jnc_TypeKind typeKind
	);

typedef
jnc_ModuleItem*
jnc_Module_FindItemFunc (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

typedef
void
jnc_Module_MapFunctionFunc (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	);

typedef
void
jnc_Module_AddSourceFunc (
	jnc_Module* module,
	int isForced,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	);

typedef
void
jnc_Module_AddImportDirFunc (
	jnc_Module* module,
	const char* dir
	);

typedef
void
jnc_Module_AddImportFunc (
	jnc_Module* module,
	const char* fileName
	);

typedef
void
jnc_Module_AddOpaqueClassTypeInfoFunc (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

typedef
void
jnc_Module_AddStaticLibFunc (
	jnc_Module* module,
	jnc_ExtensionLib* lib
	);

typedef
int
jnc_Module_ParseFunc (
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	);

typedef
int
jnc_Module_ParseFileFunc (
	jnc_Module* module,
	const char* fileName
	);

typedef
int
jnc_Module_ParseImportsFunc (jnc_Module* module);

typedef
int
jnc_Module_CalcLayoutFunc (jnc_Module* module);

typedef
int
jnc_Module_CompileFunc (jnc_Module* module);

typedef
int
jnc_Module_JitFunc (jnc_Module* module);

typedef
const char*
jnc_Module_getLlvmIrStringFunc (jnc_Module* module);

// Runtime

typedef
jnc_Runtime*
jnc_Runtime_CreateFunc ();

typedef
void
jnc_Runtime_DestroyFunc (jnc_Runtime* runtime);

typedef
jnc_Module*
jnc_Runtime_GetModuleFunc (jnc_Runtime* runtime);

typedef
jnc_GcHeap*
jnc_Runtime_GetGcHeapFunc (jnc_Runtime* runtime);

typedef
size_t 
jnc_Runtime_GetStackSizeLimitFunc (jnc_Runtime* runtime);

typedef
int
jnc_Runtime_SetStackSizeLimitFunc (
	jnc_Runtime* runtime,
	size_t sizeLimit
	);

typedef
int
jnc_Runtime_StartupFunc (
	jnc_Runtime* runtime,
	jnc_Module* module
	);
	
typedef
void
jnc_Runtime_ShutdownFunc (jnc_Runtime* runtime);

typedef
void
jnc_Runtime_InitializeThreadFunc (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	);

typedef
void
jnc_Runtime_UninitializeThreadFunc (
	jnc_Runtime* runtime,
	jnc_ExceptionRecoverySnapshot* ers
	);

typedef
jnc_SjljFrame*
jnc_Runtime_SetSjljFrameFunc (
	jnc_Runtime* runtime,
	jnc_SjljFrame* frame
	);

typedef
void
jnc_Runtime_CheckStackOverflowFunc (jnc_Runtime* runtime);

typedef
jnc_Runtime*
jnc_GetCurrentThreadRuntimeFunc ();

typedef
void
jnc_PrimeClassFunc (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	const void* vtable
	);

typedef
size_t 
jnc_StrLenFunc (jnc_DataPtr ptr);

typedef
jnc_DataPtr
jnc_StrDupFunc (
	const char* p,
	size_t length
	);

typedef
jnc_DataPtr
jnc_MemDupFunc (
	const void* p,
	size_t size
	);

// GcHeap

typedef
jnc_Runtime*
jnc_GcHeap_GetRuntimeFunc (jnc_GcHeap* gcHeap);

typedef
void 
jnc_GcHeap_GetStatsFunc (
	jnc_GcHeap* gcHeap,
	jnc_GcStats* stats
	);

typedef
void 
jnc_GcHeap_GetSizeTriggersFunc (
	jnc_GcHeap* gcHeap,
	jnc_GcSizeTriggers* triggers
	);

typedef
void 
jnc_GcHeap_SetSizeTriggersFunc (
	jnc_GcHeap* gcHeap,
	const jnc_GcSizeTriggers* triggers
	);

typedef
void
jnc_GcHeap_CollectFunc (jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_EnterNoCollectRegionFunc (jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_LeaveNoCollectRegionFunc (
	jnc_GcHeap* gcHeap,
	int canCollectNow
	);

typedef
void
jnc_GcHeap_EnterWaitRegionFunc (jnc_GcHeap* gcHeap);

typedef
void
jnc_GcHeap_LeaveWaitRegionFunc (jnc_GcHeap* gcHeap);

typedef
jnc_IfaceHdr*
jnc_GcHeap_AllocateClassFunc (
	jnc_GcHeap* gcHeap,
	jnc_ClassType* type
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateDataFunc (
	jnc_GcHeap* gcHeap,
	jnc_Type* type
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateArrayFunc (
	jnc_GcHeap* gcHeap,
	jnc_Type* type,
	size_t count
	);

typedef
jnc_DataPtr
jnc_GcHeap_AllocateBufferFunc (
	jnc_GcHeap* gcHeap,
	size_t size
	);

typedef
jnc_DataPtrValidator*
jnc_GcHeap_CreateDataPtrValidatorFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	);

typedef
void
jnc_GcHeap_WeakMarkFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_MarkDataFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_MarkClassFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

#if (_JNC_OS_WIN)
typedef
int 
jnc_GcHeap_HandleGcSehExceptionFunc (
	jnc_GcHeap* gcHeap,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif

//.............................................................................

struct jnc_ErrorFuncTable
{
	jnc_GetLastErrorFunc* m_getLastErrorFunc;
	jnc_SetErrorFunc* m_setErrorFunc;
	jnc_getErrorDescriptionFunc* m_getErrorDescriptionFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleItemDeclFuncTable
{
	jnc_ModuleItemDecl_GetNameFunc* m_getNameFunc;
	jnc_ModuleItemDecl_GetQualifiedNameFunc* m_getQualifiedNameFunc;
	jnc_ModuleItemDecl_GetStorageKindFunc* m_getStorageKindFunc;
	jnc_ModuleItemDecl_GetAccessKindFunc* m_getAccessKindFunc;
	jnc_ModuleItemDecl_GetAttributeBlockFunc* m_getAttributeBlockFunc;
	jnc_ModuleItemDecl_GetParentNamespaceFunc* m_getParentNamespaceFunc;
	jnc_ModuleItemDecl_GetParentUnitFunc* m_getParentUnitFunc;
	jnc_ModuleItemDecl_GetLineFunc* m_getLineFunc;
	jnc_ModuleItemDecl_GetColFunc* m_getColFunc;
	jnc_ModuleItemDecl_GetOffsetFunc* m_getOffsetFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleItemFuncTable
{
	jnc_ModuleItem_GetModuleFunc* m_getModuleFunc;
	jnc_ModuleItem_GetItemKindFunc* m_getItemKindFunc;
	jnc_ModuleItem_GetFlagsFunc* m_getFlagsFunc;
	jnc_ModuleItem_GetDeclFunc* m_getDeclFunc;
	jnc_ModuleItem_GetNamespaceFunc* m_getNamespaceFunc;
	jnc_ModuleItem_GetTypeFunc* m_getTypeFunc;

	jnc_VerifyModuleItemIsDerivableTypeFunc* m_verifyModuleItemIsDerivableTypeFunc;
	jnc_VerifyModuleItemIsClassTypeFunc* m_verifyModuleItemIsClassTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_AttributeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_AttributeBlockFuncTable
{
	jnc_AttributeBlock_GetAttributeCountFunc* m_getAttributeCountFunc;
	jnc_AttributeBlock_GetAttributeFunc* m_getAttributeFunc;
	jnc_AttributeBlock_FindAttributeFunc* m_findAttributeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_NamespaceFuncTable
{
	jnc_Namespace_GetItemCountFunc* m_getItemCountFunc;
	jnc_Namespace_GetItemFunc* m_getItemFunc;
	jnc_Namespace_FindFunctionFunc* m_findFunctionFunc;
	jnc_Namespace_FindPropertyFunc* m_findPropertyFunc;
	jnc_Namespace_FindClassTypeFunc* m_findClassTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_VariableFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionFuncTable
{
	jnc_Function_GetFunctionKindFunc* m_getFunctionKindFunc;
	jnc_Function_IsMemberFunc* m_isMemberFunc;
	jnc_Function_IsOverloadedFunc* m_isOverloadedFunc;
	jnc_Function_GetOverloadCountFunc* m_getOverloadCountFunc;
	jnc_Function_GetOverloadFunc* m_getOverloadFunc;
	jnc_Function_GetMachineCodeFunc* m_getMachineCodeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyFuncTable
{
	jnc_Property_GetGetterFunc* m_getGetterFunc;
	jnc_Property_GetSetterFunc* m_getSetterFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_TypedefFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_TypeFuncTable
{
	jnc_Type_GetTypeKindFunc* m_getTypeKindFunc;
	jnc_Type_GetSizeFunc* m_getSizeFunc;
	jnc_Type_GetTypeStringFunc* m_getTypeStringFunc;
	jnc_Type_CmpFunc* m_cmpFunc;
	jnc_Type_GetDataPtrTypeFunc* m_getDataPtrTypeFunc;
	jnc_Type_MarkGcRootsFunc* m_markGcRootsFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_NamedTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_BaseTypeSlotFuncTable
{
	jnc_BaseTypeSlot_GetOffsetFunc* m_getOffsetFunc;
	jnc_BaseTypeSlot_GetVTableIndexFunc* m_getVTableIndexFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DerivableTypeFuncTable
{
	jnc_DerivableType_GetMemberMethodFunc* m_getStaticConstructorFunc;
	jnc_DerivableType_GetMemberMethodFunc* m_getStaticDestructorFunc;
	jnc_DerivableType_GetMemberMethodFunc* m_getPreConstructorFunc;
	jnc_DerivableType_GetMemberMethodFunc* m_getConstructorFunc;
	jnc_DerivableType_GetMemberMethodFunc* m_getDestructorFunc;
	jnc_DerivableType_GetUnaryOperatorFunc* m_getUnaryOperatorFunc;
	jnc_DerivableType_GetBinaryOperatorFunc* m_getBinaryOperatorFunc;
	jnc_DerivableType_GetMemberMethodFunc* m_getCallOperatorFunc;
	jnc_DerivableType_GetCastOperatorFunc* m_getCastOperatorFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ArrayTypeFuncTable
{
	jnc_ArrayType_GetElementTypeFunc* m_getElementTypeFunc;
	jnc_ArrayType_GetElementCountFunc* m_GetElementCountFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_BitFieldTypeFuncTable
{
	jnc_BitFieldType_GetBaseTypeFunc* m_getBaseTypeFunc;
	jnc_BitFieldType_GetBitOffsetFunc* m_getBitOffsetFunc;
	jnc_BitFieldType_GetBitCountFunc* m_getBitCountFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionArgFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionTypeFuncTable
{
	jnc_FunctionType_GetReturnTypeFunc* m_getReturnTypeFunc;
	jnc_FunctionType_GetArgCountFunc* m_getArgCountFunc;
	jnc_FunctionType_GetArgFunc* m_getArgFunc;
	jnc_FunctionType_GetFunctionPtrTypeFunc* m_getFunctionPtrTypeFunc;
	jnc_FunctionType_GetShortTypeFunc* m_getShortTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_EnumConstFuncTable
{
	jnc_EnumConst_GetValueFunc* m_getValueFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_EnumTypeFuncTable
{
	jnc_EnumType_GetBaseTypeFunc* m_getBaseTypeFunc;
	jnc_EnumType_GetConstCountFunc* m_getConstCountFunc;
	jnc_EnumType_GetConstFunc* m_getConstFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StructFieldFuncTable
{
	jnc_StructField_GetOffsetFunc* m_getOffsetFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StructTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_UnionTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ClassTypeFuncTable
{
	jnc_ClassType_GetClassTypeKindFunc* m_getClassTypeKindFunc;
	jnc_ClassType_GetIfaceStructTypeFunc* m_getIfaceStructTypeFunc;
	jnc_ClassType_GetClassPtrTypeFunc* m_getClassPtrTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_MulticastClassTypeFuncTable
{
	jnc_MulticastClassType_GetTargetTypeFunc* m_getTargetTypeFunc;
	jnc_MulticastClassType_GetMethodFunc* m_getMethodFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_McSnapshotClassTypeFuncTable
{
	jnc_McSnapshotClassType_GetTargetTypeFunc* m_getTargetTypeFunc;
	jnc_McSnapshotClassType_GetMethodFunc* m_getMethodFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DataPtrTypeFuncTable
{
	jnc_DataPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_DataPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ClassPtrTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionPtrTypeFuncTable
{
	jnc_FunctionPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_FunctionPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyPtrTypeFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_VariantFuncTable
{
	jnc_Variant_CastFunc* m_castFunc;
	jnc_Variant_UnaryOperatorFunc* m_unaryOperatorFunc;
	jnc_Variant_BinaryOperatorFunc* m_binaryOperatorFunc;
	jnc_Variant_RelationalOperatorFunc* m_relationalOperatorFunc;
	jnc_Variant_GetHashFunc* m_getHashFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_UnitFuncTable
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleFuncTable
{
	jnc_Module_CreateFunc* m_createFunc;
	jnc_Module_DestroyFunc* m_destroyFunc;
	jnc_Module_ClearFunc* m_clearFunc;
	jnc_Module_InitializeFunc* m_initializeFunc;
	jnc_Module_GetGlobalNamespaceFunc* m_getGlobalNamespaceFunc;
	jnc_Module_GetPrimitiveTypeFunc* m_getPrimitiveTypeFunc;
	jnc_Module_FindItemFunc* m_findItemFunc;
	jnc_Module_MapFunctionFunc* m_mapFunctionFunc;
	jnc_Module_AddSourceFunc* m_addSourceFunc;
	jnc_Module_AddImportDirFunc* m_addImportDirFunc;
	jnc_Module_AddImportFunc* m_addImportFunc;
	jnc_Module_AddOpaqueClassTypeInfoFunc* m_addOpaqueClassTypeInfoFunc;
	jnc_Module_AddStaticLibFunc* m_addStaticLibFunc;
	jnc_Module_ParseFunc* m_parseFunc;
	jnc_Module_ParseFileFunc* m_parseFileFunc;
	jnc_Module_ParseImportsFunc* m_parseImportsFunc;
	jnc_Module_CalcLayoutFunc* m_calcLayoutFunc;
	jnc_Module_CompileFunc* m_compileFunc;
	jnc_Module_JitFunc* m_jitFunc;
	jnc_Module_getLlvmIrStringFunc* m_getLlvmIrStringFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_RuntimeFuncTable
{
	jnc_Runtime_CreateFunc* m_createFunc;
	jnc_Runtime_DestroyFunc* m_destroyFunc;
	jnc_Runtime_GetModuleFunc* m_getModuleFunc;
	jnc_Runtime_GetGcHeapFunc* m_getGcHeapFunc;
	jnc_Runtime_GetStackSizeLimitFunc* m_getStackSizeLimitFunc;
	jnc_Runtime_SetStackSizeLimitFunc* m_setStackSizeLimitFunc;
	jnc_Runtime_StartupFunc* m_startupFunc;
	jnc_Runtime_ShutdownFunc* m_shutdownFunc;
	jnc_Runtime_InitializeThreadFunc* m_initializeThreadFunc;
	jnc_Runtime_UninitializeThreadFunc* m_uninitializeThreadFunc;
	jnc_Runtime_SetSjljFrameFunc* m_setSjljFrameFunc;
	jnc_Runtime_CheckStackOverflowFunc* m_checkStackOverflowFunc;
	jnc_GetCurrentThreadRuntimeFunc* m_getCurrentThreadRuntimeFunc;
	jnc_PrimeClassFunc* m_primeClassFunc;
	jnc_StrLenFunc* m_strLenFunc;
	jnc_StrDupFunc* m_strDupFunc;
	jnc_MemDupFunc* m_memDupFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcHeapFuncTable
{
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
	jnc_GcHeap_WeakMarkFunc* m_weakMarkFunc;
	jnc_GcHeap_MarkDataFunc* m_markDataFunc;
	jnc_GcHeap_MarkClassFunc* m_markClassFunc;
#if (_JNC_OS_WIN)
	jnc_GcHeap_HandleGcSehExceptionFunc* m_handleGcSehExceptionFunc;
#endif // _JNC_OS_WIN
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DynamicExtensionLibHost
{
	jnc_ErrorFuncTable* m_errorFuncTable;
	jnc_ModuleItemDeclFuncTable* m_moduleItemDeclFuncTable;
	jnc_ModuleItemFuncTable* m_moduleItemFuncTable;
	jnc_AttributeFuncTable* m_attributeFuncTable;
	jnc_AttributeBlockFuncTable* m_attributeBlockFuncTable;
	jnc_NamespaceFuncTable* m_namespaceFuncTable;
	jnc_VariableFuncTable* m_variableFuncTable;
	jnc_FunctionFuncTable* m_functionFuncTable;
	jnc_PropertyFuncTable* m_propertyFuncTable;
	jnc_TypedefFuncTable* m_typedefFuncTable;
	jnc_TypeFuncTable* m_typeFuncTable;
	jnc_NamedTypeFuncTable* m_namedTypeFuncTable;
	jnc_BaseTypeSlotFuncTable* m_baseTypeSlotFuncTable;
	jnc_DerivableTypeFuncTable* m_derivableTypeFuncTable;
	jnc_ArrayTypeFuncTable* m_arrayTypeFuncTable;
	jnc_BitFieldTypeFuncTable* m_bitFieldTypeFuncTable;
	jnc_FunctionArgFuncTable* m_functionArgFuncTable;
	jnc_FunctionTypeFuncTable* m_functionTypeFuncTable;
	jnc_PropertyTypeFuncTable* m_propertyTypeFuncTable;
	jnc_EnumConstFuncTable* m_enumConstFuncTable;
	jnc_EnumTypeFuncTable* m_enumTypeFuncTable;
	jnc_StructFieldFuncTable* m_structFieldFuncTable;
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
	jnc_UnitFuncTable* m_unitFuncTable;
	jnc_ModuleFuncTable* m_moduleFuncTable;
	jnc_RuntimeFuncTable* m_runtimeFuncTable;
	jnc_GcHeapFuncTable* m_gcHeapFuncTable;
};

//.............................................................................
