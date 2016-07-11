// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ext {

class ExtensionLib;
class ExtensionLib;
class ExtensionLibHost;

//.............................................................................

class ExtensionLibMgr
{
protected:
	struct DynamicLibSourceFile: public sl::ListLink
	{
		size_t m_index;
		sl::String m_fileName;
		sl::Array <char> m_contents;
	};

	struct DynamicLibEntry: public sl::ListLink
	{
		zip::ZipReader m_zipReader;
		sl::String m_dynamicLibFilePath;
		sys::DynamicLibrary m_dynamicLib;
		sl::StringHashTableMap <DynamicLibSourceFile*> m_sourceFileMap;
		sl::StdList <DynamicLibSourceFile> m_sourceFileList;
	};

	struct LibEntry
	{
		ExtensionLib* m_extensionLib;
		DynamicLibEntry* m_dynamicLibEntry;
	};

protected:
	ct::Module* m_module;
	sl::Array <LibEntry> m_libArray;
	sl::StdList <DynamicLibEntry> m_dynamicLibList;
	sl::AutoPtrArray <sl::Array <ct::ModuleItem*> > m_itemCache;

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

	bool
	addStaticLib (ExtensionLib* lib);

	bool
	loadDynamicLib (const char* fileName);

	bool
	mapFunctions ();

	sl::StringRef
	findSourceFileContents (const char* fileName);

	const OpaqueClassTypeInfo* 
	findOpaqueClassTypeInfo (const char* qualifiedName);

	ct::ModuleItem*
	findItem (
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ExtensionLib*
getStdLib (ExtensionLibHost* host);

ExtensionLib*
getSysLib (ExtensionLibHost* host);

//.............................................................................

} // namespace ext
} // namespace jnc
