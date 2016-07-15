#include "pch.h"
#include "jnc_DynamicExtensionLibHost.h"
#include "jnc_Error.h"
#include "jnc_DerivableType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_Namespace.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_GcHeap.h"

//.............................................................................

static jnc_ErrorFuncTable g_errorFuncTable = 
{
	jnc_getLastError,
	jnc_setError,
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

static jnc_FunctionFuncTable g_functionFuncTable = 
{
	jnc_Function_getOverload,
	jnc_Function_getMachineCode,
	jnc_getMulticastCallMethod,
};

static jnc_PropertyFuncTable g_propertyFuncTable = 
{
	jnc_Property_getGetter,
	jnc_Property_getSetter,
};

static jnc_NamespaceFuncTable g_namespaceFuncTable = 
{
	jnc_Namespace_findFunction,
	jnc_Namespace_findProperty,
};

static jnc_ModuleFuncTable g_moduleFuncTable = 
{	
	jnc_Module_getGlobalNamespace,
	jnc_Module_findItem,
	jnc_Module_mapFunction,
	jnc_Module_addSource,
	jnc_Module_addOpaqueClassTypeInfo,
	jnc_verifyModuleItemIsDerivableType,
	jnc_verifyModuleItemIsClassType,
};

static jnc_RuntimeFuncTable g_runtimeFuncTable = 
{
	jnc_Runtime_getModule,
	jnc_Runtime_getGcHeap,
	jnc_Runtime_initializeThread,
	jnc_Runtime_uninitializeThread,
	jnc_getCurrentThreadRuntime,
	jnc_primeClass,
	jnc_strLen,
	jnc_strDup,
	jnc_memDup,
};

static jnc_GcHeapFuncTable g_gcHeapFuncTable =
{
	jnc_GcHeap_enterNoCollectRegion,
	jnc_GcHeap_leaveNoCollectRegion,
	jnc_GcHeap_enterWaitRegion,
	jnc_GcHeap_leaveWaitRegion,
	jnc_GcHeap_allocateClass,
	jnc_GcHeap_allocateData,
	jnc_GcHeap_allocateArray,
	jnc_GcHeap_createDataPtrValidator,
	jnc_GcHeap_gcWeakMark,
	jnc_GcHeap_gcMarkData,
	jnc_GcHeap_gcMarkClass,
#if (_AXL_ENV == AXL_ENV_WIN)
	jnc_GcHeap_handleGcSehException,
#endif // _AXL_ENV
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl = 
{
	&g_errorFuncTable,
	&g_derivableTypeFuncTable,
	&g_functionFuncTable,
	&g_propertyFuncTable,
	&g_namespaceFuncTable,
	&g_moduleFuncTable,
	&g_runtimeFuncTable,
	&g_gcHeapFuncTable,
};

//.............................................................................
