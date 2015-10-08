// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ext {

class ExtensionLib;
struct OpaqueClassTypeInfo;

//.............................................................................

class ExtensionLibMgr
{
protected:
	ct::Module* m_module;
	sl::Array <ExtensionLib*> m_libArray;
	sl::BoxList <mt::DynamicLibrary> m_dynamicLibList;
	sl::AutoPtrArray <sl::Array <ct::ModuleItem*> > m_itemCache;

public:
	ExtensionLibMgr ();

	ct::Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	ExtensionLib*
	loadDynamicLib (const char* fileName);

	bool
	addLib (ExtensionLib* lib);

	bool
	mapFunctions ();

	sl::StringSlice
	findSourceFile (const char* fileName);

	const OpaqueClassTypeInfo* 
	findOpaqueClassTypeInfo (const char* qualifiedName);

	ct::ModuleItem*
	findItem (
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		);
};

//.............................................................................

} // namespace ext
} // namespace jnc
