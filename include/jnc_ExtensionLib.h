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

#define _JNC_EXTENSIONLIB_H

#include "jnc_DynamicExtensionLibHost.h"
#include "jnc_Module.h"

/**

\defgroup extension-lib-subsystem Extension Libraries Subsystem

\addtogroup extension-lib-subsystem
@{

\struct jnc_DynamicExtensionLibHost
	\verbatim

	Opaque structure passed to the dynamic extension library main function.

	Indirectly provides access to runtime and module of the Jancy host(on behalf of which this dynamic extension library is being loaded).

	\endverbatim

*/

typedef struct jnc_ExtensionLib jnc_ExtensionLib;

//..............................................................................

typedef
void
jnc_ExtensionLib_AddSourcesFunc(jnc_Module* module);

typedef
void
jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc(jnc_Module* module);

typedef
bool_t
jnc_ExtensionLib_MapAddressesFunc(jnc_Module* module);

typedef
void
jnc_ExtensionLib_UpdateCapabilitiesFunc();

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ExtensionLib {
	const jnc_Guid* m_guid;
	const char* m_name;
	const char* m_description;

	jnc_ExtensionLib_AddSourcesFunc* m_addSourcesFunc;
	jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc* m_addOpaqueClassTypeInfosFunc;
	jnc_ExtensionLib_MapAddressesFunc* m_mapAddressesFunc;
	jnc_ExtensionLib_UpdateCapabilitiesFunc* m_updateCapabilitiesFunc;
};

//..............................................................................

typedef
jnc_ExtensionLib*
jnc_DynamicExtensionLibMainFunc(jnc_DynamicExtensionLibHost* host);

typedef
bool_t
jnc_DynamicExtensionLibUnloadFunc();

JNC_SELECT_ANY char jnc_g_dynamicExtensionLibMainFuncName[] = "jncDynamicExtensionLibMain";
JNC_SELECT_ANY char jnc_g_dynamicExtensionLibUnloadFuncName[] = "jncDynamicExtensionLibUnload";

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
extern jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;
#elif (defined _JNC_CORE)
extern jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl;
#endif

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define JNC_DECLARE_LIB(LibPrefix) \
	JNC_EXTERN_C \
	jnc_ExtensionLib* \
	LibPrefix##_getLib();

#define JNC_DEFINE_LIB_EX(LibPrefix, libGuid, libName, libDescription, updateCapabilities) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addSources(jnc_Module* module); \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addOpaqueClassTypeInfos(jnc_Module* module); \
	JNC_EXTERN_C \
	bool_t \
	LibPrefix##_mapAddresses(jnc_Module* module); \
	JNC_EXTERN_C \
	void \
	LibPrefix##_updateCapabilities(); \
	JNC_EXTERN_C \
	JNC_EXPORT_O \
	jnc_ExtensionLib* \
	LibPrefix##_getLib() { \
		static jnc_ExtensionLib lib = { \
			&(libGuid), \
			libName, \
			libDescription, \
			LibPrefix##_addSources, \
			LibPrefix##_addOpaqueClassTypeInfos, \
			LibPrefix##_mapAddresses, \
			updateCapabilities \
		}; \
		return &lib; \
	}

#define JNC_DEFINE_LIB(LibPrefix, libGuid, libName, libDescription) \
	JNC_DEFINE_LIB_EX(LibPrefix, libGuid, libName, libDescription, NULL)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE(LibPrefix) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addSources(jnc_Module* module) { \
		jnc_ExtensionLib* lib = LibPrefix##_getLib();

#define JNC_LIB_SOURCE_FILE(fileName, sourceVar) \
	jnc_Module_addSource(module, lib, fileName, sourceVar, sizeof(sourceVar) - 1);

#define JNC_LIB_IMPORT(fileName) \
	jnc_Module_addImport(module, fileName);

#define JNC_LIB_REQUIRE_EX(itemKind, name, flags) \
	jnc_Module_require(module, itemKind, name, flags);

#define JNC_LIB_REQUIRE(itemKind, name) \
	JNC_LIB_REQUIRE_EX(itemKind, name, jnc_ModuleRequireFlag_Essential)

#define JNC_LIB_REQUIRE_TYPE_EX(typeKind, name, flags) \
	jnc_Module_requireType(module, typeKind, name, flags);

#define JNC_LIB_REQUIRE_TYPE(typeKind, name) \
	JNC_LIB_REQUIRE_TYPE_EX(typeKind, name, jnc_ModuleRequireFlag_Essential)

#define JNC_END_LIB_SOURCE_FILE_TABLE() \
	}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(LibPrefix) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addOpaqueClassTypeInfos(jnc_Module* module) {

#define JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(TypePrefix) \
		jnc_Module_addOpaqueClassTypeInfo( \
			module, \
			TypePrefix##_getQualifiedName(), \
			TypePrefix##_getOpaqueClassTypeInfo() \
		);

#define JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_FUNCTION_MAP(LibPrefix) \
	JNC_EXTERN_C \
	bool_t \
	LibPrefix##_mapAddresses(jnc_Module* module) { \
		bool_t result = 1; \
		jnc_FindModuleItemResult findResult = jnc_g_nullFindModuleItemResult; \
		jnc_Variable* variable = NULL; \
		jnc_Function* function = NULL; \
		jnc_FunctionOverload* functionOverload = NULL; \
		jnc_OverloadableFunction overloadableFunction = { NULL }; \
		jnc_Property* prop = NULL; \
		jnc_GlobalNamespace* global = jnc_Module_getGlobalNamespace(module); \
		jnc_Namespace* nspace = jnc_ModuleItem_getNamespace((jnc_ModuleItem*)global); \
		size_t overloadIdx = 0; \

#define JNC_END_LIB_FUNCTION_MAP() \
		return 1; \
	}

//..............................................................................

#define JNC_DECLARE_TYPE_EX(TypePrefix, JncType) \
	JNC_EXTERN_C \
	const char* \
	TypePrefix##_getQualifiedName(); \
	JNC_EXTERN_C \
	JncType* \
	TypePrefix##_getType(jnc_Module* module); \
	JNC_EXTERN_C \
	bool_t \
	TypePrefix##_mapAddresses ( \
		jnc_Module* module, \
		bool_t isRequired \
	);

#define JNC_DECLARE_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX( \
		TypePrefix, \
		jnc_DerivableType \
	)

#define JNC_DECLARE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX( \
		TypePrefix, \
		jnc_ClassType \
	) \
	const void* \
	TypePrefix##_getVtable();

#define JNC_DECLARE_OPAQUE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_CLASS_TYPE(TypePrefix) \
	JNC_EXTERN_C \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo();

//..............................................................................

#define JNC_DEFINE_TYPE_EX(TypePrefix, JncType, qualifiedName, libGuid, cacheSlot) \
	JNC_EXTERN_C \
	const char* \
	TypePrefix##_getQualifiedName() { \
		return qualifiedName; \
	} \
	JNC_EXTERN_C \
	JncType* \
	TypePrefix##_getType(jnc_Module* module) { \
		jnc_FindModuleItemResult findResult = jnc_Module_findExtensionLibItem( \
			module, \
			qualifiedName, \
			&(libGuid), \
			cacheSlot \
		); \
		return \
			findResult.m_item && \
			jnc_ModuleItem_getItemKind(findResult.m_item) == jnc_ModuleItemKind_Type ? \
			(JncType*)(jnc_Type*)findResult.m_item : NULL; \
	}

#define JNC_DEFINE_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_DEFINE_TYPE_EX( \
		TypePrefix, \
		jnc_DerivableType, \
		qualifiedName, \
		libGuid, \
		cacheSlot \
	)

#define JNC_DEFINE_CLASS_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_DEFINE_TYPE_EX( \
		TypePrefix, \
		jnc_ClassType, \
		qualifiedName, \
		libGuid, \
		cacheSlot \
	)

#define JNC_DEFINE_OPAQUE_CLASS_TYPE_EX(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc, requireOpaqueItemsFunc, isNonCreatable) \
	JNC_DEFINE_CLASS_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_EXTERN_C \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo() { \
		static jnc_OpaqueClassTypeInfo typeInfo = { \
			sizeof(Type), \
			(jnc_MarkOpaqueGcRootsFunc*)jnc_pvoid_cast(markOpaqueGcRootsFunc), \
			requireOpaqueItemsFunc, \
			isNonCreatable, \
		}; \
		return &typeInfo; \
	}

#define JNC_DEFINE_OPAQUE_CLASS_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc) \
	JNC_DEFINE_OPAQUE_CLASS_TYPE_EX( \
		TypePrefix, \
		qualifiedName, \
		libGuid, \
		cacheSlot, \
		Type, \
		markOpaqueGcRootsFunc, \
		NULL, \
		0 \
	)

#define JNC_DEFINE_OPAQUE_CLASS_TYPE_REQ(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc) \
	JNC_EXTERN_C \
	void \
	TypePrefix##_requireOpaqueItems(jnc_Module* module); \
	JNC_DEFINE_OPAQUE_CLASS_TYPE_EX( \
		TypePrefix, \
		qualifiedName, \
		libGuid, \
		cacheSlot, \
		Type, \
		markOpaqueGcRootsFunc, \
		TypePrefix##_requireOpaqueItems, \
		0 \
	)

#define JNC_DEFINE_OPAQUE_CLASS_TYPE_NC(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc) \
	JNC_DEFINE_OPAQUE_CLASS_TYPE_EX( \
		TypePrefix, \
		qualifiedName, \
		libGuid, \
		cacheSlot, \
		Type, \
		markOpaqueGcRootsFunc,  \
		NULL, \
		1 \
	)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_DECLARE_TYPE_STATIC_METHODS_EX(TypePrefix, JncType) \
	static \
	const char* \
	getQualifiedName() { \
		return TypePrefix##_getQualifiedName(); \
	} \
	static \
	JncType* \
	getType(jnc::Module* module) { \
		return TypePrefix##_getType(module); \
	} \
	static \
	bool \
	mapAddresses ( \
		jnc::Module* module, \
		bool isRequired \
	) { \
		return TypePrefix##_mapAddresses(module, isRequired) != 0; \
	}

#define JNC_DECLARE_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_TYPE_STATIC_METHODS_EX( \
		TypePrefix, \
		jnc::DerivableType \
	)

#define JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_TYPE_STATIC_METHODS_EX( \
		TypePrefix, \
		jnc::ClassType \
	) \
	static \
	const void* \
	getVtable() { \
		return TypePrefix##_getVtable(); \
	}

#define JNC_DECLARE_OPAQUE_CLASS_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(TypePrefix) \
	static \
	const jnc_OpaqueClassTypeInfo* \
	getOpaqueClassTypeInfo() { \
		return TypePrefix##_getOpaqueClassTypeInfo(); \
	}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_CLASS_TYPE_VTABLE(TypePrefix) \
	const void* \
	TypePrefix##_getVtable() { \
		static const void* const vtable[] = {

#define JNC_CLASS_TYPE_VTABLE_ENTRY(function) \
			jnc_pvoid_cast(function),

#define JNC_END_CLASS_TYPE_VTABLE() \
			NULL, \
		}; \
		return vtable; \
	}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_OPAQUE_CLASS_REQUIRE_TABLE(TypePrefix) \
	void \
	TypePrefix##_requireOpaqueItems(jnc_Module* module) {

#define JNC_OPAQUE_CLASS_REQUIRE(itemKind, name) \
		jnc_Module_require(module, itemKind, name, jnc_ModuleRequireFlag_Essential);

#define JNC_OPAQUE_CLASS_REQUIRE_TYPE(typeKind, name) \
		jnc_Module_requireType(module, typeKind, name, jnc_ModuleRequireFlag_Essential);

#define JNC_END_OPAQUE_CLASS_REQUIRE_TABLE() \
	}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_TYPE_FUNCTION_MAP(TypePrefix) \
	bool_t \
	TypePrefix##_mapAddresses ( \
		jnc_Module* module, \
		bool_t isRequired \
	) { \
		bool_t result = 1; \
		jnc_FindModuleItemResult findResult = jnc_g_nullFindModuleItemResult; \
		jnc_Namespace* nspace = NULL; \
		jnc_Function* function = NULL; \
		jnc_FunctionOverload* functionOverload = NULL; \
		jnc_OverloadableFunction overloadableFunction = { NULL }; \
		jnc_Property* prop = NULL; \
		jnc_Variable* variable = NULL; \
		size_t overloadIdx = 0; \
		jnc_DerivableType* type = (jnc_DerivableType*)TypePrefix##_getType(module); \
		if (!type) \
			return !isRequired; \
		nspace = jnc_ModuleItem_getNamespace((jnc_ModuleItem*)type); \
		if (!jnc_Namespace_isReady(nspace)) \
			return !isRequired; \

#define JNC_END_TYPE_FUNCTION_MAP() \
		return 1; \
	}

//..............................................................................

#define JNC_MAP_TYPE_IMPL(TypePrefix, isRequired) \
	result = TypePrefix##_mapAddresses(module, isRequired); \
	if (!result) \
		return 0;

#define JNC_MAP_TYPE(TypePrefix) \
	JNC_MAP_TYPE_IMPL(TypePrefix, 0)

#define JNC_MAP_TYPE_REQ(TypePrefix) \
	JNC_MAP_TYPE_IMPL(TypePrefix, 1)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_MAP_FUNCTION_IMPL(function, p) \
	if (function) { \
		result = jnc_Module_mapFunction(module, function, jnc_pvoid_cast(p)); \
		if (!result) \
			return 0; \
	}

#define JNC_MAP_OVERLOADABLE_FUNCTION(item, p) \
	function = NULL; \
	functionOverload = NULL; \
	if (item) { \
		jnc_ModuleItemKind itemKind = jnc_ModuleItem_getItemKind(item); \
		if (itemKind == jnc_ModuleItemKind_Function) { \
			functionOverload = NULL; \
			function = (jnc_Function*)item; \
			JNC_MAP_FUNCTION_IMPL(function, p); \
		} else if (itemKind == jnc_ModuleItemKind_FunctionOverload) { \
			functionOverload = (jnc_FunctionOverload*)item; \
			function = jnc_FunctionOverload_getOverload(functionOverload, 0); \
			JNC_MAP_FUNCTION_IMPL(function, p); \
			overloadIdx = 1; \
		} \
	}

#define JNC_MAP_OVERLOAD(p) \
	if (functionOverload) { \
		function = jnc_FunctionOverload_getOverload(functionOverload, overloadIdx++); \
		JNC_MAP_FUNCTION_IMPL(function, p); \
	}

#define JNC_MAP_CONSTRUCTOR(p) \
	overloadableFunction = jnc_DerivableType_getConstructor(type); \
	JNC_MAP_OVERLOADABLE_FUNCTION(overloadableFunction.m_item, p); \

#define JNC_MAP_DESTRUCTOR(p) \
	function = jnc_DerivableType_getDestructor(type); \
	JNC_MAP_FUNCTION_IMPL(function, p);

#define JNC_MAP_UNARY_OPERATOR(opKind, p) \
	overloadableFunction = jnc_DerivableType_getUnaryOperator(type, opKind); \
	JNC_MAP_OVERLOADABLE_FUNCTION(overloadableFunction.m_item, p); \

#define JNC_MAP_BINARY_OPERATOR(opKind, p) \
	overloadableFunction = jnc_DerivableType_getBinaryOperator(type, opKind); \
	JNC_MAP_OVERLOADABLE_FUNCTION(overloadableFunction.m_item, p); \

#define JNC_MAP_CALL_OPERATOR(p) \
	overloadableFunction = jnc_DerivableType_getCallOperator(); \
	JNC_MAP_OVERLOADABLE_FUNCTION(overloadableFunction.m_item, p); \

#define JNC_MAP_CAST_OPERATOR(i, p) \
	function = jnc_DerivableType_getCastOperator(i); \
	JNC_MAP_FUNCTION_IMPL(function, p);

#define JNC_FIND_ITEM_IMPL(name, findMethod, itemKind, ItemType, item) \
	findResult = jnc_Namespace_##findMethod(nspace, name); \
	item = findResult.m_item && jnc_ModuleItem_getItemKind(findResult.m_item) == itemKind ? \
		(ItemType*)findResult.m_item : NULL; \

#define JNC_MAP_FUNCTION(name, p) \
	findResult = jnc_Namespace_findDirectChildItem(nspace, name); \
	JNC_MAP_OVERLOADABLE_FUNCTION(findResult.m_item, p);

#define JNC_MAP_FUNCTION_Q(name, p) \
	findResult = jnc_Namespace_findItemNoParse(nspace, name); \
	JNC_MAP_OVERLOADABLE_FUNCTION(findResult.m_item, p);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_MAP_PROPERTY_GETTER(prop, p) \
	do { \
		function = jnc_Property_getGetter(prop); \
		JNC_ASSERT(function); \
		JNC_MAP_FUNCTION_IMPL(function, p); \
	} while (0)

#define JNC_MAP_PROPERTY_SETTER(prop, p) \
	do { \
		overloadableFunction = jnc_Property_getSetter(prop); \
		JNC_MAP_OVERLOADABLE_FUNCTION(overloadableFunction.m_item, p); \
	} while (0)

#define JNC_MAP_PROPERTY_IMPL(name, findMethod, getter, setter) \
	JNC_FIND_ITEM_IMPL(name, findMethod, jnc_ModuleItemKind_Property, jnc_Property, prop); \
	if (prop) { \
		JNC_MAP_PROPERTY_GETTER(prop, getter); \
		JNC_MAP_PROPERTY_SETTER(prop, setter); \
	}

#define JNC_MAP_PROPERTY(name, getter, setter) \
	JNC_MAP_PROPERTY_IMPL(name, findDirectChildItem, getter, setter);

#define JNC_MAP_PROPERTY_Q(name, getter, setter) \
	JNC_MAP_PROPERTY_IMPL(name, findItemNoParse, getter, setter);

#define JNC_MAP_CONST_PROPERTY_IMPL(name, findMethod, getter) \
	JNC_FIND_ITEM_IMPL(name, findMethod, jnc_ModuleItemKind_Property, jnc_Property, prop); \
	if (prop) \
		JNC_MAP_PROPERTY_GETTER(prop, getter);

#define JNC_MAP_CONST_PROPERTY(name, getter) \
	JNC_MAP_CONST_PROPERTY_IMPL(name, findDirectChildItem, getter)

#define JNC_MAP_CONST_PROPERTY_Q(name, getter) \
	JNC_MAP_CONST_PROPERTY_IMPL(name, findItemNoParse, getter)

#define JNC_MAP_AUTOGET_PROPERTY_IMPL(name, findMethod, setter) \
	JNC_FIND_ITEM_IMPL(name, findMethod, jnc_ModuleItemKind_Property, jnc_Property, prop); \
	if (prop) \
		JNC_MAP_PROPERTY_SETTER(prop, setter);

#define JNC_MAP_AUTOGET_PROPERTY(name, setter) \
	JNC_MAP_AUTOGET_PROPERTY_IMPL(name, findDirectChildItem, setter)

#define JNC_MAP_AUTOGET_PROPERTY_Q(name, setter) \
	JNC_MAP_AUTOGET_PROPERTY_IMPL(name, findItemNoParse, setter)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_MAP_VARIABLE_IMPL(name, findMethod, p) \
	JNC_FIND_ITEM_IMPL(name, findMethod, jnc_ModuleItemKind_Variable, jnc_Variable, variable); \
	if (variable) { \
		result = jnc_Module_mapVariable(module, variable, p); \
		if (!result) \
			return 0; \
	}

#define JNC_MAP_VARIABLE(name, p) \
	JNC_MAP_VARIABLE_IMPL(name, findDirectChildItem, p)

#define JNC_MAP_VARIABLE_Q(name, p) \
	JNC_MAP_VARIABLE_IMPL(name, findItemNoParse, p)

//..............................................................................

// standard libraries

JNC_DECLARE_LIB(jnc_CoreLib)
JNC_DECLARE_LIB(jnc_IntrospectionLib)
JNC_DECLARE_LIB(jnc_StdLib)
JNC_DECLARE_LIB(jnc_SysLib)

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// overriding stdin/stdout/stderr of jancy scripts

typedef
size_t
jnc_StdLib_StdInputFunc(
	void* p,
	size_t size
);

typedef
size_t
jnc_StdLib_StdOutputFunc(
	const void* p,
	size_t size
);

// NULL pointer means default (stdin/stdout/stderr)

JNC_EXTERN_C
void
jnc_StdLib_setStdIo(
	jnc_StdLib_StdInputFunc* getsFunc,
	jnc_StdLib_StdOutputFunc* printOutFunc,
	jnc_StdLib_StdOutputFunc* printErrFunc
);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifndef __cplusplus

#	define jnc_pvoid_cast(x) (x)

#else // __cplusplus

//..............................................................................

// pvoid_cast is used for casting member function pointers to void*

template <typename T>
void*
jnc_pvoid_cast(T x) {
	JNC_ASSERT(sizeof(x) == sizeof(void*) || sizeof(x) == sizeof(void*)* 2);
	return *(void**)&x;
}

#	if (!_JNC_CPP_GCC)

// this overload is to make sure it's ok to pvoid_cast (NULL) on 64-bit systems
// gcc takes care of it automatically (it will not attempt to use 'int' for NULL)

inline
void*
jnc_pvoid_cast(int x) {
	return (void*)(intptr_t)x;
}

#	endif // AXL_CPP_GCC

namespace jnc {

//..............................................................................

template <typename T>
struct TailPaddingCheck: T {
	char m_field; // this field might be allocated in T's tail-padding
};

#define JNC_HAS_REUSABLE_TAIL_PADDING(T) (sizeof(jnc::TailPaddingCheck<T>) == sizeof(T))
#define JNC_ASSERT_NO_REUSABLE_TAIL_PADDING(T) JNC_ASSERT(!JNC_HAS_REUSABLE_TAIL_PADDING(T))

//..............................................................................

typedef jnc_ExtensionLib_AddSourcesFunc ExtensionLib_AddSourcesFunc;
typedef jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc ExtensionLib_AddOpaqueClassTypeInfosFunc;
typedef jnc_ExtensionLib_MapAddressesFunc ExtensionLib_MapAddressesFunc;
typedef jnc_ExtensionLib_UpdateCapabilitiesFunc ExtensionLib_UpdateCapabilitiesFunc;

typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_DynamicExtensionLibHost DynamicExtensionLibHost;
typedef jnc_DynamicExtensionLibMainFunc DynamicExtensionLibMainFunc;
typedef jnc_DynamicExtensionLibUnloadFunc DynamicExtensionLibUnloadFunc;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef jnc_StdLib_StdInputFunc StdLib_StdInputFunc;
typedef jnc_StdLib_StdOutputFunc StdLib_StdOutputFunc;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ExtensionLib*
StdLib_getLib() {
	return jnc_StdLib_getLib();
}

inline
ExtensionLib*
SysLib_getLib() {
	return jnc_SysLib_getLib();
}

inline
void
StdLib_setStdIo(
	StdLib_StdInputFunc* getsFunc,
	StdLib_StdOutputFunc* printOutFunc,
	StdLib_StdOutputFunc* printErrFunc
) {
	jnc_StdLib_setStdIo(getsFunc, printOutFunc, printErrFunc);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
