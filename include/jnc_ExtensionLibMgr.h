// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_ExtensionLib.h"

namespace jnc {

class ExtensionLib;

//.............................................................................

class ExtensionLibMgr
{
protected:
	Module* m_module;
	rtl::Array <ExtensionLib*> m_libArray;
	rtl::BoxList <mt::DynamicLibrary> m_dynamicLibList;
	rtl::AutoPtrArray <rtl::Array <ModuleItem*> > m_itemCache;

public:
	ExtensionLibMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	bool
	loadDynamicLib (const char* fileName);

	bool
	addLib (ExtensionLib* lib);

	bool
	mapFunctions ();

	rtl::StringSlice
	findSourceFile (const char* fileName);

	const OpaqueClassTypeInfo* 
	findOpaqueClassTypeInfo (const char* qualifiedName);

	ModuleItem*
	findItem (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		);

	Type*
	findType (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		)
	{
		ModuleItem* item = findItem (libSlot, itemSlot, name);
		return item ? verifyModuleItemIsType (item, name) : NULL;
	}

	DerivableType*
	findDerivableType (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		)
	{
		ModuleItem* item = findItem (libSlot, itemSlot, name);
		return item ? verifyModuleItemIsDerivableType (item, name) : NULL;
	}

	ClassType*
	findClassType (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		)
	{
		ModuleItem* item = findItem (libSlot, itemSlot, name);
		return item ? verifyModuleItemIsClassType (item, name) : NULL;
	}

	Function*
	findFunction (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		)
	{
		ModuleItem* item = findItem (libSlot, itemSlot, name);
		return item ? verifyModuleItemIsFunction (item, name) : NULL;
	}

	Function*
	findFunction (
		size_t libSlot,
		size_t itemSlot,
		const char* name,
		size_t overloadIdx
		)
	{
		Function* function = findFunction (libSlot, itemSlot, name);
		return function ? function->getOverload (overloadIdx) : NULL;
	}

	Property*
	findProperty (
		size_t libSlot,
		size_t itemSlot,
		const char* name
		)
	{
		ModuleItem* item = findItem (libSlot, itemSlot, name);
		return item ? verifyModuleItemIsProperty (item, name) : NULL;
	}
};

//.............................................................................

class ExtensionLibSlotDb
{
	typedef rtl::HashTableMap <
		rtl::Guid, 
		size_t, 
		rtl::HashDjb2 <rtl::Guid>, 
		rtl::CmpBin <rtl::Guid> 
		> SlotMap;

protected:
	size_t m_slot;
	SlotMap m_slotMap;

public:
	ExtensionLibSlotDb ()
	{
		m_slot = 0;
	}

	size_t 
	getSlot (const rtl::Guid& guid);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ExtensionLibSlotDb*
getExtensionLibSlotDb ()
{
	return rtl::getSingleton <ExtensionLibSlotDb> ();
}

//.............................................................................

} // namespace jnc
