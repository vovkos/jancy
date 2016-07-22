#pragma once

#define _JNC_DYNAMICEXTENSIONLIBHOST_H

#include "jnc_RuntimeStructs.h"
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

// ArrayType

typedef
jnc_Type*
jnc_ArrayType_GetElementTypeFunc (jnc_ArrayType* type);

typedef
size_t
jnc_ArrayType_GetElementCountFunc (jnc_ArrayType* type);

// FunctionType

typedef
jnc_Type*
jnc_FunctionType_GetReturnTypeFunc (jnc_FunctionType* type);

typedef
size_t
jnc_FunctionType_GetArgCountFunc (jnc_FunctionType* type);

// DataPtrType

typedef
jnc_DataPtrTypeKind
jnc_DataPtrType_GetPtrTypeKindFunc (jnc_DataPtrType* type);

typedef
jnc_Type*
jnc_DataPtrType_GetTargetTypeFunc (jnc_DataPtrType* type);

// FunctionPtrType

typedef
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_GetPtrTypeKindFunc (jnc_FunctionPtrType* type);

typedef
jnc_FunctionType*
jnc_FunctionPtrType_GetTargetTypeFunc (jnc_FunctionPtrType* type);

// DerivableType

typedef
jnc_Function*
jnc_DerivableType_GetPreConstructorFunc (jnc_DerivableType* type);

typedef
jnc_Function*
jnc_DerivableType_GetConstructorFunc (jnc_DerivableType* type);

typedef
jnc_Function*
jnc_DerivableType_GetDestructorFunc (jnc_DerivableType* type);

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
jnc_DerivableType_GetCallOperatorFunc (jnc_DerivableType* type);

typedef
jnc_Function*
jnc_DerivableType_GetCastOperatorFunc (
	jnc_DerivableType* type,
	size_t idx
	);

typedef
jnc_Namespace*
jnc_DerivableType_GetNamespaceFunc (jnc_DerivableType* type);

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

typedef
jnc_FunctionPtrType*
jnc_McSnapshotClassType_GetTargetTypeFunc (jnc_McSnapshotClassType* type);

typedef
jnc_Function*
jnc_McSnapshotClassType_GetMethodFunc (
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	);

// Function

typedef
jnc_FunctionType*
jnc_Function_GetTypeFunc (jnc_Function* function);

typedef
jnc_Function*
jnc_Function_GetOverloadFunc (
	jnc_Function* function,
	size_t overloadIdx
	);

typedef
void*
jnc_Function_GetMachineCodeFunc (jnc_Function* function);

// Property

typedef
jnc_PropertyType*
jnc_Property_GetTypeFunc (jnc_Property* prop);

typedef
jnc_Function*
jnc_Property_GetGetterFunc (jnc_Property* prop);

typedef
jnc_Function*
jnc_Property_GetSetterFunc (jnc_Property* prop);

// Namespace

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

// GlobalNamespace

typedef
jnc_ModuleItemDecl*
jnc_GlobalNamespace_GetItemDeclFunc (jnc_GlobalNamespace* nspace);

typedef
jnc_Namespace*
jnc_GlobalNamespace_GetNamespaceFunc (jnc_GlobalNamespace* nspace);

// Variant

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
int
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
jnc_Module_GetLlvmIrStringFunc (jnc_Module* module);

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

#if (_AXL_ENV == AXL_ENV_WIN)
typedef
int 
jnc_GcHeap_HandleGcSehExceptionFunc (
	jnc_GcHeap* gcHeap,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif // _AXL_ENV

//.............................................................................

struct jnc_ErrorFuncTable
{
	jnc_GetLastErrorFunc* m_getLastErrorFunc;
	jnc_SetErrorFunc* m_setErrorFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleItemFuncTable
{
	jnc_ModuleItem_GetModuleFunc* m_getModuleFunc;
	jnc_ModuleItem_GetItemKindFunc* m_getItemKindFunc;
	jnc_ModuleItem_GetFlagsFunc* m_getFlagsFunc;
	jnc_VerifyModuleItemIsDerivableTypeFunc* m_verifyModuleItemIsDerivableTypeFunc;
	jnc_VerifyModuleItemIsClassTypeFunc* m_verifyModuleItemIsClassTypeFunc;
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

struct jnc_ArrayTypeFuncTable
{
	jnc_ArrayType_GetElementTypeFunc* m_getElementTypeFunc;
	jnc_ArrayType_GetElementCountFunc* m_GetElementCountFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionTypeFuncTable
{
	jnc_FunctionType_GetReturnTypeFunc* m_getReturnTypeFunc;
	jnc_FunctionType_GetArgCountFunc* m_getArgCountFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DataPtrTypeFuncTable
{
	jnc_DataPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_DataPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_FunctionPtrTypeFuncTable
{
	jnc_FunctionPtrType_GetPtrTypeKindFunc* m_getPtrTypeKindFunc;
	jnc_FunctionPtrType_GetTargetTypeFunc* m_getTargetTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DerivableTypeFuncTable
{
	jnc_DerivableType_GetPreConstructorFunc* m_getPreConstructorFunc;
	jnc_DerivableType_GetConstructorFunc* m_getConstructorFunc;
	jnc_DerivableType_GetDestructorFunc* m_getDestructorFunc;
	jnc_DerivableType_GetUnaryOperatorFunc* m_getUnaryOperatorFunc;
	jnc_DerivableType_GetBinaryOperatorFunc* m_getBinaryOperatorFunc;
	jnc_DerivableType_GetCallOperatorFunc* m_getCallOperatorFunc;
	jnc_DerivableType_GetCastOperatorFunc* m_getCastOperatorFunc;
	jnc_DerivableType_GetNamespaceFunc* m_getNamespaceFunc;
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

struct jnc_FunctionFuncTable
{
	jnc_Function_GetTypeFunc* m_getTypeFunc;
	jnc_Function_GetOverloadFunc* m_getOverloadFunc;
	jnc_Function_GetMachineCodeFunc* m_getMachineCodeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyFuncTable
{
	jnc_Property_GetTypeFunc* m_getTypeFunc;
	jnc_Property_GetGetterFunc* m_getGetterFunc;
	jnc_Property_GetSetterFunc* m_getSetterFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_NamespaceFuncTable
{
	jnc_Namespace_FindFunctionFunc* m_findFunctionFunc;
	jnc_Namespace_FindPropertyFunc* m_findPropertyFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GlobalNamespaceFuncTable
{
	jnc_GlobalNamespace_GetItemDeclFunc* m_getItemDeclFunc;
	jnc_GlobalNamespace_GetNamespaceFunc* m_getNamespaceFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_VariantFuncTable
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
	jnc_Module_GetLlvmIrStringFunc* m_getLlvmIrStringFunc;
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
#if (_AXL_ENV == AXL_ENV_WIN)
	jnc_GcHeap_HandleGcSehExceptionFunc* m_handleGcSehExceptionFunc;
#endif // _AXL_ENV
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DynamicExtensionLibHost
{
	jnc_ErrorFuncTable* m_errorFuncTable;
	jnc_ModuleItemFuncTable* m_moduleItemFuncTable;
	jnc_TypeFuncTable* m_typeFuncTable;
	jnc_ArrayTypeFuncTable* m_arrayTypeFuncTable;
	jnc_FunctionTypeFuncTable* m_functionTypeFuncTable;
	jnc_DataPtrTypeFuncTable* m_dataPtrTypeFuncTable;
	jnc_FunctionPtrTypeFuncTable* m_functionPtrTypeFuncTable;
	jnc_DerivableTypeFuncTable* m_derivableTypeFuncTable;
	jnc_MulticastClassTypeFuncTable* m_multicastClassTypeFuncTable;
	jnc_McSnapshotClassTypeFuncTable* m_mcSnapshotClassTypeFuncTable;
	jnc_FunctionFuncTable* m_functionFuncTable;
	jnc_PropertyFuncTable* m_propertyFuncTable;
	jnc_NamespaceFuncTable* m_namespaceFuncTable;
	jnc_GlobalNamespaceFuncTable* m_globalNamespaceFuncTable;
	jnc_VariantFuncTable* m_variantFuncTable;
	jnc_ModuleFuncTable* m_moduleFuncTable;
	jnc_RuntimeFuncTable* m_runtimeFuncTable;
	jnc_GcHeapFuncTable* m_gcHeapFuncTable;
};

//.............................................................................
