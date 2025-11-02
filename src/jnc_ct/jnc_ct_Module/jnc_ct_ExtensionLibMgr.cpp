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

#include "pch.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ExtensionLibMgr::ExtensionLibMgr() {
	m_module = Module::getCurrentConstructedModule();
	m_dynamicLibraryDir = io::getTempDir();
	m_codeAuthenticator = NULL;
	ASSERT(m_module);
}

void
ExtensionLibMgr::clear() {
	unloadDynamicLibs();

	m_libArray.clear();
	m_dynamicLibList.clear();
	m_sourceFileList.clear();
	m_sourceFileMap.clear();
	m_opaqueClassTypeInfoMap.clear();
	m_itemCache.clear();
	m_itemCacheMap.clear();

	delete m_codeAuthenticator;
	m_codeAuthenticator = NULL;
}

void
ExtensionLibMgr::updateCapabilities() {
	size_t count = m_libArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ExtensionLib* lib = m_libArray[i];
		if (lib->m_updateCapabilitiesFunc)
			lib->m_updateCapabilitiesFunc();
	}
}

void
ExtensionLibMgr::closeDynamicLibZipReaders() {
	sl::Iterator<DynamicLibEntry> it = m_dynamicLibList.getHead();
	for (; it; it++)
		it->m_zipReader.close();
}

void
ExtensionLibMgr::unloadDynamicLibs() {
	sl::Iterator<DynamicLibEntry> it = m_dynamicLibList.getHead();
	for (; it; it++) {
		DynamicLibEntry* entry = *it;
		if (entry->m_dynamicLib.isOpen()) {
			DynamicExtensionLibUnloadFunc* unloadFunc = (DynamicExtensionLibUnloadFunc*)entry->m_dynamicLib.getFunction(jnc_g_dynamicExtensionLibUnloadFuncName);
			if (!unloadFunc || unloadFunc())
				entry->m_dynamicLib.close();
			else
				entry->m_dynamicLib.detach(); // don't unload
		}

		if (!entry->m_dynamicLibFilePath.isEmpty()) {
			io::deleteFile(entry->m_dynamicLibFilePath);
			entry->m_dynamicLibFilePath.clear();
		}
	}
}

void
ExtensionLibMgr::setDynamicExtensionAuthenticatorConfig(const CodeAuthenticatorConfig* config) {
	m_codeAuthenticator = new sys::CodeAuthenticator;

#if (_JNC_OS_WIN)
	m_codeAuthenticator->setup(
		sl::StringRef(),
		config->m_expectedSubjectName,
		config->m_expectedIssuerName,
		sl::ArrayRef<char>(
			config->m_expectedSerialNumber,
			config->m_expectedSerialNumberSize
		)
	);
#elif (_JNC_OS_LINUX)
	m_codeAuthenticator->setup(
		config->m_signatureSectionName,
		config->m_publicKeyPem
	);
#elif (_JNC_OS_DARWIN)
	m_codeAuthenticator->setup(config->m_expectedTeamId);
#endif
}

void
ExtensionLibMgr::addStaticLib(ExtensionLib* lib) {
	m_libArray.append(lib);

	lib->m_addSourcesFunc(m_module);
	lib->m_addOpaqueClassTypeInfosFunc(m_module);
}

bool
ExtensionLibMgr::loadDynamicLib(const sl::StringRef& filePath) {
	static char jncxExt[] = ".jncx";
	static char jncExt[] = ".jnc";
	static char binExt[] = ".bin";

	bool result;

	DynamicLibEntry* entry = new DynamicLibEntry;
	entry->m_lib = NULL;
	m_dynamicLibList.insertTail(entry);

	result = entry->m_zipReader.openFile(filePath);
	if (!result)
		return false;

	entry->m_zipFilePath = filePath;

	size_t dynamicLibFileIdx = -1;
	sl::String dynamicLibFileName;
	sl::String dynamicLibFilePath;

	char buffer[256];
	sl::Array<size_t> forcedImportIdxArray(rc::BufKind_Stack, buffer, sizeof(buffer));

	sl::Iterator<SourceFile> sourceFileIt = m_sourceFileList.getTail(); // save source file iterator

	size_t count = entry->m_zipReader.getFileCount();
	for (size_t i = 0; i < count; i++) {
		sl::String fileName = entry->m_zipReader.getFileName(i);
		if (fileName.isEmpty()) // wat?
			continue;

		if (fileName.isSuffix(binExt)) {
			dynamicLibFileName = fileName;
			dynamicLibFileIdx = i;
		} else if (fileName.isSuffix(jncExt)) {
			if (fileName[0] == '.')
				forcedImportIdxArray.append(i);
			else {
				SourceFile* sourceFile = new SourceFile;
				sourceFile->m_fileName = fileName;
				sourceFile->m_zipReader = &entry->m_zipReader;
				sourceFile->m_zipIndex = i;
				m_sourceFileList.insertTail(sourceFile);
				m_sourceFileMap[fileName] = sourceFile;
			}
		}
	}

	if (m_module->getCompileFlags() & ModuleCompileFlag_ExternalExtensionBin) { // prefer external bin
		dynamicLibFilePath = filePath;

		if (filePath.isSuffix(jncxExt))
			dynamicLibFilePath.chop(lengthof(jncxExt));

		dynamicLibFilePath += '-';
		dynamicLibFilePath += AXL_CPU_STRING;
		dynamicLibFilePath += binExt;

		if (!io::doesFileExist(dynamicLibFilePath))
			dynamicLibFilePath.clear();
	}

	if (dynamicLibFilePath.isEmpty() && dynamicLibFileIdx != -1) { // use zipped bin
		dynamicLibFilePath.format("%s/%llx-%s", m_dynamicLibraryDir.sz(), sys::getTimestamp (), dynamicLibFileName.sz());

		result = entry->m_zipReader.extractFileToFile(dynamicLibFileIdx, dynamicLibFilePath);
		if (!result)
			return false;

		entry->m_dynamicLibFilePath = dynamicLibFilePath;
	}

	if (dynamicLibFilePath.isEmpty()) { // source-only jncx
		size_t count = forcedImportIdxArray.getCount();
		for (size_t i = 0; i < count; i++) {
			size_t j = forcedImportIdxArray[i];
			sl::Array<char> contents = entry->m_zipReader.extractFileToMem(j);
			sl::StringRef source(contents.getHdr(), contents.cp(), contents.getCount());
			m_module->m_importMgr.addImport(entry->m_lib, NULL, source);
		}

		return true;
	}

	if (m_codeAuthenticator) {
		result = m_codeAuthenticator->verifyFile(dynamicLibFilePath);
		if (!result)
			return false;
	}

	result = entry->m_dynamicLib.open(dynamicLibFilePath);
	if (!result)
		return false;

	DynamicExtensionLibMainFunc* mainFunc = (DynamicExtensionLibMainFunc*)entry->m_dynamicLib.getFunction(jnc_g_dynamicExtensionLibMainFuncName);
	if (!mainFunc)
		return false;

	ExtensionLib* lib = mainFunc(&jnc_g_dynamicExtensionLibHostImpl);
	if (!lib)
		return false;

	for (sourceFileIt++; sourceFileIt; sourceFileIt++)
		sourceFileIt->m_lib = lib;

	entry->m_lib = lib;
	m_libArray.append(lib);
	m_dynamicLibMap[lib] = entry;

	lib->m_addSourcesFunc(m_module);
	lib->m_addOpaqueClassTypeInfosFunc(m_module);
	return true;
}

bool
ExtensionLibMgr::mapAddresses() {
	size_t count = m_libArray.getCount();
	for (size_t i = 0; i < count; i++) {
		bool result = m_libArray[i]->m_mapAddressesFunc(m_module) != 0;
		if (!result)
			return false;
	}

	return true;
}

FindModuleItemResult
ExtensionLibMgr::findItem(
	const sl::StringRef& name,
	const sl::Guid& libGuid,
	size_t cacheSlot
) {
	ASSERT(m_module);

	if (cacheSlot == -1) // no caching for this item
		return m_module->m_namespaceMgr.getGlobalNamespace()->findItemImpl<sl::False>(name);

	ItemCacheEntry* entry;
	ItemCacheMap::Iterator it = m_itemCacheMap.visit(libGuid);
	if (it->m_value) {
		entry = it->m_value;
	} else {
		entry = new ItemCacheEntry;
		m_itemCache.insertTail(entry);
		it->m_value = entry;
	}

	size_t count = entry->m_itemArray.getCount();
	if (count <= cacheSlot)
		entry->m_itemArray.setCountZeroConstruct(cacheSlot + 1);

	if (entry->m_itemArray[cacheSlot])
		return FindModuleItemResult(entry->m_itemArray[cacheSlot]);

	FindModuleItemResult findResult = m_module->m_namespaceMgr.getGlobalNamespace()->findItemImpl<sl::False>(name);
	if (findResult.m_item)
		entry->m_itemArray.rwi()[cacheSlot] = findResult.m_item;

	return findResult;
}

bool
ExtensionLibMgr::findSourceFileContents(
	const sl::StringRef& fileName,
	ExtensionLib** lib,
	sl::StringRef* contents
) {
	sl::StringHashTableIterator<SourceFile*> it = m_sourceFileMap.find(fileName);
	if (!it)
		return false;

	SourceFile* file = it->m_value;
	if (file->m_zipIndex != -1) {
		sl::Array<char> contents = file->m_zipReader->extractFileToMem(file->m_zipIndex);
		file->m_contents = sl::StringRef(contents.getHdr(), contents.cp(), contents.getCount());
		file->m_zipReader = NULL;
		file->m_zipIndex = -1;
	}

	*lib = file->m_lib;
	*contents = file->m_contents;
	return true;
}

void
ExtensionLibMgr::addSource(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& contents
) {
	SourceFile* file = new SourceFile;
	file->m_lib = lib;
	file->m_fileName = fileName;
	file->m_contents = contents;
	file->m_zipReader = NULL;
	file->m_zipIndex = -1;
	m_sourceFileList.insertTail(file);
	m_sourceFileMap[fileName] = file;
}

//..............................................................................

} // namespace ct
} // namespace jnc
