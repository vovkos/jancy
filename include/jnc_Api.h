// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Module.h"

namespace jnc {

//.............................................................................

#define JNC_BEGIN_TYPE_EX(Type, moduleGetApiType, name, slot) \
static \
Type* \
getApiType (jnc::Module* module) \
{ \
	return module->moduleGetApiType (slot, name); \
} \
static \
const char* \
getApiTypeName () \
{ \
	return name; \
} \
static \
size_t \
getApiTypeSlot () \
{ \
	return slot; \
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
	Type* type = module->moduleGetApiType (slot, name); \
	if (!type) \
		return isRequired ? false : true; \
	jnc::Namespace* nspace = type;

#define JNC_BEGIN_TYPE(name, slot) \
	JNC_BEGIN_TYPE_EX(jnc::DerivableType, getApiDerivableType, name, slot)

#define JNC_END_TYPE() \
	if (isOpaqueClassType (type) && !((jnc::ClassType*) type)->getGcRootEnumProc ()) \
	{ \
		err::setFormatStringError ("JNC_OPAQUE_CLASS is missing for '%s'", type->getTypeString ().cc ()); \
		return false; \
	} \
	return true; \
}

//.............................................................................

#define JNC_BEGIN_CLASS(name, slot) \
jnc::IfaceHdr* \
getRootIfaceHdr () \
{ \
	return (jnc::IfaceHdr*) (char*) this; \
} \
JNC_BEGIN_TYPE_EX (jnc::ClassType, getApiClassType, name, slot) \

#define JNC_END_CLASS() \
JNC_END_TYPE () \
static \
void* \
getApiClassVTable () \
{ \
	return NULL; \
}

#define JNC_END_CLASS_BEGIN_VTABLE() \
JNC_END_TYPE () \
static \
void* \
getApiClassVTable () \
{ \
	static void* VTable [] = \
	{

#define JNC_VTABLE_FUNCTION(function) \
	pvoid_cast (function),

#define JNC_END_VTABLE() \
	}; \
	return VTable; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB() \
static \
bool \
mapFunctions (jnc::Module* module) \
{ \
	bool result = true; \
	jnc::Function* function = NULL; \
	size_t overloadIdx = 0; \
	jnc::Property* prop = NULL; \
	jnc::Namespace* nspace = module->m_namespaceMgr.getGlobalNamespace (); \

#define JNC_END_LIB() \
	return true; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_LIB(Lib) \
	result = Lib::mapFunctions (module); \
	if (!result) \
		return false;

#define JNC_TYPE_EX(Type, isRequired) \
	result = Type::mapFunctions (module, isRequired); \
	if (!result) \
		return false;

#define JNC_TYPE(Type) \
	JNC_TYPE_EX(Type, true)

#define JNC_OPT_TYPE(Type) \
	JNC_TYPE_EX(Type, false)

#define JNC_MAP(function, proc) \
	module->mapFunction (function, pvoid_cast (proc));

#define JNC_OVERLOAD(proc) \
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

#define JNC_OPAQUE_CLASS(cls, gcRootEnumProc) \
	type->setupOpaqueClass (sizeof (jnc::ObjHdr) + sizeof (cls), (jnc::ClassTypeGcRootEnumProc*) gcRootEnumProc);

#define JNC_OPERATOR_NEW(proc) \
	function = type->getOperatorNew (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator new", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc) \

#define JNC_PRECONSTRUCTOR(proc) \
	function = type->getPreconstructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no preconstructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_CONSTRUCTOR(proc) \
	function = type->getConstructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_DESTRUCTOR(proc) \
	function = type->getDestructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no destructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_UNARY_OPERATOR(opKind, proc) \
	function = type->getUnaryOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc::getUnOpKindString (opKind)); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_BINARY_OPERATOR(opKind, proc) \
	function = type->getBinaryOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc::getBinOpKindString (opKind)); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_CALL_OPERATOR(proc) \
	function = type->getCallOperator (opKind); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator ()", type->getTypeString ().cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_CAST_OPERATOR(i, proc) \
	function = type->getCastOperator (i); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no cast operator #%d", type->getTypeString ().cc (), i); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc)

#define JNC_FUNCTION_EX(name, proc, isRequired) \
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

#define JNC_FUNCTION(name, proc) \
	JNC_FUNCTION_EX(name, proc, true)

#define JNC_OPT_FUNCTION(name, proc) \
	JNC_FUNCTION_EX(name, proc, false)

#define JNC_STD_FUNCTION_FORCED(stdFuncKind, proc) \
	function = module->m_functionMgr.getStdFunction (stdFuncKind); \
	ASSERT (function); \
	JNC_MAP (function, proc);

#define JNC_STD_FUNCTION(stdFuncKind, proc) \
	if (module->m_functionMgr.isStdFunctionUsed (stdFuncKind)) \
	{ \
		JNC_STD_FUNCTION_FORCED (stdFuncKind, proc); \
	}

#define JNC_STD_TYPE(stdType, Type) \
	if (module->m_typeMgr.isStdTypeUsed (stdType)) \
	{ \
		JNC_TYPE (Type); \
	}

#define JNC_MAP_PROPERTY_SETTER(prop, proc) \
	function = prop->getSetter (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no setter", prop->m_tag.cc ()); \
		return false; \
	} \
	overloadIdx = 0; \
	JNC_MAP (function, proc);

#define JNC_PROPERTY(name, getterProc, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP (prop->getGetter (), getterProc); \
	JNC_MAP_PROPERTY_SETTER (prop, setterProc);

#define JNC_CONST_PROPERTY(name, getterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP (prop->getGetter (), getterProc);

#define JNC_AUTOGET_PROPERTY(name, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	JNC_MAP_PROPERTY_SETTER (prop, setterProc);

//.............................................................................

void
prime (
	ClassType* type,
	void* vtable,
	ObjHdr* object,
	size_t scopeLevel,
	ObjHdr* root,
	uintptr_t flags
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
class ApiObjBox:
	public ObjHdr,
	public T
{
public:
	void
	prime (
		ClassType* type,
		size_t scopeLevel,
		ObjHdr* root,
		uintptr_t flags = 0
		)
	{
		jnc::prime (type, T::getApiClassVTable (), this, scopeLevel, root, flags);
	}

	void
	prime (
		Module* module,
		size_t scopeLevel,
		ObjHdr* root,
		uintptr_t flags = 0
		)
	{
		jnc::prime (T::getApiType (module), T::getApiClassVTable (), this, scopeLevel, root, flags);
	}

	void
	prime (
		ClassType* type,
		ObjHdr* root
		)
	{
		prime (type, root->m_scopeLevel, root, root->m_flags);
	}

	void
	prime (
		Module* module,
		ObjHdr* root
		)
	{
		prime (T::getApiType (module), root->m_scopeLevel, root, root->m_flags);
	}

	void
	prime (ClassType* type) // most common primer with scope level 0
	{
		prime (type, 0, this, 0);
	}

	void
	prime (Module* module) // most common primer with scope level 0
	{
		prime (T::getApiType (module), 0, this, 0);
	}
};

//.............................................................................

// ensures no tail-padding (which might lead to ABI-incompatibility)

#if (_AXL_CPP == AXL_CPP_MSC)

template <typename T>
class ApiBase: public T
{
};

#else 

template <typename T>
class ApiBase: public T
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
