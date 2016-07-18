#pragma once

#define _JNC_DYNAMICEXTENSIONLIBHOST_H

#include "jnc_RuntimeStructs.h"
#include "jnc_Multicast.h"
#include "jnc_Variant.h"
#include "jnc_OpKind.h"

//.............................................................................

typedef struct jnc_ErrorFuncTable jnc_ErrorFuncTable;
typedef struct jnc_TypeFuncTable jnc_TypeFuncTable;
typedef struct jnc_DerivableTypeFuncTable jnc_DerivableTypeFuncTable;
typedef struct jnc_FunctionFuncTable jnc_FunctionFuncTable;
typedef struct jnc_MulticastFuncTable jnc_MulticastFuncTable;
typedef struct jnc_PropertyFuncTable jnc_PropertyFuncTable;
typedef struct jnc_NamespaceFuncTable jnc_NamespaceFuncTable;
typedef struct jnc_VariantFuncTable jnc_VariantFuncTable;
typedef struct jnc_ModuleFuncTable jnc_ModuleFuncTable;
typedef struct jnc_RuntimeFuncTable jnc_RuntimeFuncTable;
typedef struct jnc_DynamicExtensionLibHost jnc_DynamicExtensionLibHost;

//.............................................................................

// Error

typedef
jnc_Error*
jnc_GetLastErrorFunc ();

typedef
void
jnc_SetErrorFunc (jnc_Error* error);

// Type

typedef
size_t
jnc_Type_GetSizeFunc (jnc_Type* type);

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

// Function

typedef
jnc_Function*
jnc_Function_GetOverloadFunc (
	jnc_Function* function,
	size_t overloadIdx
	);

typedef
void*
jnc_Function_GetMachineCodeFunc (jnc_Function* function);

// Multicast

typedef
jnc_Function*
jnc_Multicast_getCallMethodFunc (jnc_Multicast* multicast);

typedef
jnc_Function*
jnc_McSnapshot_getCallMethodFunc (jnc_McSnapshot* multicast);

// Property

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

// Variant

// Module

typedef
jnc_Namespace*
jnc_Module_GetGlobalNamespaceFunc (jnc_Module* module);

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
	size_t size
	);

typedef
void
jnc_Module_AddOpaqueClassTypeInfoFunc (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

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

// Runtime

typedef
jnc_Module*
jnc_Runtime_GetModuleFunc (jnc_Runtime* runtime);

typedef
jnc_GcHeap*
jnc_Runtime_GetGcHeapFunc (jnc_Runtime* runtime);

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
jnc_Runtime*
jnc_GetCurrentThreadRuntimeFunc ();

typedef
void
jnc_PrimeClassFunc (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable
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
jnc_DataPtrValidator*
jnc_GcHeap_CreateDataPtrValidatorFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	);

typedef
void
jnc_GcHeap_GcWeakMarkFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_GcMarkDataFunc (
	jnc_GcHeap* gcHeap,	
	jnc_Box* box
	);

typedef
void
jnc_GcHeap_GcMarkClassFunc (
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

struct jnc_TypeFuncTable
{
	jnc_Type_GetSizeFunc* m_getSizeFunc;
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

struct jnc_FunctionFuncTable
{
	jnc_Function_GetOverloadFunc* m_getOverloadFunc;
	jnc_Function_GetMachineCodeFunc* m_getMachineCodeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_MulticastFuncTable
{
	jnc_Multicast_getCallMethodFunc* m_getCallMethodFunc;
	jnc_McSnapshot_getCallMethodFunc* m_getSnapshotCallMethodFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_PropertyFuncTable
{
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

struct jnc_VariantFuncTable
{
};
//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ModuleFuncTable
{
	jnc_Module_GetGlobalNamespaceFunc* m_getGlobalNamespaceFunc;
	jnc_Module_FindItemFunc* m_findItemFunc;
	jnc_Module_MapFunctionFunc* m_mapFunctionFunc;
	jnc_Module_AddSourceFunc* m_addSourceFunc;
	jnc_Module_AddOpaqueClassTypeInfoFunc* m_addOpaqueClassTypeInfoFunc;
	jnc_VerifyModuleItemIsDerivableTypeFunc* m_verifyModuleItemIsDerivableTypeFunc;
	jnc_VerifyModuleItemIsClassTypeFunc* m_verifyModuleItemIsClassTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_RuntimeFuncTable
{
	jnc_Runtime_GetModuleFunc* m_getModuleFunc;
	jnc_Runtime_GetGcHeapFunc* m_getGcHeapFunc;
	jnc_Runtime_InitializeThreadFunc* m_initializeThreadFunc;
	jnc_Runtime_UninitializeThreadFunc* m_uninitializeThreadFunc;
	jnc_GetCurrentThreadRuntimeFunc* m_getCurrentThreadRuntimeFunc;
	jnc_PrimeClassFunc* m_primeClassFunc;
	jnc_StrLenFunc* m_strLenFunc;
	jnc_StrDupFunc* m_strDupFunc;
	jnc_MemDupFunc* m_memDupFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_GcHeapFuncTable
{
	jnc_GcHeap_EnterNoCollectRegionFunc* m_enterNoCollectRegionFunc;
	jnc_GcHeap_LeaveNoCollectRegionFunc* m_leaveNoCollectRegionFunc;
	jnc_GcHeap_EnterWaitRegionFunc* m_enterWaitRegionFunc;
	jnc_GcHeap_LeaveWaitRegionFunc* m_leaveWaitRegionFunc;
	jnc_GcHeap_AllocateClassFunc* m_allocateClassFunc;
	jnc_GcHeap_AllocateDataFunc* m_allocateDataFunc;
	jnc_GcHeap_AllocateArrayFunc* m_allocateArrayFunc;
	jnc_GcHeap_CreateDataPtrValidatorFunc* m_createDataPtrValidatorFunc;
	jnc_GcHeap_GcWeakMarkFunc* m_gcWeakMarkFunc;
	jnc_GcHeap_GcMarkDataFunc* m_gcMarkDataFunc;
	jnc_GcHeap_GcMarkClassFunc* m_gcMarkClassFunc;
#if (_AXL_ENV == AXL_ENV_WIN)
	jnc_GcHeap_HandleGcSehExceptionFunc* m_handleGcSehExceptionFunc;
#endif // _AXL_ENV
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DynamicExtensionLibHost
{
	jnc_ErrorFuncTable* m_errorFuncTable;
	jnc_TypeFuncTable* m_TypeFuncTable;
	jnc_DerivableTypeFuncTable* m_derivableTypeFuncTable;
	jnc_FunctionFuncTable* m_functionFuncTable;
	jnc_PropertyFuncTable* m_propertyFuncTable;
	jnc_MulticastFuncTable* m_multicastFuncTable;
	jnc_NamespaceFuncTable* m_namespaceFuncTable;
	jnc_VariantFuncTable* m_variantFuncTable;
	jnc_ModuleFuncTable* m_moduleFuncTable;
	jnc_RuntimeFuncTable* m_runtimeFuncTable;
	jnc_GcHeapFuncTable* m_gcHeapFuncTable;
};

//.............................................................................
