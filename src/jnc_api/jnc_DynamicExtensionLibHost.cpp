#include "pch.h"
#include "jnc_DynamicExtensionLibHost.h"
#include "jnc_Error.h"
#include "jnc_Type.h"
#include "jnc_ArrayType.h"
#include "jnc_EnumType.h"
#include "jnc_DerivableType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_Namespace.h"
#include "jnc_Variant.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_GcHeap.h"

#//.............................................................................

static jnc_ErrorFuncTable g_errorFuncTable = 
{
	jnc_getLastError,
	jnc_setError,
};

static jnc_ModuleItemFuncTable g_moduleItemFuncTable =
{
	jnc_ModuleItem_getModule,
	jnc_ModuleItem_getItemKind,
	jnc_ModuleItem_getFlags,
	jnc_verifyModuleItemIsDerivableType,
	jnc_verifyModuleItemIsClassType,
};

static jnc_TypeFuncTable g_typeFuncTable = 
{
	jnc_Type_getTypeKind,
	jnc_Type_getSize,
	jnc_Type_getTypeString,
	jnc_Type_cmp,
	jnc_Type_getDataPtrType,
	jnc_Type_markGcRoots,
};

static jnc_ArrayTypeFuncTable g_arrayTypeFuncTable =
{
	jnc_ArrayType_getElementType,
	jnc_ArrayType_getElementCount,
};

static jnc_FunctionTypeFuncTable g_functionTypeFuncTable =
{
	jnc_FunctionType_getReturnType,
	jnc_FunctionType_getArgCount,
};

static jnc_DataPtrTypeFuncTable g_dataPtrTypeFuncTable =
{
	jnc_DataPtrType_getPtrTypeKind,
	jnc_DataPtrType_getTargetType,
};

static jnc_FunctionPtrTypeFuncTable g_functionPtrTypeFuncTable =
{
	jnc_FunctionPtrType_getPtrTypeKind,
	jnc_FunctionPtrType_getTargetType,
};

static jnc_DerivableTypeFuncTable g_derivableTypeFuncTable = 
{
	jnc_DerivableType_getPreConstructor,
	jnc_DerivableType_getConstructor,
	jnc_DerivableType_getDestructor,
	jnc_DerivableType_getUnaryOperator,
	jnc_DerivableType_getBinaryOperator,
	jnc_DerivableType_getCallOperator,
	jnc_DerivableType_getCastOperator,
	jnc_DerivableType_getNamespace,
};

static jnc_MulticastClassTypeFuncTable g_multicastClassTypeFuncTable =
{
	jnc_MulticastClassType_getTargetType,
	jnc_MulticastClassType_getMethod,
};

static jnc_McSnapshotClassTypeFuncTable g_mcSnapshotClassTypeFuncTable =
{
	jnc_McSnapshotClassType_getTargetType,
	jnc_McSnapshotClassType_getMethod,
};

static jnc_FunctionFuncTable g_functionFuncTable = 
{
	jnc_Function_getType,
	jnc_Function_getOverload,
	jnc_Function_getMachineCode,
};

static jnc_PropertyFuncTable g_propertyFuncTable = 
{
	jnc_Property_getType,
	jnc_Property_getGetter,
	jnc_Property_getSetter,
};

static jnc_NamespaceFuncTable g_namespaceFuncTable = 
{
	jnc_Namespace_findFunction,
	jnc_Namespace_findProperty,
};

static jnc_GlobalNamespaceFuncTable g_globalNamespaceFuncTable = 
{
	jnc_GlobalNamespace_getItemDecl,
	jnc_GlobalNamespace_getNamespace,
};

static jnc_VariantFuncTable g_variantFuncTable;

static jnc_ModuleFuncTable g_moduleFuncTable = 
{	
	jnc_Module_create,
	jnc_Module_destroy,
	jnc_Module_clear,
	jnc_Module_initialize,
	jnc_Module_getGlobalNamespace,
	jnc_Module_getPrimitiveType,
	jnc_Module_findItem,
	jnc_Module_mapFunction,
	jnc_Module_addSource,
	jnc_Module_addImportDir,
	jnc_Module_addImport,
	jnc_Module_addOpaqueClassTypeInfo,
	jnc_Module_addStaticLib,
	jnc_Module_parse,
	jnc_Module_parseFile,
	jnc_Module_parseImports,
	jnc_Module_calcLayout,
	jnc_Module_compile,
	jnc_Module_jit,
	jnc_Module_getLlvmIrString_v,

};

static jnc_RuntimeFuncTable g_runtimeFuncTable = 
{
	jnc_Runtime_create,
	jnc_Runtime_destroy,
	jnc_Runtime_getModule,
	jnc_Runtime_getGcHeap,
	jnc_Runtime_getStackSizeLimit,
	jnc_Runtime_setStackSizeLimit,
	jnc_Runtime_startup,
	jnc_Runtime_shutdown,
	jnc_Runtime_initializeThread,
	jnc_Runtime_uninitializeThread,
	jnc_Runtime_checkStackOverflow,
	jnc_getCurrentThreadRuntime,
	jnc_primeClass,
	jnc_strLen,
	jnc_strDup,
	jnc_memDup,
};

static jnc_GcHeapFuncTable g_gcHeapFuncTable =
{
	jnc_GcHeap_getRuntime,
	jnc_GcHeap_getStats,
	jnc_GcHeap_getSizeTriggers,
	jnc_GcHeap_setSizeTriggers,
	jnc_GcHeap_collect,
	jnc_GcHeap_enterNoCollectRegion,
	jnc_GcHeap_leaveNoCollectRegion,
	jnc_GcHeap_enterWaitRegion,
	jnc_GcHeap_leaveWaitRegion,
	jnc_GcHeap_allocateClass,
	jnc_GcHeap_tryAllocateClass,
	jnc_GcHeap_allocateData,
	jnc_GcHeap_tryAllocateData,
	jnc_GcHeap_allocateArray,
	jnc_GcHeap_tryAllocateArray,
	jnc_GcHeap_allocateBuffer,
	jnc_GcHeap_tryAllocateBuffer,
	jnc_GcHeap_createDataPtrValidator,
	jnc_GcHeap_weakMark,
	jnc_GcHeap_markData,
	jnc_GcHeap_markClass,
#if (_AXL_ENV == AXL_ENV_WIN)
	jnc_GcHeap_handleGcSehException,
#endif // _AXL_ENV
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl = 
{
	&g_errorFuncTable,
	&g_moduleItemFuncTable,
	&g_typeFuncTable,
	&g_arrayTypeFuncTable,
	&g_functionTypeFuncTable,
	&g_dataPtrTypeFuncTable,
	&g_functionPtrTypeFuncTable,
	&g_derivableTypeFuncTable,
	&g_multicastClassTypeFuncTable,
	&g_mcSnapshotClassTypeFuncTable,
	&g_functionFuncTable,
	&g_propertyFuncTable,
	&g_namespaceFuncTable,
	&g_globalNamespaceFuncTable,
	&g_variantFuncTable,
	&g_moduleFuncTable,
	&g_runtimeFuncTable,
	&g_gcHeapFuncTable,
};

//.............................................................................
