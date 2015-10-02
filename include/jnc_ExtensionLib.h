// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

class Module;
class ExtensionLibSlotDb;
struct OpaqueClassTypeInfo;

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
	rtl::StringSlice
	findSourceFile (const char* fileName)
	{
		return rtl::StringSlice ();
	}
};

//.............................................................................

typedef 
ExtensionLib*
GetExtensionLibFunc (ExtensionLibSlotDb* slotDb);

//.............................................................................

#define JNC_BEGIN_TYPE_MAP_EX(Type, verify, name, libSlot, typeSlot) \
static \
Type* \
getType (jnc::Module* module) \
{ \
	jnc::ModuleItem* item = module->m_extensionLibMgr.findItem (libSlot, typeSlot, name); \
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
getLibSlot () \
{ \
	return libSlot; \
} \
static \
size_t \
getTypeSlot () \
{ \
	return typeSlot; \
} \
static \
bool \
mapFunctions ( \
	jnc::Module* module, \
	bool isRequired \
	) \
{ \
	bool result = true; \
	jnc::Function* function = NULL; \
	size_t overloadIdx = 0; \
	jnc::Property* prop = NULL; \
	Type* type = getType (module); \
	if (!type) \
		return isRequired ? false : true; \
	jnc::Namespace* nspace = type;

#define JNC_BEGIN_TYPE_MAP(name, libId, slot) \
	JNC_BEGIN_TYPE_MAP_EX(jnc::DerivableType, verifyModuleItemIsDerivableType, name, libId, slot)

#define JNC_END_TYPE_MAP() \
	return true; \
}

//.............................................................................

#define JNC_BEGIN_CLASS_TYPE_MAP(name, libId, slot) \
	JNC_BEGIN_TYPE_MAP_EX (jnc::ClassType, verifyModuleItemIsClassType, name, libId, slot) \

#define JNC_END_CLASS_TYPE_MAP() \
	JNC_END_TYPE_MAP () \

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
const jnc::OpaqueClassTypeInfo* \
getOpaqueClassTypeInfo () \
{ \
	static jnc::OpaqueClassTypeInfo typeInfo = \
	{ \
		sizeof (Type), \
		(jnc::Class_MarkOpaqueGcRootsFunc*) pvoid_cast (markOpaqueGcRootsFunc), \
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
mapFunctions (jnc::Module* module) \
{ \
	bool result = true; \
	jnc::Function* function = NULL; \
	size_t overloadIdx = 0; \
	jnc::Property* prop = NULL; \
	jnc::Namespace* nspace = module->m_namespaceMgr.getGlobalNamespace (); \

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

#define JNC_MAP(function, proc) \
	module->mapFunction (function, pvoid_cast (proc));

#define JNC_MAP_OVERLOAD(proc) \
	if (function) \
	{ \
		jnc::Function* overload = function->getOverload (++overloadIdx); \
		if (!overload) \
		{ \
			err::setFormatStringError ("'%s' has no overload #%d", function->m_tag.cc (), overloadIdx); \
			return false; \
		} \
		JNC_MAP (overload, proc) \
	}

#define JNC_MAP_PRECONSTRUCTOR(proc) \
	function = type->getPreConstructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no preconstructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_CONSTRUCTOR(proc) \
	function = type->getConstructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_DESTRUCTOR(proc) \
	function = type->getDestructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no destructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_UNARY_OPERATOR(opKind, proc) \
	function = type->getUnaryOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc::getUnOpKindString (opKind)); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_BINARY_OPERATOR(opKind, proc) \
	function = type->getBinaryOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc::getBinOpKindString (opKind)); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_CALL_OPERATOR(proc) \
	function = type->getCallOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator ()", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_CAST_OPERATOR(i, proc) \
	function = type->getCastOperator (i); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no cast operator #%d", type->getTypeString ().cc (), i); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_MAP_FUNCTION_EX(name, proc, isRequired) \
	function = nspace->getFunctionByName (name); \
	if (function) \
	{ \
		overloadIdx = 0; \
		JNC_MAP (function, proc) \
	} \
	else if (isRequired) \
	{ \
		return false; \
	}

#define JNC_MAP_FUNCTION(name, proc) \
	JNC_MAP_FUNCTION_EX(name, proc, false)

#define JNC_MAP_FUNCTION_REQ(name, proc) \
	JNC_MAP_FUNCTION_EX(name, proc, true)

#define JNC_MAP_PROPERTY_SETTER(prop, proc) \
	function = prop->getSetter (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no setter", prop->m_tag.cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc);

#define JNC_MAP_PROPERTY(name, getterProc, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP (prop->getGetter (), getterProc); \
	JNC_MAP_PROPERTY_SETTER (prop, setterProc);

#define JNC_MAP_CONST_PROPERTY(name, getterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP (prop->getGetter (), getterProc);

#define JNC_MAP_AUTOGET_PROPERTY(name, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP_PROPERTY_SETTER (prop, setterProc);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE_EX(Map) \
virtual \
const jnc::OpaqueClassTypeInfo* \
findOpaqueClassTypeInfo (const char* fileName) \
{ \
	Map* map = axl::rtl::getSingleton <Map> (); \
	axl::rtl::StringHashTableMapIterator <const jnc::OpaqueClassTypeInfo*> it = map->find (fileName); \
	return it ? it->m_value : NULL; \
} \
class Map: public axl::rtl::StringHashTableMap <const jnc::OpaqueClassTypeInfo*> \
{ \
public: \
	Map () \
	{ 

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE_EX (OpaqueClassTypeInfoMap)

#define JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(qualifiedName, Type) \
		visit (qualifiedName)->m_value = Type::getOpaqueClassTypeInfo ();

#define JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	} \
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE_EX(Map) \
virtual \
axl::rtl::StringSlice \
findSourceFile (const char* fileName) \
{ \
	Map* map = axl::rtl::getSingleton <Map> (); \
	axl::rtl::StringHashTableMapIterator <axl::rtl::StringSlice> it = map->find (fileName); \
	return it ? it->m_value : axl::rtl::StringSlice (); \
} \
class Map: public axl::rtl::StringHashTableMap <axl::rtl::StringSlice> \
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
forcedExport (jnc::Module* module) \
{ \
	bool result = true;

#define JNC_LIB_FORCED_SOURCE_FILE(name, sourceVar) \
	module->m_importMgr.addSource (name, axl::rtl::StringSlice (sourceVar, lengthof (sourceVar))); \

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

} // namespace jnc
