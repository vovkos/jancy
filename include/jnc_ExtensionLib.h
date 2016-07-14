// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"

typedef struct jnc_StringSlice jnc_StringSlice;
typedef struct jnc_ExtensionLib jnc_ExtensionLib;
typedef struct jnc_ExtensionLibFuncTable jnc_ExtensionLibFuncTable;
typedef struct jnc_DynamicExtensionLibHost jnc_DynamicExtensionLibHost;

//.............................................................................

typedef
int
jnc_ExtensionLib_ForcedExportFunc (
	jnc_ExtensionLib* self,	
	jnc_Module* module
	);

typedef
int
jnc_ExtensionLib_MapFunctionsFunc (
	jnc_ExtensionLib* self,	
	jnc_Module* module
	);

typedef
const jnc_OpaqueClassTypeInfo*
jnc_ExtensionLib_FindOpaqueClassTypeInfo (
	jnc_ExtensionLib* self,	
	const char* qualifiedName
	);

typedef
jnc_StringSlice
jnc_ExtensionLib_FindSourceFileContentsFunc (
	jnc_ExtensionLib* self,	
	const char* fileName
	);

//.............................................................................

struct jnc_ExtensionLibFuncTable
{
	jnc_ExtensionLib_ForcedExportFunc* m_forcedExportFunc;
	jnc_ExtensionLib_MapFunctionsFunc* m_mapFunctionsFunc;
	jnc_ExtensionLib_FindOpaqueClassTypeInfo* m_findOpaqueClassTypeInfoFunc;
	jnc_ExtensionLib_FindSourceFileContentsFunc* m_findSourceFileContentsFunc;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ExtensionLib
{
	jnc_ExtensionLibFuncTable* m_funcTable;

#ifdef __cplusplus
	bool
	forcedExport (jnc_Module* module)
	{	
		return m_funcTable->m_forcedExportFunc (this, module) != 0;
	}

	bool
	mapFunctions (jnc_Module* module)
	{
		return m_funcTable->m_mapFunctionsFunc (this, module) != 0;
	}

	const jnc_OpaqueClassTypeInfo*
	findOpaqueClassTypeInfo (const char* qualifiedName)
	{
		return m_funcTable->m_findOpaqueClassTypeInfoFunc (this, qualifiedName);
	}

	jnc_StringSlice
	findSourceFileContents (const char* fileName)
	{
		return m_funcTable->m_findSourceFileContentsFunc (this,	fileName);
	}
#endif // __cplusplus
};

//.............................................................................

typedef 
jnc_ExtensionLib*
jnc_DynamicExtensionLibMainFunc (jnc_DynamicExtensionLibHost* host);

AXL_SELECT_ANY
char
jnc_g_dynamicExtensionLibMainFuncName [] = "jncExtensionLibMain";

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
extern jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;
#else if (defined _JNC_CORE)
extern jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl;
#endif

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_StringSlice StringSlice;
typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_ExtensionLibFuncTable ExtensionLibFuncTable;
typedef jnc_DynamicExtensionLibHost DynamicExtensionLibHost;
typedef jnc_DynamicExtensionLibMainFunc DynamicExtensionLibMainFunc;

typedef jnc_ExtensionLib_ForcedExportFunc ExtensionLib_ForcedExportFunc;
typedef jnc_ExtensionLib_MapFunctionsFunc ExtensionLib_MapFunctionsFunc;
typedef jnc_ExtensionLib_FindOpaqueClassTypeInfo ExtensionLib_FindOpaqueClassTypeInfo;
typedef jnc_ExtensionLib_FindSourceFileContentsFunc ExtensionLib_FindSourceFileContentsFunc;

//.............................................................................

} // namespace jnc

#endif __cplusplus

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define JNC_DECLARE_TYPE_EX(JncType, TypePrefix) \
	JncType* \
	TypePrefix##_getType (jnc_Module* module); \
	int \
	TypePrefix##_mapFunctions ( \
		jnc_Module* module, \
		int isRequired \
		);

#define JNC_DECLARE_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX(jnc_DerivableType, TypePrefix)

#define JNC_DECLARE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX(jnc_ClassType, TypePrefix) \
	void* \
	TypePrefix##_getVTable (jnc_Module* module); \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo ();

#define JNC_DECLARE_OPAQUE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_CLASS_TYPE(TypePrefix) \

#define JNC_BEGIN_TYPE_MAP_EX(JncType, verify, TypePrefix, name, libGuid, cacheSlot) \
	JncType* \
	TypePrefix##_getType (jnc_Module* module) \
	{ \
		jnc_ModuleItem* item = jnc_Module_findItem (module, name, libGuid, cacheSlot); \
		return item ? verify (item, name) : NULL; \
	} \
	int \
	TypePrefix##_mapFunctions ( \
		jnc_Module* module, \
		int isRequired \
		) \
	{ \
		int result = 1; \
		jnc_Function* function = NULL; \
		jnc_Property* prop = NULL; \
		jnc_Namespace* nspace = NULL; \
		size_t overloadIdx = 0; \
		JncType* type = getType (module); \
		if (!type) \
			return !isRequired; \
		nspace = jnc_DerivableType_getNamespace ((jnc_DerivableType*) type);

#define JNC_BEGIN_TYPE_MAP(TypePrefix, name, libGuid, cacheSlot) \
	JNC_BEGIN_TYPE_MAP_EX ( \
		jnc_DerivableType, \
		jnc_verifyModuleItemIsDerivableType, \
		TypePrefix, \
		name, \
		libGuid, \
		cacheSlot \
		)

#define JNC_END_TYPE_MAP(TypePrefix) \
		return 1; \
	}

//.............................................................................

#define JNC_BEGIN_CLASS_TYPE_MAP(TypePrefix, name, libGuid, cacheSlot) \
	JNC_BEGIN_TYPE_MAP_EX ( \
		jnc_ClassType,  \
		jnc_verifyModuleItemIsClassType, \
		TypePrefix, \
		name, \
		libGuid, \
		cacheSlot \
		)

#define JNC_END_CLASS_TYPE_MAP(TypePrefix) \
	JNC_END_TYPE_MAP () \
	void* \
	TypePrefix##_getVTable () \
	{ \
		return NULL; \
	}

#define JNC_BEGIN_CLASS_TYPE_VTABLE(TypePrefix) \
	JNC_END_TYPE_MAP () \
	void* \
	TypePrefix##_getVTable () \
	{ \
		static void* vtable [] = \
		{

#define JNC_CLASS_TYPE_VTABLE_ENTRY(function) \
			pvoid_cast (function),

#define JNC_END_CLASS_TYPE_VTABLE() \
		}; \
		return vtable; \
	}

#define JNC_OPAQUE_CLASS_TYPE_INFO_EX(TypePrefix, Type, markOpaqueGcRootsFunc, isNonCreatable) \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo () \
	{ \
		static jnc_OpaqueClassTypeInfo typeInfo = \
		{ \
			sizeof (Type), \
			(jnc_MarkOpaqueGcRootsFunc*) pvoid_cast (markOpaqueGcRootsFunc), \
			isNonCreatable, \
		}; \
		return &typeInfo; \
	}

#define JNC_OPAQUE_CLASS_TYPE_INFO(TypePrefix, markOpaqueGcRootsFunc) \
	JNC_OPAQUE_CLASS_TYPE_INFO_EX (TypePrefix, markOpaqueGcRootsFunc, 0)

#define JNC_OPAQUE_CLASS_TYPE_INFO_NC(TypePrefix, markOpaqueGcRootsFunc) \
	JNC_OPAQUE_CLASS_TYPE_INFO_EX (TypePrefix, markOpaqueGcRootsFunc, 1)

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_MAP(LibPrefix) \
	int \
	LibPrefix##_mapFunctions (jnc_Module* module) \
	{ \
		int result = 1; \
		jnc_Function* function = NULL; \
		jnc_Property* prop = NULL; \
		jnc_Namespace* nspace = jnc_Module_getGlobalNamespace (module); \
		size_t overloadIdx = 0; \

	#define JNC_END_LIB_MAP() \
		return 1; \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_MAP_TYPE_EX(TypePrefix, isRequired) \
	result = TypePrefix_##mapFunctions (module, isRequired); \
	if (!result) \
		return 0;

#define JNC_MAP_TYPE(TypePrefix) \
	JNC_MAP_TYPE_EX(TypePrefix, 0)

#define JNC_MAP_TYPE_REQ(TypePrefix) \
	JNC_MAP_TYPE_EX(TypePrefix, 1)

#define JNC_MAP_HELPER(function) \
	result = function (module); \
	if (!result) \
		return 0;

#define JNC_MAP(function, p) \
	jnc_Module_mapFunction (module, function, pvoid_cast (p));

#define JNC_MAP_OVERLOAD(p) \
	if (function) \
	{ \
		jnc_Function* overload = jnc_Function_getOverload (function, ++overloadIdx); \
		if (!overload) \
			return 0; \
		JNC_MAP (overload, p) \
	}

#define JNC_MAP_PRECONSTRUCTOR(p) \
	function = jnc::ext::getTypePreconstructor (type); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CONSTRUCTOR(p) \
	function = jnc::ext::getTypeConstructor (type); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_DESTRUCTOR(p) \
	function = jnc::ext::getClassTypeDestructor (type); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_UNARY_OPERATOR(opKind, p) \
	function = jnc::ext::getTypeUnaryOperator (type, opKind); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_BINARY_OPERATOR(opKind, p) \
	function = jnc::ext::getTypeBinaryOperator (type, opKind); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CALL_OPERATOR(p) \
	function = jnc::ext::getClassType type->getCallOperator (opKind); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CAST_OPERATOR(i, p) \
	function = jnc::ext::getClassType type->getCastOperator (i); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_FUNCTION(name, p) \
	function = jnc::ext::findNamespaceFunction (nspace, name); \
	if (function) \
	{ \
		overloadIdx = 0; \
		JNC_MAP (function, p) \
	}

#define JNC_MAP_PROPERTY_SETTER(prop, p) \
	function = jnc::ext::getPropertySetter (prop); \
	if (!function) \
		return false; \
	overloadIdx = 0; \
	JNC_MAP (function, p);

#define JNC_MAP_PROPERTY_GETTER(prop, p) \
	function = jnc::ext::getPropertyGetter (prop); \
	ASSERT (function); \
	overloadIdx = 0; \
	JNC_MAP (function, p);

#define JNC_MAP_PROPERTY(name, getter, setter) \
	prop = jnc::ext::findNamespaceProperty (nspace, name); \
	if (!prop) \
		return false; \
	JNC_MAP_PROPERTY_GETTER (prop, getter); \
	JNC_MAP_PROPERTY_SETTER (prop, setter);

#define JNC_MAP_CONST_PROPERTY(name, getter) \
	prop = jnc::ext::findNamespaceProperty (nspace, name); \
	if (!prop) \
		return false; \
	JNC_MAP_PROPERTY_GETTER (prop, getter);

#define JNC_MAP_AUTOGET_PROPERTY(name, setter) \
	prop = jnc::ext::findNamespaceProperty (nspace, name); \
	if (!prop) \
		return false; \
	JNC_MAP_PROPERTY_SETTER (prop, setter);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE_EX(Map) \
virtual \
const jnc::OpaqueClassTypeInfo* \
findOpaqueClassTypeInfo (const char* fileName) \
{ \
	Map* map = axl::sl::getSingleton <Map> (); \
	axl::sl::StringHashTableMapIterator <const jnc::OpaqueClassTypeInfo*> it = map->find (fileName); \
	return it ? it->m_value : NULL; \
} \
class Map: public axl::sl::StringHashTableMap <const jnc::OpaqueClassTypeInfo*> \
{ \
public: \
	Map () \
	{ 

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE_EX (OpaqueClassTypeInfoMap)

#define JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Type) \
		visit (Type::getTypeName ())->m_value = Type::getOpaqueClassTypeInfo ();

#define JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	} \
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE_EX(Table) \
virtual \
axl::sl::StringRef \
findSourceFileContents (const char* fileName) \
{ \
	Table* table = axl::sl::getSingleton <Table> (); \
	return table->find (fileName); \
} \
class Table \
{ \
protected: \
	axl::sl::StringHashTableMap <axl::sl::StringRef> m_fileNameMap; \
public: \
	axl::sl::StringRef \
	find (const char* fileName) \
	{ \
		axl::sl::StringHashTableMapIterator <axl::sl::StringRef> it = m_fileNameMap.find (fileName); \
		return it ? it->m_value : axl::sl::StringRef (); \
	} \
	Table () \
	{

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE() \
	JNC_BEGIN_LIB_SOURCE_FILE_TABLE_EX (SourceFileTable)

#define JNC_LIB_SOURCE_FILE(fileName, sourceVar) \
		m_fileNameMap [fileName] = axl::sl::StringRef (sourceVar, lengthof (sourceVar)); \

#define JNC_END_LIB_SOURCE_FILE_TABLE() \
	} \
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_FORCED_EXPORT() \
virtual \
bool \
forcedExport (jnc::ext::Module* module) \
{ \
	bool result = true;

#define JNC_LIB_FORCED_IMPORT(fileName) \
	result = jnc::ext::addModuleImport (module, fileName); \
	if (!result) \
		return false;

#define JNC_LIB_FORCED_SOURCE_FILE(fileName, sourceVar) \
	jnc::ext::addModuleSource (module, fileName, sourceVar, lengthof (sourceVar)); \

#define JNC_END_LIB_FORCED_EXPORT() \
	return true; \
}

//.............................................................................

// implicit tail-padding (might lead to ABI-incompatibility if omitted)

#if (_AXL_CPP == AXL_CPP_MSC)

template <typename T>
class BaseTailPadding
{
	// microsoft compiler does not re-use tail padding
};

#else 

template <typename T>
class BaseTailPadding
{
private:
	struct TailPaddingCheck: T
	{
		char m_field; // this field might be allocated in T's tail-padding
	};

	char m_tailPadding [sizeof (T) - offsetof (TailPaddingCheck, m_field)];
};

#endif

//.............................................................................
