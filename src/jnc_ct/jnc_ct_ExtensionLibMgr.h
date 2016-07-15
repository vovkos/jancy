// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ct {

//.............................................................................

class ExtensionLibMgr
{
protected:
	struct SourceFile: sl::ListLink
	{
		sl::StringRef m_fileName;
		sl::StringRef m_contents;
		zip::ZipReader* m_zipReader;
		size_t m_zipIndex;
	};

	struct DynamicLibEntry: sl::ListLink
	{
		ExtensionLib* m_lib;
		zip::ZipReader m_zipReader;
		sl::String m_dynamicLibFilePath;
		sys::DynamicLibrary m_dynamicLib;
	};

	struct ItemCacheEntry: sl::ListLink
	{
		sl::Guid m_libGuid;
		sl::Array <ct::ModuleItem*> m_itemArray;
	};

	typedef sl::HashTableMap <
		sl::Guid, 
		ItemCacheEntry*, 
		sl::HashDjb2 <sl::Guid>,
		sl::CmpBin <sl::Guid>
		> ItemCacheMap;

protected:
	ct::Module* m_module;
	sl::Array <ExtensionLib*> m_libArray;
	sl::StdList <DynamicLibEntry> m_dynamicLibList;
	sl::StdList <SourceFile> m_sourceFileList;
	sl::StringHashTableMap <SourceFile*> m_sourceFileMap;
	sl::StringHashTableMap <const OpaqueClassTypeInfo*> m_opaqueClassTypeInfoMap;
	sl::StdList <ItemCacheEntry> m_itemCache;
	ItemCacheMap m_itemCacheMap;

public:
	sl::String m_dynamicLibraryDir;

public:
	ExtensionLibMgr ();
	
	~ExtensionLibMgr ()
	{
		clear ();
	}

	ct::Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	void
	addStaticLib (ExtensionLib* lib);

	bool
	loadDynamicLib (const char* fileName);

	bool
	mapFunctions ();

	sl::StringRef
	findSourceFileContents (const char* fileName);

	const OpaqueClassTypeInfo* 
	findOpaqueClassTypeInfo (const char* qualifiedName)
	{
		sl::StringHashTableMapIterator <const OpaqueClassTypeInfo*> it = m_opaqueClassTypeInfoMap.find (qualifiedName);
		return it ? it->m_value : NULL;
	}

	ct::ModuleItem*
	findItem (
		const char* name,
		const sl::Guid& libGuid,
		size_t cacheSlot
		);

	void
	addSource (
		const sl::StringRef& fileName,
		const sl::StringRef& contents
		);

	void
	addOpaqueClassTypeInfo (
		const char* qualifiedName,
		const OpaqueClassTypeInfo* info
		)
	{
		m_opaqueClassTypeInfoMap [qualifiedName] = info;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ExtensionLib*
getStdLib ();

ExtensionLib*
getSysLib ();

//.............................................................................

} // namespace ct
} // namespace jnc
