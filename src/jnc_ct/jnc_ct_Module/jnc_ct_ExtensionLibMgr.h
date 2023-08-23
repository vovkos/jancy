//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_ct_ModuleItem.h"

namespace jnc {
namespace ct {

//..............................................................................

class ExtensionLibMgr {
protected:
	struct SourceFile: sl::ListLink {
		ExtensionLib* m_lib;
		sl::StringRef m_fileName;
		sl::StringRef m_contents;
		zip::ZipReader* m_zipReader;
		size_t m_zipIndex;
	};

	struct DynamicLibEntry: sl::ListLink {
		ExtensionLib* m_lib;
		zip::ZipReader m_zipReader;
		sl::String m_zipFilePath;
		sl::String m_dynamicLibFilePath;
		sys::DynamicLib m_dynamicLib;
	};

	struct ItemCacheEntry: sl::ListLink {
		sl::Guid m_libGuid;
		sl::Array<ct::ModuleItem*> m_itemArray;
	};

	typedef sl::DuckTypeHashTable<sl::Guid, ItemCacheEntry*> ItemCacheMap;

protected:
	ct::Module* m_module;
	sl::Array<ExtensionLib*> m_libArray;
	sl::List<DynamicLibEntry> m_dynamicLibList;
	sl::SimpleHashTable<ExtensionLib*, DynamicLibEntry*> m_dynamicLibMap;
	sl::List<SourceFile> m_sourceFileList;
	sl::StringHashTable<SourceFile*> m_sourceFileMap;
	sl::StringHashTable<const OpaqueClassTypeInfo*> m_opaqueClassTypeInfoMap;
	sl::List<ItemCacheEntry> m_itemCache;
	ItemCacheMap m_itemCacheMap;
	sys::CodeAuthenticator* m_codeAuthenticator;

public:
	sl::String m_dynamicLibraryDir;

public:
	ExtensionLibMgr();

	~ExtensionLibMgr() {
		clear();
	}

	ct::Module*
	getModule() {
		return m_module;
	}

	const sl::StringHashTable<SourceFile*>&
	getSourceFileMap() {
		return m_sourceFileMap;
	}

	void
	clear();

	void
	setDynamicExtensionAuthenticatorConfig(const CodeAuthenticatorConfig* config);

	void
	updateCapabilities();

	void
	addStaticLib(ExtensionLib* lib);

	bool
	loadDynamicLib(const sl::StringRef& filePath);

	void
	closeDynamicLibZipReaders();

	bool
	mapAddresses();

	sl::StringRef
	getExtensionLibFilePath(ExtensionLib* lib) {
		sl::MapIterator<ExtensionLib*, DynamicLibEntry*> it = m_dynamicLibMap.find(lib);
		return it ? sl::StringRef(it->m_value->m_zipFilePath) : sl::StringRef();
	}

	const OpaqueClassTypeInfo*
	findOpaqueClassTypeInfo(const sl::StringRef& qualifiedName) {
		sl::StringHashTableIterator<const OpaqueClassTypeInfo*> it = m_opaqueClassTypeInfoMap.find(qualifiedName);
		return it ? it->m_value : NULL;
	}

	FindModuleItemResult
	findItem(
		const sl::StringRef& name,
		const sl::Guid& libGuid,
		size_t cacheSlot
	);

	bool
	findSourceFileContents(
		const sl::StringRef& fileName,
		ExtensionLib** lib,
		sl::StringRef* contents
	);

	void
	addSource(
		ExtensionLib* lib,
		const sl::StringRef& fileName,
		const sl::StringRef& contents
	);

	void
	addOpaqueClassTypeInfo(
		const sl::StringRef& qualifiedName,
		const OpaqueClassTypeInfo* info
	) {
		m_opaqueClassTypeInfoMap[qualifiedName] = info;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
