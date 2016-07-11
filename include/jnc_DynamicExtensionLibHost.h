// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Error.h"
#include "jnc_DerivableType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_Namespace.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

//.............................................................................

typedef struct jnc_Error_FuncTable jnc_Error_FuncTable;
typedef struct jnc_DerivableType_FuncTable jnc_DerivableType_FuncTable;
typedef struct jnc_Function_FuncTable jnc_Function_FuncTable;
typedef struct jnc_Property_FuncTable jnc_Property_FuncTable;
typedef struct jnc_Namespace_FuncTable jnc_Namespace_FuncTable;
typedef struct jnc_Module_FuncTable jnc_Module_FuncTable;
typedef struct jnc_Runtime_FuncTable jnc_Runtime_FuncTable;
typedef struct jnc_DynamicExtensionLibHost jnc_DynamicExtensionLibHost;

//.............................................................................

// Error

typedef
jnc_Error*
jnc_GetLastErrorFunc ();

typedef
void
jnc_SetErrorFunc (jnc_Error* error);

typedef
void
jnc_PropagateLastErrorFunc ();

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

typedef
void
jnc_PrimeClassFunc (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable
	);

// Function

typedef
jnc_Function*
jnc_Function_GetOverloadFunc (
	jnc_Function* self,
	size_t overloadIdx
	);

typedef
void*
jnc_Function_GetMachineCodeFunc (jnc_Function* self);

// Property

typedef
jnc_Function*
jnc_Property_GetGetterFunc (jnc_Property* self);

typedef
jnc_Function*
jnc_Property_GetSetterFunc (jnc_Property* self);

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
jnc_Module_GetGlobalNamespaceFunc (jnc_Module* self);

typedef
jnc_ModuleItem*
jnc_Module_FindItemFunc (
	jnc_Module* self,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	);

typedef
void
jnc_Module_MapFunctionFunc (
	jnc_Module* self,
	jnc_Function* function,
	void* p
	);

typedef
bool
jnc_Module_AddImportFunc (
	jnc_Module* self,
	const char* fileName
	);

typedef
void
jnc_Module_AddSourceFunc (
	jnc_Module* self,
	const char* fileName,
	const char* source,
	size_t size
	);

// Runtime

typedef
jnc_ModuleItem*
jnc_Runtime_FindModuleItemFunc (
	jnc_Runtime* self,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	);

typedef
void
jnc_Runtime_InitializeThreadFunc (
	jnc_Runtime* self,
	jnc_ExceptionRecoverySnapshot* ers
	);

typedef
void
jnc_Runtime_UninitializeThreadFunc (
	jnc_Runtime* self,
	jnc_ExceptionRecoverySnapshot* ers
	);

typedef
void
jnc_Runtime_EnterNoCollectRegionFunc (jnc_Runtime* self);

typedef
void
jnc_Runtime_LeaveNoCollectRegionFunc (
	jnc_Runtime* self,
	int canCollectNow
	);

typedef
void
jnc_Runtime_EnterWaitRegionFunc (jnc_Runtime* self);

typedef
void
jnc_Runtime_LeaveWaitRegionFunc (jnc_Runtime* self);

typedef
jnc_IfaceHdr*
jnc_Runtime_AllocateClassFunc (
	jnc_Runtime* self,
	jnc_ClassType* type
	);

typedef
jnc_DataPtr
jnc_Runtime_AllocateDataFunc (
	jnc_Runtime* self,
	jnc_Type* type
	);

typedef
jnc_DataPtr
jnc_Runtime_AllocateArrayFunc (
	jnc_Runtime* self,
	jnc_Type* type,
	size_t count
	);

typedef
jnc_DataPtrValidator*
jnc_Runtime_CreateDataPtrValidatorFunc (
	jnc_Runtime* self,	
	jnc_Box* box,
	void* rangeBegin,
	size_t rangeLength
	);

typedef
void
jnc_Runtime_GcWeakMarkFunc (
	jnc_Runtime* self,	
	jnc_Box* box
	);

typedef
void
jnc_Runtime_GcMarkDataFunc (
	jnc_Runtime* self,	
	jnc_Box* box
	);

typedef
void
jnc_Runtime_GcMarkClassFunc (
	jnc_Runtime* self,	
	jnc_Box* box
	);

#if (_AXL_ENV == AXL_ENV_WIN)
typedef
int 
jnc_Runtime_HandleGcSehExceptionFunc (
	jnc_Runtime* self,
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	);
#endif // _AXL_ENV

typedef
jnc_Runtime*
jnc_GetCurrentThreadRuntimeFunc ();

//.............................................................................

struct jnc_Error_FuncTable
{
	jnc_GetLastErrorFunc* m_getLastErrorFunc;
	jnc_SetErrorFunc* m_setErrorFunc;
	jnc_PropagateLastErrorFunc* m_propagateLastErrorFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DerivableType_FuncTable
{
	jnc_DerivableType_GetPreConstructorFunc* m_getPreConstructorFunc;
	jnc_DerivableType_GetConstructorFunc* m_GetConstructorFunc;
	jnc_DerivableType_GetDestructorFunc* m_GetDestructorFunc;
	jnc_DerivableType_GetUnaryOperatorFunc* m_GetUnaryOperatorFunc;
	jnc_DerivableType_GetBinaryOperatorFunc* m_GetBinaryOperatorFunc;
	jnc_DerivableType_GetCallOperatorFunc* m_GetCallOperatorFunc;
	jnc_DerivableType_GetCastOperatorFunc* m_GetCastOperatorFunc;
	jnc_DerivableType_GetNamespaceFunc* m_GetNamespaceFunc;
	jnc_PrimeClassFunc* m_primeClassFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Function_FuncTable
{
	jnc_Function_GetOverloadFunc* m_getOverloadFunc;
	jnc_Function_GetMachineCodeFunc* m_getMachineCodeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Property_FuncTable
{
	jnc_Property_GetGetterFunc* m_getGetterFunc;
	jnc_Property_GetSetterFunc* m_getSetterFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Namespace_FuncTable
{
	jnc_Namespace_FindFunctionFunc* m_findFunctionFunc;
	jnc_Namespace_FindPropertyFunc* m_findPropertyFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Module_FuncTable
{
	jnc_Module_GetGlobalNamespaceFunc* m_getGlobalNamespaceFunc;
	jnc_Module_FindItemFunc* m_findItemFunc;
	jnc_Module_MapFunctionFunc* m_mapFunctionFunc;
	jnc_Module_AddImportFunc* m_addImportFunc;
	jnc_Module_AddSourceFunc* m_addSourceFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_Runtime_FuncTable
{
	jnc_Runtime_FindModuleItemFunc* m_findModuleItemFunc;
	jnc_Runtime_InitializeThreadFunc* m_initializeThreadFunc;
	jnc_Runtime_UninitializeThreadFunc* m_uninitializeThreadFunc;
	jnc_Runtime_EnterNoCollectRegionFunc* m_enterNoCollectRegionFunc;
	jnc_Runtime_LeaveNoCollectRegionFunc* m_leaveNoCollectRegionFunc;
	jnc_Runtime_EnterWaitRegionFunc* m_enterWaitRegionFunc;
	jnc_Runtime_LeaveWaitRegionFunc* m_leaveWaitRegionFunc;
	jnc_Runtime_AllocateClassFunc* m_allocateClassFunc;
	jnc_Runtime_AllocateDataFunc* m_allocateDataFunc;
	jnc_Runtime_AllocateArrayFunc* m_allocateArrayFunc;
	jnc_Runtime_CreateDataPtrValidatorFunc* m_createDataPtrValidatorFunc;
	jnc_Runtime_GcWeakMarkFunc* m_gcWeakMarkFunc;
	jnc_Runtime_GcMarkDataFunc* m_gcMarkDataFunc;
	jnc_Runtime_GcMarkClassFunc* m_gcMarkClassFunc;
#if (_AXL_ENV == AXL_ENV_WIN)
	jnc_Runtime_HandleGcSehExceptionFunc* m_handleGcSehExceptionFunc;
#endif // _AXL_ENV
	jnc_GetCurrentThreadRuntimeFunc* m_getCurrentThreadRuntimeFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_DynamicExtensionLibHost
{
	jnc_Error_FuncTable* m_errorFuncTable;
	jnc_DerivableType_FuncTable* m_derivableTypeFuncTable;
	jnc_Function_FuncTable* m_functionFuncTable;
	jnc_Property_FuncTable* m_propertyFuncTable;
	jnc_Namespace_FuncTable* m_namespaceFuncTable;
	jnc_Module_FuncTable* m_moduleFuncTable;
	jnc_Runtime_FuncTable* m_runtimeFuncTable;
};

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
extern jnc_DynamicExtensionLibHost* g_jnc_dynamicExtensionLibHost;
#endif

//.............................................................................

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
Function*
getPropertyGetter (Property* prop)
{
	return g_extensionLibHost->getPropertyGetter (prop);
}

inline 
Function*
getPropertySetter (Property* prop)
{
	return g_extensionLibHost->getPropertySetter (prop);
}

inline 
Function*
getTypePreConstructor (DerivableType* type)
{
	return g_extensionLibHost->getTypePreConstructor (type);
}

inline 
Function*
getTypeConstructor (DerivableType* type)
{
	return g_extensionLibHost->getTypeConstructor (type);
}

inline 
Function*
getClassTypeDestructor (ClassType* type)
{
	return g_extensionLibHost->getClassTypeDestructor (type);
}

inline 
Function*
getTypeUnaryOperator (
	DerivableType* type,
	UnOpKind opKind	
	)
{
	return g_extensionLibHost->getTypeUnaryOperator (type, opKind);
}

inline 
Function*
getTypeBinaryOperator (
	DerivableType* type,
	BinOpKind opKind	
	)
{
	return g_extensionLibHost->getTypeBinaryOperator (type, opKind);
}

inline 
Function*
getTypeCallOperator (DerivableType* type)
{
	return g_extensionLibHost->getTypeCallOperator (type);
}

inline 
Function*
getTypeCastOperator (
	DerivableType* type,
	size_t idx
	)
{
	return g_extensionLibHost->getTypeCastOperator (type, idx);
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

inline
Function*
getPropertyGetter (Property* prop)
{
	return prop->getGetter ();
}

Function*
getPropertySetter (Property* prop);

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
