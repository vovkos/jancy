// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"
#include "jnc_OpKind.h"

//.............................................................................

typedef struct jnc_ErrorFuncTable jnc_ErrorFuncTable;
typedef struct jnc_DerivableTypeFuncTable jnc_DerivableTypeFuncTable;
typedef struct jnc_FunctionFuncTable jnc_FunctionFuncTable;
typedef struct jnc_PropertyFuncTable jnc_PropertyFuncTable;
typedef struct jnc_NamespaceFuncTable jnc_NamespaceFuncTable;
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

typedef
jnc_Function*
jnc_GetMulticastCallMethodFunc (jnc_Multicast* multicast);

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
bool
jnc_Module_AddImportFunc (
	jnc_Module* module,
	const char* fileName
	);

typedef
void
jnc_Module_AddSourceFunc (
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t size
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
jnc_ModuleItem*
jnc_Runtime_FindModuleItemFunc (
	jnc_Runtime* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

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
jnc_GcHeap*
jnc_Runtime_GetGcHeapFunc (jnc_Runtime* runtime);

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
	jnc_GetMulticastCallMethodFunc* m_getMulticastCallMethodFunc;
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

struct jnc_ModuleFuncTable
{
	jnc_Module_GetGlobalNamespaceFunc* m_getGlobalNamespaceFunc;
	jnc_Module_FindItemFunc* m_findItemFunc;
	jnc_Module_MapFunctionFunc* m_mapFunctionFunc;
	jnc_Module_AddImportFunc* m_addImportFunc;
	jnc_Module_AddSourceFunc* m_addSourceFunc;
	jnc_VerifyModuleItemIsDerivableTypeFunc* m_verifyModuleItemIsDerivableTypeFunc;
	jnc_VerifyModuleItemIsClassTypeFunc* m_verifyModuleItemIsClassTypeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_RuntimeFuncTable
{
	jnc_Runtime_FindModuleItemFunc* m_findModuleItemFunc;
	jnc_Runtime_InitializeThreadFunc* m_initializeThreadFunc;
	jnc_Runtime_UninitializeThreadFunc* m_uninitializeThreadFunc;
	jnc_Runtime_GetGcHeapFunc* m_getGcHeapFunc;
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
	jnc_DerivableTypeFuncTable* m_derivableTypeFuncTable;
	jnc_FunctionFuncTable* m_functionFuncTable;
	jnc_PropertyFuncTable* m_propertyFuncTable;
	jnc_NamespaceFuncTable* m_namespaceFuncTable;
	jnc_ModuleFuncTable* m_moduleFuncTable;
	jnc_RuntimeFuncTable* m_runtimeFuncTable;
	jnc_GcHeapFuncTable* m_gcHeapFuncTable;
};

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_GetLastErrorFunc GetLastErrorFunc;
typedef jnc_SetErrorFunc SetErrorFunc;
typedef jnc_DerivableType_GetPreConstructorFunc GetPreConstructorFunc;
typedef jnc_DerivableType_GetConstructorFunc DerivableType_GetConstructorFunc;
typedef jnc_DerivableType_GetDestructorFunc DerivableType_GetDestructorFunc;
typedef jnc_DerivableType_GetUnaryOperatorFunc DerivableType_GetUnaryOperatorFunc;
typedef jnc_DerivableType_GetBinaryOperatorFunc DerivableType_GetBinaryOperatorFunc;
typedef jnc_DerivableType_GetCallOperatorFunc DerivableType_GetCallOperatorFunc;
typedef jnc_DerivableType_GetCastOperatorFunc DerivableType_GetCastOperatorFunc;
typedef jnc_DerivableType_GetNamespaceFunc DerivableType_GetNamespaceFunc;
typedef jnc_PrimeClassFunc PrimeClassFunc;
typedef jnc_Function_GetOverloadFunc Function_GetOverloadFunc;
typedef jnc_Function_GetMachineCodeFunc Function_GetMachineCodeFunc;
typedef jnc_Property_GetGetterFunc Property_GetGetterFunc;
typedef jnc_Property_GetSetterFunc Property_GetSetterFunc;
typedef jnc_Namespace_FindFunctionFunc Namespace_FindFunctionFunc;
typedef jnc_Namespace_FindPropertyFunc Namespace_FindPropertyFunc;
typedef jnc_Module_GetGlobalNamespaceFunc Module_GetGlobalNamespaceFunc;
typedef jnc_Module_FindItemFunc Module_FindItemFunc;
typedef jnc_Module_MapFunctionFunc Module_MapFunctionFunc;
typedef jnc_Module_AddImportFunc Module_AddImportFunc;
typedef jnc_Module_AddSourceFunc Module_AddSourceFunc;
typedef jnc_Runtime_FindModuleItemFunc Runtime_FindModuleItemFunc;
typedef jnc_Runtime_InitializeThreadFunc Runtime_InitializeThreadFunc;
typedef jnc_Runtime_UninitializeThreadFunc Runtime_UninitializeThreadFunc;
typedef jnc_Runtime_GetGcHeapFunc Runtime_GetGcHeapFunc;
typedef jnc_GetCurrentThreadRuntimeFunc GetCurrentThreadRuntimeFunc;
typedef jnc_GcHeap_EnterNoCollectRegionFunc GcHeap_EnterNoCollectRegionFunc;
typedef jnc_GcHeap_LeaveNoCollectRegionFunc GcHeap_LeaveNoCollectRegionFunc;
typedef jnc_GcHeap_EnterWaitRegionFunc GcHeap_EnterWaitRegionFunc;
typedef jnc_GcHeap_LeaveWaitRegionFunc GcHeap_LeaveWaitRegionFunc;
typedef jnc_GcHeap_AllocateClassFunc GcHeap_AllocateClassFunc;
typedef jnc_GcHeap_AllocateDataFunc GcHeap_AllocateDataFunc;
typedef jnc_GcHeap_AllocateArrayFunc GcHeap_AllocateArrayFunc;
typedef jnc_GcHeap_CreateDataPtrValidatorFunc GcHeap_CreateDataPtrValidatorFunc;
typedef jnc_GcHeap_GcWeakMarkFunc GcHeap_GcWeakMarkFunc;
typedef jnc_GcHeap_GcMarkDataFunc GcHeap_GcMarkDataFunc;
typedef jnc_GcHeap_GcMarkClassFunc GcHeap_GcMarkClassFunc;
typedef jnc_GcHeap_HandleGcSehExceptionFunc GcHeap_HandleGcSehExceptionFunc;

typedef jnc_ErrorFuncTable ErrorFuncTable;
typedef jnc_DerivableTypeFuncTable DerivableTypeFuncTable;
typedef jnc_FunctionFuncTable FunctionFuncTable;
typedef jnc_PropertyFuncTable PropertyFuncTable;
typedef jnc_NamespaceFuncTable NamespaceFuncTable;
typedef jnc_ModuleFuncTable ModuleFuncTable;
typedef jnc_RuntimeFuncTable RuntimeFuncTable;
typedef jnc_DynamicExtensionLibHost DynamicExtensionLibHost;

} // namespace jnc

#endif // __cplusplus

#if 0

#ifdef _JNC_CORE
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#else
#	include "jnc_OpKind.h"
#endif

namespace jnc {
namespace ext {

class ExtensionLib;

#ifdef _JNC_SHARED_EXTENSION_LIB
	
// treat everything as opaque classes

class Runtime; 
class Module; 
class ModuleItem;
class Namespace;
class Function;
class Property;

// a tiny hack to reduce number of overloads

class Type
{
};

class DerivableType: public Type
{
};

class ClassType: public DerivableType
{
};

#else

typedef rt::Runtime       Runtime;
typedef ct::Module        Module;
typedef ct::ModuleItem    ModuleItem;
typedef ct::Namespace     Namespace;
typedef ct::Function      Function;
typedef ct::Property      Property;
typedef ct::Type          Type;
typedef ct::DerivableType DerivableType;
typedef ct::ClassType     ClassType;

#endif

//.............................................................................

#ifdef _JNC_SHARED_EXTENSION_LIB

extern ExtensionLibHost* g_extensionLibHost;

inline
Namespace*
getModuleGlobalNamespace (Module* module)
{
	return g_extensionLibHost->getModuleGlobalNamespace (module);
}

inline 
ModuleItem*
findModuleItem (
	Module* module,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return g_extensionLibHost->findModuleItem (module, name, libCacheSlot, itemCacheSlot);
}

inline 
ModuleItem*
findModuleItem (
	Runtime* runtime,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return g_extensionLibHost->findModuleItem (runtime, name, libCacheSlot, itemCacheSlot);
}

inline 
Function*
findNamespaceFunction (
	Namespace* nspace,
	const char* name
	)
{
	return g_extensionLibHost->findNamespaceFunction (nspace, name);
}

inline 
Property*
findNamespaceProperty (
	Namespace* nspace,
	const char* name
	)
{
	return g_extensionLibHost->findNamespaceProperty (nspace, name);
}

inline
Function*
getFunctionOverload (
	Function* function,
	size_t overloadIdx
	)
{
	return g_extensionLibHost->getFunctionOverload (function, overloadIdx);
}



inline
DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return g_extensionLibHost->verifyModuleItemIsDerivableType (item, name);
}

inline
ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	return g_extensionLibHost->verifyModuleItemIsClassType (item, name);
}

inline
Namespace*
getTypeNamespace (DerivableType* type)
{
	return g_extensionLibHost->getTypeNamespace (type);
}

inline
void
mapFunction (
	Module* module,
	Function* function,
	void* p
	)
{
	g_extensionLibHost->mapFunction (module, function, p);
}

inline
bool
addModuleImport (
	Module* module,
	const char* fileName
	)
{
	return g_extensionLibHost->addImport (module, fileName);
}

inline
void
addModuleSource (
	Module* module,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	g_extensionLibHost->addSource (module, fileName, source, length);
}

inline 
void
setError (const err::ErrorHdr* errorHdr)
{
	g_extensionLibHost->setError (errorHdr);
}

inline 
void
setError (const err::Error& error)
{
	g_extensionLibHost->setError (error);
}

inline 
void
propagateLastError ()
{
	g_extensionLibHost->setError (err::getLastError ());
}

#else

ExtensionLibHost*
getStdExtensionLibHost ();

inline
Namespace*
getModuleGlobalNamespace (Module* module)
{
	return module->m_namespaceMgr.getGlobalNamespace ();
}

inline 
ModuleItem*
findModuleItem (
	Module* module,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return module->m_extensionLibMgr.findItem (name, libCacheSlot, itemCacheSlot);
}

inline 
ModuleItem*
findModuleItem (
	Runtime* runtime,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return runtime->getModule ()->m_extensionLibMgr.findItem (name, libCacheSlot, itemCacheSlot);
}

inline 
Function*
findNamespaceFunction (
	Namespace* nspace,
	const char* name
	)
{
	return nspace->findFunctionByName (name);
}

inline 
Property*
findNamespaceProperty (
	Namespace* nspace,
	const char* name
	)
{
	return nspace->findPropertyByName (name);
}

Function*
getFunctionOverload (
	Function* function,
	size_t overloadIdx
	);


Function*
getTypePreConstructor (DerivableType* type);

Function*
getTypeConstructor (DerivableType* type);

Function*
getClassTypeDestructor (ClassType* type);

Function*
getTypeUnaryOperator (
	DerivableType* type,
	UnOpKind opKind	
	);

Function*
getTypeBinaryOperator (
	DerivableType* type,
	BinOpKind opKind	
	);

Function*
getTypeCallOperator (DerivableType* type);

Function*
getTypeCastOperator (
	DerivableType* type,
	size_t idx
	);

inline
DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return ct::verifyModuleItemIsDerivableType (item, name);
}

inline
ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	return ct::verifyModuleItemIsClassType (item, name);
}

inline
Namespace*
getTypeNamespace (DerivableType* type)
{
	return type;
}

inline
void
mapFunction (
	Module* module,
	Function* function,
	void* p
	)
{
	module->mapFunction (function, p);
}

inline
bool
addModuleImport (
	Module* module,
	const char* fileName
	)
{
	return module->m_importMgr.addImport (fileName);
}

inline
void
addModuleSource (
	Module* module,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	module->m_importMgr.addSource (fileName, axl::sl::StringRef (source, length));
}

#endif

//.............................................................................

} // namespace ext
} // namespace jnc

#endif
