// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Module.h"

namespace jnc {

//.............................................................................

#define JNC_API_BEGIN_CLASS(name, slot) \
jnc::IfaceHdr* \
getRootIfaceHdr () \
{ \
	return (jnc::IfaceHdr*) (char*) this; \
} \
static \
size_t \
getApiClassSlot () \
{ \
	return slot; \
} \
static \
const char* \
getApiClassName () \
{ \
	return name; \
} \
static \
jnc::ClassType* \
getApiClassType () \
{ \
	jnc::Module* module = jnc::getCurrentThreadModule (); \
	return module->getApiClassType (getApiClassSlot (), getApiClassName ()); \
} \
static \
bool \
mapFunctions (jnc::Module* module) \
{ \
	bool result = true; \
	jnc::Function* function = NULL; \
	jnc::Property* prop = NULL; \
	jnc::ClassType* type = module->getApiClassType (getApiClassSlot (), getApiClassName ()); \
	if (!type) \
		return false; \
	jnc::Namespace* nspace = type;

#define JNC_API_END_CLASS() \
	return true; \
} \
static \
void* \
getApiClassVTable () \
{ \
	return NULL; \
}

//.............................................................................

#define JNC_API_END_CLASS_BEGIN_VTABLE() \
	return true; \
} \
static \
void* \
getApiClassVTable () \
{ \
	static void* VTable [] = \
	{

#define JNC_API_VTABLE_FUNCTION(function) \
	pvoid_cast (function),

#define JNC_API_END_VTABLE() \
	}; \
	return VTable; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_API_BEGIN_LIB() \
static \
bool \
mapFunctions (jnc::Module* module) \
{ \
	bool result = true; \
	jnc::Function* function = NULL; \
	jnc::Property* prop = NULL; \
	jnc::Namespace* nspace = module->m_namespaceMgr.getGlobalNamespace ();

#define JNC_API_END_LIB() \
	return true; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_API_LIB(Lib) \
	result = Lib::mapFunctions (module); \
	if (!result) \
		return false;

#define JNC_API_CLASS(Class) \
	result = Class::mapFunctions (module); \
	if (!result) \
		return false;

#define JNC_API_OPERATOR_NEW(proc) \
	function = type->getOperatorNew (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no operator new", type->getTypeString ().cc ()); \
		return false; \
	} \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_CONSTRUCTOR(proc) \
	function = type->getConstructor (); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ()); \
		return false; \
	} \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_CONSTRUCTOR_OVERLOAD(overloadIdx, proc) \
	function = type->getConstructor (overloadIdx); \
	if (!function) \
	{ \
		err::setFormatStringError ("'%s' has no constructor overload #%d", type->getTypeString ().cc (), overloadIdx); \
		return false; \
	} \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_FUNCTION(name, proc) \
	function = nspace->getFunctionByName (name); \
	if (!function) \
		return false; \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_FUNCTION_OVERLOAD(name, overloadIdx, proc) \
	function = nspace->getFunctionByName (name, overloadIdx); \
	if (!function) \
		return false; \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_STD_FUNCTION_FORCED(stdFunc, proc) \
	function = module->m_functionMgr.getStdFunction (stdFunc); \
	ASSERT (function); \
	module->mapFunction (function->getLlvmFunction (), pvoid_cast (proc));

#define JNC_API_STD_FUNCTION(stdFunc, proc) \
	if (module->m_functionMgr.isStdFunctionUsed (stdFunc)) \
	{ \
		JNC_API_STD_FUNCTION_FORCED (stdFunc, proc); \
	}

#define JNC_API_PROPERTY(name, getterProc, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	module->mapFunction (prop->getGetter ()->getLlvmFunction (), pvoid_cast (getterProc)); \
	module->mapFunction (prop->getSetter ()->getLlvmFunction (), pvoid_cast (setterProc));

#define JNC_API_CONST_PROPERTY(name, getterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	module->mapFunction (prop->getGetter ()->getLlvmFunction (), pvoid_cast (getterProc));

#define JNC_API_AUTOGET_PROPERTY(name, setterProc) \
	prop = nspace->getPropertyByName (name); \
	if (!prop) \
		return false; \
	module->mapFunction (prop->getSetter ()->getLlvmFunction (), pvoid_cast (setterProc));

//.............................................................................

void
prime (
	ClassType* type,
	void* pVTable,
	ObjHdr* object,
	size_t scopeLevel,
	ObjHdr* root,
	uintptr_t flags
	);

template <typename T>
class ApiObjBox:
	public ObjHdr,
	public T
{
public:
	void
	prime (
		size_t scopeLevel,
		jnc::ObjHdr* root,
		uintptr_t flags = 0
		)
	{
		jnc::prime (T::getApiClassType (), T::getApiClassVTable (), this, scopeLevel, root, flags);
	}

	void
	prime (jnc::ObjHdr* root)
	{
		prime (root->m_scopeLevel, root, root->m_flags);
	}

	void
	prime () // most common primer with scope level 0
	{
		prime (0, this, 0);
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
		char m_field; // this filed might be allocated in T's tail-padding
	};

	char m_tailPadding [sizeof (T) - offsetof (TailPaddingCheck, m_field)];
};

#endif

//.............................................................................

} // namespace jnc
