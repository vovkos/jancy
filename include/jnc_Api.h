// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Module.h"

namespace jnc {

//.............................................................................

#define JNC_API_BEGIN_CLASS(Name, Slot) \
jnc::TIfaceHdr* \
GetRootIfaceHdr () \
{ \
	return (jnc::TIfaceHdr*) (char*) this; \
} \
static \
size_t \
GetApiClassSlot () \
{ \
	return Slot; \
} \
static \
const char* \
GetApiClassName () \
{ \
	return Name; \
} \
static \
jnc::CClassType* \
GetApiClassType () \
{ \
	jnc::CModule* pModule = jnc::GetCurrentThreadModule (); \
	return pModule->GetApiClassType (GetApiClassSlot (), GetApiClassName ()); \
} \
static \
bool \
Export (jnc::CModule* pModule) \
{ \
	bool Result = true; \
	jnc::CFunction* pFunction = NULL; \
	jnc::CProperty* pProperty = NULL; \
	jnc::CClassType* pType = pModule->GetApiClassType (GetApiClassSlot (), GetApiClassName ()); \
	if (!pType) \
		return false; \
	jnc::CNamespace* pNamespace = pType;

#define JNC_API_END_CLASS() \
	return true; \
} \
static \
void* \
GetApiClassVTable () \
{ \
	return NULL; \
}

//.............................................................................
	
#define JNC_API_END_CLASS_BEGIN_VTABLE() \
	return true; \
} \
static \
void* \
GetApiClassVTable () \
{ \
	static void* VTable [] = \
	{

#define JNC_API_VTABLE_FUNCTION(Function) \
	pvoid_cast (Function),

#define JNC_API_END_VTABLE() \
	}; \
	return VTable; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_API_BEGIN_LIB() \
static \
bool \
Export (jnc::CModule* pModule) \
{ \
	bool Result = true; \
	jnc::CFunction* pFunction = NULL; \
	jnc::CProperty* pProperty = NULL; \
	jnc::CNamespace* pNamespace = pModule->m_NamespaceMgr.GetGlobalNamespace ();

#define JNC_API_END_LIB() \
	return true; \
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_API_LIB(Lib) \
	Result = Lib::Export (pModule); \
	if (!Result) \
		return false;

#define JNC_API_CLASS(Class) \
	Result = Class::Export (pModule); \
	if (!Result) \
		return false;

#define JNC_API_FUNCTION(Name, Function) \
	pFunction = pNamespace->GetFunctionByName (Name); \
	if (!pFunction) \
		return false; \
	pModule->MapFunction (pFunction->GetLlvmFunction (), pvoid_cast (Function));

#define JNC_API_STD_FUNCTION_FORCED(StdFunc, Function) \
	pFunction = pModule->m_FunctionMgr.GetStdFunction (StdFunc); \
	ASSERT (pFunction); \
	pModule->MapFunction (pFunction->GetLlvmFunction (), pvoid_cast (Function));

#define JNC_API_STD_FUNCTION(StdFunc, Function) \
	if (pModule->m_FunctionMgr.IsStdFunctionUsed (StdFunc)) \
	{ \
		JNC_API_STD_FUNCTION_FORCED (StdFunc, Function); \
	}

#define JNC_API_PROPERTY(Name, Getter, Setter) \
	pProperty = pNamespace->GetPropertyByName (Name); \
	if (!pProperty) \
		return false; \
	pModule->MapFunction (pProperty->GetGetter ()->GetLlvmFunction (), pvoid_cast (Getter)); \
	pModule->MapFunction (pProperty->GetSetter ()->GetLlvmFunction (), pvoid_cast (Setter));

#define JNC_API_CONST_PROPERTY(Name, Getter) \
	pProperty = pNamespace->GetPropertyByName (Name); \
	if (!pProperty) \
		return false; \
	pModule->MapFunction (pProperty->GetGetter ()->GetLlvmFunction (), pvoid_cast (Getter));

#define JNC_API_AUTOGET_PROPERTY(Name, Setter) \
	pProperty = pNamespace->GetPropertyByName (Name); \
	if (!pProperty) \
		return false; \
	pModule->MapFunction (pProperty->GetSetter ()->GetLlvmFunction (), pvoid_cast (Setter));

//.............................................................................

void
Prime (
	CClassType* pType,
	void* pVTable,
	TObjHdr* pObject,
	size_t ScopeLevel,
	TObjHdr* pRoot,
	uintptr_t Flags
	);

template <typename T>
class CApiObjBoxT:
	public TObjHdr,
	public T
{
public:
	void
	Prime (
		size_t ScopeLevel,
		jnc::TObjHdr* pRoot,
		uintptr_t Flags = 0
		)
	{
		jnc::Prime (T::GetApiClassType (), T::GetApiClassVTable (), this, ScopeLevel, pRoot, Flags);
	}

	void
	Prime (jnc::TObjHdr* pRoot)
	{
		Prime (pRoot->m_ScopeLevel, pRoot, pRoot->m_Flags);
	}

	void
	Prime () // most common primer with scope level 0
	{
		Prime (0, this, 0);
	}
};

//.............................................................................

} // namespace jnc
