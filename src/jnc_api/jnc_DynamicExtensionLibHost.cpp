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
#include "jnc_rt_Runtime.h"

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

//..............................................................................

static jnc_ErrorFuncTable g_errorFuncTable =
{
	sizeof(jnc_ErrorFuncTable),
	jnc_getLastError,
	jnc_setError,
	jnc_setErrno,
	jnc_setStringError,
	jnc_getErrorDescription_v,
};

static jnc_ModuleItemDeclFuncTable g_moduleItemDeclFuncTable =
{
	sizeof(jnc_ModuleItemDeclFuncTable),
	jnc_ModuleItemDecl_getName,
	jnc_ModuleItemDecl_getQualifiedName,
	jnc_ModuleItemDecl_getStorageKind,
	jnc_ModuleItemDecl_getAccessKind,
	jnc_ModuleItemDecl_getAttributeBlock,
	jnc_ModuleItemDecl_getParentNamespace,
	jnc_ModuleItemDecl_getParentUnit,
	jnc_ModuleItemDecl_getLine,
	jnc_ModuleItemDecl_getCol,
	jnc_ModuleItemDecl_getOffset,
};

static jnc_ModuleItemFuncTable g_moduleItemFuncTable =
{
	sizeof(jnc_ModuleItemFuncTable),
	jnc_ModuleItem_getModule,
	jnc_ModuleItem_getItemKind,
	jnc_ModuleItem_getFlags,
	jnc_ModuleItem_getDecl,
	jnc_ModuleItem_getNamespace,
	jnc_ModuleItem_getType,
	jnc_verifyModuleItemIsDerivableType,
	jnc_verifyModuleItemIsClassType,
};

static jnc_AttributeFuncTable g_attributeFuncTable =
{
	sizeof(jnc_AttributeFuncTable),
};

static jnc_AttributeBlockFuncTable g_attributeBlockFuncTable =
{
	sizeof(jnc_AttributeBlockFuncTable),
	jnc_AttributeBlock_getAttributeCount,
	jnc_AttributeBlock_getAttribute,
	jnc_AttributeBlock_findAttribute,
};

static jnc_NamespaceFuncTable g_namespaceFuncTable =
{
	sizeof(jnc_NamespaceFuncTable),
	jnc_Namespace_getItemCount,
	jnc_Namespace_getItem,
	jnc_Namespace_findVariable,
	jnc_Namespace_findFunction,
	jnc_Namespace_findProperty,
	jnc_Namespace_findClassType,
};

static jnc_GlobalNamespaceFuncTable g_globalNamespaceFuncTable =
{
	sizeof(jnc_GlobalNamespaceFuncTable),
};

static jnc_VariableFuncTable g_variableFuncTable =
{
	sizeof(jnc_VariableFuncTable),
};

static jnc_FunctionFuncTable g_functionFuncTable =
{
	sizeof(jnc_FunctionFuncTable),
	jnc_Function_getFunctionKind,
	jnc_Function_isMember,
	jnc_Function_isOverloaded,
	jnc_Function_getOverloadCount,
	jnc_Function_getOverload,
	jnc_Function_getMachineCode,
};

static jnc_PropertyFuncTable g_propertyFuncTable =
{
	sizeof(jnc_PropertyFuncTable),
	jnc_Property_getGetter,
	jnc_Property_getSetter,
};

static jnc_TypedefFuncTable g_typedefFuncTable =
{
	sizeof(jnc_TypedefFuncTable),
};

static jnc_TypeFuncTable g_typeFuncTable =
{
	sizeof(jnc_TypeFuncTable),
	jnc_Type_getTypeKind,
	jnc_Type_getSize,
	jnc_Type_getTypeString,
	jnc_Type_cmp,
	jnc_Type_getDataPtrType,
	jnc_Type_markGcRoots,
};

static jnc_NamedTypeFuncTable g_namedTypeFuncTable =
{
	sizeof(jnc_NamedTypeFuncTable),
};

static jnc_BaseTypeSlotFuncTable g_baseTypeSlotFuncTable =
{
	sizeof(jnc_BaseTypeSlotFuncTable),
	jnc_BaseTypeSlot_getOffset,
	jnc_BaseTypeSlot_getVTableIndex,
};

static jnc_DerivableTypeFuncTable g_derivableTypeFuncTable =
{
	sizeof(jnc_DerivableTypeFuncTable),
	jnc_DerivableType_getStaticConstructor,
	jnc_DerivableType_getStaticDestructor,
	jnc_DerivableType_getPreConstructor,
	jnc_DerivableType_getConstructor,
	jnc_DerivableType_getDestructor,
	jnc_DerivableType_getUnaryOperator,
	jnc_DerivableType_getBinaryOperator,
	jnc_DerivableType_getCallOperator,
	jnc_DerivableType_getCastOperator,
};

static jnc_ArrayTypeFuncTable g_arrayTypeFuncTable =
{
	sizeof(jnc_ArrayTypeFuncTable),
	jnc_ArrayType_getElementType,
	jnc_ArrayType_getElementCount,
};

static jnc_BitFieldTypeFuncTable g_bitFieldTypeFuncTable =
{
	sizeof(jnc_BitFieldTypeFuncTable),
	jnc_BitFieldType_getBaseType,
	jnc_BitFieldType_getBitOffset,
	jnc_BitFieldType_getBitCount,
};

static jnc_FunctionArgFuncTable g_functionArgFuncTable =
{
	sizeof(jnc_FunctionArgFuncTable),
};

static jnc_FunctionTypeFuncTable g_functionTypeFuncTable =
{
	sizeof(jnc_FunctionTypeFuncTable),
	jnc_FunctionType_getReturnType,
	jnc_FunctionType_getArgCount,
	jnc_FunctionType_getArg,
	jnc_FunctionType_getFunctionPtrType,
	jnc_FunctionType_getShortType,
};

static jnc_PropertyTypeFuncTable g_propertyTypeFuncTable =
{
	sizeof(jnc_PropertyTypeFuncTable),
};

static jnc_EnumConstFuncTable g_enumConstFuncTable =
{
	sizeof(jnc_EnumConstFuncTable),
	jnc_EnumConst_getValue,
};

static jnc_EnumTypeFuncTable g_enumTypeFuncTable =
{
	sizeof(jnc_EnumTypeFuncTable),
	jnc_EnumType_getBaseType,
	jnc_EnumType_getConstCount,
	jnc_EnumType_getConst,
};

static jnc_StructFieldFuncTable g_structFieldFuncTable =
{
	sizeof(jnc_StructFieldFuncTable),
	jnc_StructField_getOffset,
};

static jnc_StructTypeFuncTable g_structTypeFuncTable =
{
	sizeof(jnc_StructTypeFuncTable),
};

static jnc_UnionTypeFuncTable g_unionTypeFuncTable =
{
	sizeof(jnc_UnionTypeFuncTable),
};

static jnc_ClassTypeFuncTable g_classTypeFuncTable =
{
	sizeof(jnc_ClassTypeFuncTable),
	jnc_ClassType_getClassTypeKind,
	jnc_ClassType_getIfaceStructType,
	jnc_ClassType_getClassPtrType,
};

static jnc_MulticastClassTypeFuncTable g_multicastClassTypeFuncTable =
{
	sizeof(jnc_MulticastClassTypeFuncTable),
	jnc_MulticastClassType_getTargetType,
	jnc_MulticastClassType_getMethod,
};

static jnc_McSnapshotClassTypeFuncTable g_mcSnapshotClassTypeFuncTable =
{
	sizeof(jnc_McSnapshotClassTypeFuncTable),
	jnc_McSnapshotClassType_getTargetType,
	jnc_McSnapshotClassType_getMethod,
};

static jnc_DataPtrTypeFuncTable g_dataPtrTypeFuncTable =
{
	sizeof(jnc_DataPtrTypeFuncTable),
	jnc_DataPtrType_getPtrTypeKind,
	jnc_DataPtrType_getTargetType,
};

static jnc_ClassPtrTypeFuncTable g_classPtrTypeFuncTable =
{
	sizeof(jnc_ClassPtrTypeFuncTable),
};

static jnc_FunctionPtrTypeFuncTable g_functionPtrTypeFuncTable =
{
	sizeof(jnc_FunctionPtrTypeFuncTable),
	jnc_FunctionPtrType_getPtrTypeKind,
	jnc_FunctionPtrType_getTargetType,
};

static jnc_PropertyPtrTypeFuncTable g_propertyPtrTypeFuncTable =
{
	sizeof(jnc_PropertyPtrTypeFuncTable),
};

static jnc_VariantFuncTable g_variantFuncTable =
{
	sizeof(jnc_VariantFuncTable),
	jnc_Variant_cast,
	jnc_Variant_unaryOperator,
	jnc_Variant_binaryOperator,
	jnc_Variant_relationalOperator,
	jnc_Variant_getMember,
	jnc_Variant_setMember,
	jnc_Variant_getElement,
	jnc_Variant_setElement,
	jnc_Variant_hash,
};

static jnc_UnitFuncTable g_unitFuncTable =
{
	sizeof(jnc_UnitFuncTable),
};

static jnc_ModuleFuncTable g_moduleFuncTable =
{
	sizeof(jnc_ModuleFuncTable),
	jnc_Module_create,
	jnc_Module_destroy,
	jnc_Module_clear,
	jnc_Module_initialize,
	jnc_Module_getGlobalNamespace,
	jnc_Module_getPrimitiveType,
	jnc_Module_findItem,
	jnc_Module_mapVariable,
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
	sizeof(jnc_RuntimeFuncTable),
	jnc_Runtime_create,
	jnc_Runtime_destroy,
	jnc_Runtime_getModule,
	jnc_Runtime_getGcHeap,
	jnc_Runtime_isAborted,
	jnc_Runtime_getStackSizeLimit,
	jnc_Runtime_setStackSizeLimit,
	jnc_Runtime_startup,
	jnc_Runtime_shutdown,
	jnc_Runtime_abort,
	jnc_Runtime_initializeCallSite,
	jnc_Runtime_uninitializeCallSite,
	jnc_Runtime_setSjljFrame,
	jnc_Runtime_getUserData,
	jnc_Runtime_setUserData,
	jnc_getCurrentThreadRuntime,
	jnc_getCurrentThreadTls,
	jnc_dynamicThrow,
	jnc_primeClass,
	jnc_strLen,
	jnc_strDup,
	jnc_memDup,
};

static jnc_GcHeapFuncTable g_gcHeapFuncTable =
{
	sizeof(jnc_GcHeapFuncTable),
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
	jnc_GcHeap_getDynamicLayout,
	jnc_GcHeap_resetDynamicLayout,
	jnc_GcHeap_weakMark,
	jnc_GcHeap_markData,
	jnc_GcHeap_markClass,
	jnc_GcHeap_addBoxToCallSite,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl =
{
	sizeof(jnc_DynamicExtensionLibHost),
	err::getErrorMgr(),
	&g_errorFuncTable,
	&g_moduleItemDeclFuncTable,
	&g_moduleItemFuncTable,
	&g_attributeFuncTable,
	&g_attributeBlockFuncTable,
	&g_namespaceFuncTable,
	&g_globalNamespaceFuncTable,
	&g_variableFuncTable,
	&g_functionFuncTable,
	&g_propertyFuncTable,
	&g_typedefFuncTable,
	&g_typeFuncTable,
	&g_namedTypeFuncTable,
	&g_baseTypeSlotFuncTable,
	&g_derivableTypeFuncTable,
	&g_arrayTypeFuncTable,
	&g_bitFieldTypeFuncTable,
	&g_functionArgFuncTable,
	&g_functionTypeFuncTable,
	&g_propertyTypeFuncTable,
	&g_enumConstFuncTable,
	&g_enumTypeFuncTable,
	&g_structFieldFuncTable,
	&g_structTypeFuncTable,
	&g_unionTypeFuncTable,
	&g_classTypeFuncTable,
	&g_multicastClassTypeFuncTable,
	&g_mcSnapshotClassTypeFuncTable,
	&g_dataPtrTypeFuncTable,
	&g_classPtrTypeFuncTable,
	&g_functionPtrTypeFuncTable,
	&g_propertyPtrTypeFuncTable,
	&g_variantFuncTable,
	&g_unitFuncTable,
	&g_moduleFuncTable,
	&g_runtimeFuncTable,
	&g_gcHeapFuncTable,
};

//..............................................................................
