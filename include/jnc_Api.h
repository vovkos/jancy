// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Module.h"
#include "jnc_StdLibApiSlots.h"

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
	return true; \
}

//.............................................................................

#define JNC_BEGIN_CLASS_TYPE(name, slot) \
	JNC_BEGIN_TYPE_EX (jnc::ClassType, getApiClassType, name, slot) \

#define JNC_END_CLASS_TYPE() \
JNC_END_TYPE () \
static \
void* \
getApiVTable () \
{ \
	return NULL; \
}

#define JNC_END_CLASS_TYPE_BEGIN_VTABLE() \
JNC_END_TYPE () \
static \
void* \
getApiVTable () \
{ \
	static void* vtable [] = \
	{

#define JNC_VTABLE_FUNCTION(function) \
	pvoid_cast (function),

#define JNC_END_VTABLE() \
	}; \
	return vtable; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_OPAQUE_CLASS_TYPE_EX(Type, name, slot, isNonCreatable) \
static \
const jnc::OpaqueClassTypeInfo* \
getOpaqueClassTypeInfo () \
{ \
	static jnc::OpaqueClassTypeInfo typeInfo = \
	{ \
		sizeof (Type), \
		isNonCreatable, \
	}; \
	return &typeInfo; \
} \
JNC_BEGIN_CLASS_TYPE (name, slot)

#define JNC_BEGIN_OPAQUE_CLASS_TYPE(Type, name, slot) \
	JNC_BEGIN_OPAQUE_CLASS_TYPE_EX (Type, name, slot, false)

#define JNC_BEGIN_OPAQUE_CLASS_TYPE_NC(Type, name, slot) \
	JNC_BEGIN_OPAQUE_CLASS_TYPE_EX (Type, name, slot, true)

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

#define JNC_BEGIN_OPAQUE_CLASS_TYPE_DB() \
class ThisOpaqueClassTypeDb: public jnc::StdOpaqueClassTypeDb \
{ \
public: \
	ThisOpaqueClassTypeDb () \
	{

#define JNC_END_OPAQUE_CLASS_TYPE_DB() \
	} \
}; \
static \
jnc::OpaqueClassTypeDb* \
getOpaqueClassTypeDb () \
{ \
	return rtl::getSingleton <ThisOpaqueClassTypeDb> (); \
}

#define JNC_OPAQUE_CLASS_TYPE(Type) \
	setOpaqueClassTypeInfo (Type::getApiTypeName (), Type::getOpaqueClassTypeInfo ());

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

#define JNC_MARK_OPAQUE_GC_ROOTS_FUNC(proc) \
	result = type->setMarkOpaqueGcRootsFunc ((jnc::Class_MarkOpaqueGcRootsFunc*) proc); \
	if (!result) \
		return false;

#define JNC_PRECONSTRUCTOR(proc) \
	function = type->getPreConstructor (); \
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
	Box* box,
	Box* root,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	);

inline
void
prime (
	Box* box,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	prime (box, box, type, vtable);
}

template <typename T>
void
prime (
	Module* module,
	ClassBox <T>* p,
	Box* root
	)
{
	prime (p, root, T::getApiType (module), T::getApiVTable ());
}

template <typename T>
void
prime (
	Module* module,
	ClassBox <T>* p
	)
{
	prime (p, p, T::getApiType (module), T::getApiVTable ());
}

//.............................................................................

// implicit tail-padding (might lead to ABI-incompatibility if omitted)

#if (_AXL_CPP == AXL_CPP_MSC)

template <typename T>
class ApiBaseTailPadding
{
	// microsoft compiler does not re-use tail padding
};

#else 

template <typename T>
class ApiBaseTailPadding
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
