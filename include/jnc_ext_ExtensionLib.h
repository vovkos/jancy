// This file is part of AXL (R) Library

// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ext_OpaqueClassTypeInfo.h"
#include "jnc_ext_ExtensionLibHost.h"

namespace jnc {
namespace ext {

//.............................................................................

class ExtensionLib
{
public:
	virtual 
	bool
	forcedExport (Module* module)
	{
		return true;
	}

	virtual 
	bool
	mapFunctions (Module* module)
	{
		return true;
	}

	virtual
	const OpaqueClassTypeInfo*
	findOpaqueClassTypeInfo (const char* qualifiedName)
	{
		return NULL;
	}

	virtual
	size_t
	getSourceFileCount ()
	{
		return 0;
	}

	virtual
	size_t
	getSourceFileNameTable (
		const char** fileNameTable,
		size_t count
		)
	{
		return 0;
	}

	virtual
	sl::StringSlice
	findSourceFile (const char* fileName)
	{
		return sl::StringSlice ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef 
ExtensionLib*
ExtensionLibMainFunc (ExtensionLibHost* host);

AXL_SELECT_ANY
char
g_extensionLibMainFuncName [] = "jncExtensionLibMain";

//.............................................................................

#define JNC_BEGIN_TYPE_MAP_EX(Type, verify, name, libCacheSlot, typeCacheSlot) \
static \
Type* \
getType (jnc::ext::Module* module) \
{ \
	jnc::ext::ModuleItem* item = jnc::ext::findModuleItem (module, name, libCacheSlot, typeCacheSlot); \
	return item ? verify (item, name) : NULL; \
} \
static \
Type* \
getType (jnc::ext::Runtime* runtime) \
{ \
	jnc::ext::ModuleItem* item = jnc::ext::findModuleItem (runtime, name, libCacheSlot, typeCacheSlot); \
	return item ? verify (item, name) : NULL; \
} \
static \
const char* \
getTypeName () \
{ \
	return name; \
} \
static \
size_t \
getLibCacheSlot () \
{ \
	return libCacheSlot; \
} \
static \
size_t \
getTypeCacheSlot () \
{ \
	return typeCacheSlot; \
} \
static \
bool \
mapFunctions ( \
	jnc::ext::Module* module, \
	bool isRequired \
	) \
{ \
	bool result = true; \
	jnc::ext::Function* function = NULL; \
	size_t overloadIdx = 0; \
	jnc::ext::Property* prop = NULL; \
	Type* type = getType (module); \
	if (!type) \
		return isRequired ? false : true; \
	jnc::ext::Namespace* nspace = jnc::ext::getTypeNamespace (type);

#define JNC_BEGIN_TYPE_MAP(name, libCacheSlot, typeCacheSlot) \
	JNC_BEGIN_TYPE_MAP_EX ( \
		jnc::ext::DerivableType, \
		jnc::ext::verifyModuleItemIsDerivableType, \
		name, \
		libCacheSlot, \
		typeCacheSlot \
		)

#define JNC_END_TYPE_MAP() \
	return true; \
}

//.............................................................................

#define JNC_BEGIN_CLASS_TYPE_MAP(name, libCacheSlot, typeCacheSlot) \
	JNC_BEGIN_TYPE_MAP_EX ( \
		jnc::ext::ClassType,  \
		jnc::ext::verifyModuleItemIsClassType, \
		name, \
		libCacheSlot, \
		typeCacheSlot \
		)

#define JNC_END_CLASS_TYPE_MAP() \
	JNC_END_TYPE_MAP () \
	static \
	void* \
	getVTable () \
	{ \
		return NULL; \
	}

#define JNC_BEGIN_CLASS_TYPE_VTABLE() \
JNC_END_TYPE_MAP () \
static \
void* \
getVTable () \
{ \
	static void* vtable [] = \
	{

#define JNC_CLASS_TYPE_VTABLE_ENTRY(function) \
		pvoid_cast (function),

#define JNC_END_CLASS_TYPE_VTABLE() \
	}; \
	return vtable; \
}

#define JNC_OPAQUE_CLASS_TYPE_INFO_EX(Type, markOpaqueGcRootsFunc, isNonCreatable) \
static \
const jnc::ext::OpaqueClassTypeInfo* \
getOpaqueClassTypeInfo () \
{ \
	static jnc::ext::OpaqueClassTypeInfo typeInfo = \
	{ \
		sizeof (Type), \
		(jnc::ext::MarkOpaqueGcRootsFunc*) pvoid_cast (markOpaqueGcRootsFunc), \
		isNonCreatable, \
	}; \
	return &typeInfo; \
} \

#define JNC_OPAQUE_CLASS_TYPE_INFO(Type, markOpaqueGcRootsFunc) \
	JNC_OPAQUE_CLASS_TYPE_INFO_EX (Type, markOpaqueGcRootsFunc, false)

#define JNC_OPAQUE_CLASS_TYPE_INFO_NC(Type, markOpaqueGcRootsFunc) \
	JNC_OPAQUE_CLASS_TYPE_INFO_EX (Type, markOpaqueGcRootsFunc, true)

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_MAP() \
virtual \
bool \
mapFunctions (jnc::ext::Module* module) \
{ \
	bool result = true; \
	jnc::ext::Function* function = NULL; \
	size_t overloadIdx = 0; \
	jnc::ext::Property* prop = NULL; \
	jnc::ext::Namespace* nspace = jnc::ext::getModuleGlobalNamespace (module);

#define JNC_END_LIB_MAP() \
	return true; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_MAP_TYPE_EX(Type, isRequired) \
	result = Type::mapFunctions (module, isRequired); \
	if (!result) \
		return false;

#define JNC_MAP_TYPE(Type) \
	JNC_MAP_TYPE_EX(Type, false)

#define JNC_MAP_TYPE_REQ(Type) \
	JNC_MAP_TYPE_EX(Type, true)

#define JNC_MAP_HELPER(function) \
	result = function (module); \
	if (!result) \
		return false;

#define JNC_MAP(function, p) \
	jnc::ext::mapFunction (module, function, pvoid_cast (p));

#define JNC_MAP_OVERLOAD(p) \
	if (function) \
	{ \
		jnc::ext::Function* overload = jnc::ext::getFunctionOverload (function, ++overloadIdx); \
		if (!overload) \
			return false; \
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
const jnc::ext::OpaqueClassTypeInfo* \
findOpaqueClassTypeInfo (const char* fileName) \
{ \
	Map* map = axl::mt::getSingleton <Map> (); \
	axl::sl::StringHashTableMapIterator <const jnc::ext::OpaqueClassTypeInfo*> it = map->find (fileName); \
	return it ? it->m_value : NULL; \
} \
class Map: public axl::sl::StringHashTableMap <const jnc::ext::OpaqueClassTypeInfo*> \
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

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE_EX(Map) \
virtual \
size_t \
getSourceFileCount () \
{ \
	Map* map = axl::mt::getSingleton <Map> (); \
	return map->getCount (); \
} \
virtual \
size_t \
getSourceFileNameTable ( \
	const char** fileNameTable, \
	size_t count \
	) \
{ \
	Map* map = axl::mt::getSingleton <Map> (); \
	size_t maxCount = map->getCount (); \
	if (count > maxCount) \
		count = maxCount; \
	axl::sl::StringHashTableMapIterator <axl::sl::StringSlice> it = map->getHead (); \
	for (size_t i = 0; i < count; i++, it++) \
		fileNameTable [i] = it->m_key; \
	return count; \
} \
virtual \
axl::sl::StringSlice \
findSourceFile (const char* fileName) \
{ \
	Map* map = axl::mt::getSingleton <Map> (); \
	axl::sl::StringHashTableMapIterator <axl::sl::StringSlice> it = map->find (fileName); \
	return it ? it->m_value : axl::sl::StringSlice (); \
} \
class Map: public axl::sl::StringHashTableMap <axl::sl::StringSlice> \
{ \
public: \
	Map () \
	{ 

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE() \
	JNC_BEGIN_LIB_SOURCE_FILE_TABLE_EX (SourceFileMap)

#define JNC_LIB_SOURCE_FILE_TABLE_ENTRY(fileName, sourceVar) \
		visit (fileName)->m_value.copy (sourceVar, lengthof (sourceVar));

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

#define JNC_LIB_FORCED_SOURCE_FILE(name, sourceVar) \
	jnc::ext::addModuleSource (module, name, sourceVar, lengthof (sourceVar)); \

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

} // namespace ext
} // namespace jnc
