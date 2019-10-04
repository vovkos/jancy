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

ExtensionLibMgr::ExtensionLibMgr()
{
	m_module = Module::getCurrentConstructedModule();
	m_dynamicLibraryDir = io::getTempDir();
	ASSERT(m_module);
}

void
ExtensionLibMgr::clear()
{
	m_libArray.clear();

	while (!m_dynamicLibList.isEmpty())
	{
		DynamicLibEntry* entry = m_dynamicLibList.removeHead();

		if (entry->m_dynamicLib.isOpen())
		{
			DynamicExtensionLibUnloadFunc* unloadFunc = (DynamicExtensionLibUnloadFunc*)entry->m_dynamicLib.getFunction(jnc_g_dynamicExtensionLibUnloadFuncName);
			if (!unloadFunc || unloadFunc())
				entry->m_dynamicLib.close();
			else
				entry->m_dynamicLib.detach(); // don't unload
		}

		if (!entry->m_dynamicLibFilePath.isEmpty())
			io::deleteFile(entry->m_dynamicLibFilePath);

		AXL_MEM_DELETE(entry);
	}

	m_sourceFileList.clear();
	m_sourceFileMap.clear();
	m_opaqueClassTypeInfoMap.clear();
	m_itemCache.clear();
	m_itemCacheMap.clear();
}

void
ExtensionLibMgr::addStaticLib(ExtensionLib* lib)
{
	m_libArray.append(lib);

	lib->m_addSourcesFunc(m_module);
	lib->m_addOpaqueClassTypeInfosFunc(m_module);
}

bool
ExtensionLibMgr::loadDynamicLib(const sl::StringRef& fileName)
{
	static char jncExt[] = ".jnc";
	static char binExt[] = ".bin";

	bool result;

	DynamicLibEntry* entry = AXL_MEM_NEW(DynamicLibEntry);
	m_dynamicLibList.insertTail(entry);
	result = entry->m_zipReader.openFile(fileName);
	if (!result)
		return false;

	size_t dynamicLibFileIdx = -1;
	sl::String dynamicLibFileName;

	char buffer[256];
	sl::Array<size_t> forcedImportIdxArray(ref::BufKind_Stack, buffer, sizeof(buffer));

	sl::Iterator<SourceFile> sourceFileIt = m_sourceFileList.getTail(); // save source file iterator

	size_t count = entry->m_zipReader.getFileCount();
	for (size_t i = 0; i < count; i++)
	{
		sl::String fileName = entry->m_zipReader.getFileName(i);
		if (fileName.isEmpty()) // wat?
			continue;

		if (fileName.isSuffix(binExt))
		{
			dynamicLibFileName = fileName;
			dynamicLibFileIdx = i;
		}
		else if (fileName.isSuffix(jncExt))
		{
			if (fileName[0] == '.')
			{
				forcedImportIdxArray.append(i);
			}
			else
			{
				SourceFile* sourceFile = AXL_MEM_NEW(SourceFile);
				sourceFile->m_fileName = fileName;
				sourceFile->m_zipReader = &entry->m_zipReader;
				sourceFile->m_zipIndex = i;
				m_sourceFileList.insertTail(sourceFile);
				m_sourceFileMap[fileName] = sourceFile;
			}
		}
	}

	if (dynamicLibFileIdx == -1) // source-only extension
	{
		size_t count = forcedImportIdxArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			size_t j = forcedImportIdxArray[i];
			sl::Array<char> contents = entry->m_zipReader.extractFileToMem(j);
			sl::StringRef source(contents.getHdr(), contents.cp(), contents.getCount());

			m_module->m_importMgr.addImport(entry->m_lib, NULL, source);
		}

		return true;
	}

	entry->m_dynamicLibFilePath.format("%s/%llx-%s", m_dynamicLibraryDir.sz(), sys::getTimestamp (), dynamicLibFileName.sz());

	result =
		entry->m_zipReader.extractFileToFile(dynamicLibFileIdx, entry->m_dynamicLibFilePath) &&
		entry->m_dynamicLib.open(entry->m_dynamicLibFilePath);

	if (!result)
		return false;

	DynamicExtensionLibMainFunc* mainFunc = (DynamicExtensionLibMainFunc*)entry->m_dynamicLib.getFunction(jnc_g_dynamicExtensionLibMainFuncName);
	if (!mainFunc)
		return false;

	ExtensionLib* lib = mainFunc(&jnc_g_dynamicExtensionLibHostImpl);
	if (!lib)
	{
		err::setFormatStringError("cannot get extension lib in '%s'", fileName.sz());
		return false;
	}

	for (sourceFileIt++; sourceFileIt; sourceFileIt++)
		sourceFileIt->m_lib = lib;

	entry->m_lib = lib;
	m_libArray.append(lib);

	lib->m_addSourcesFunc(m_module);
	lib->m_addOpaqueClassTypeInfosFunc(m_module);

	return true;
}

bool
ExtensionLibMgr::mapAddresses()
{
	size_t count = m_libArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		bool result = m_libArray[i]->m_mapAddressesFunc(m_module) != 0;
		if (!result)
			return false;
	}

	return true;
}

bool
ExtensionLibMgr::findSourceFileContents(
	const sl::StringRef& fileName,
	ExtensionLib** lib,
	sl::StringRef* contents
	)
{
	sl::StringHashTableIterator<SourceFile*> it = m_sourceFileMap.find(fileName);
	if (!it)
		return false;

	SourceFile* file = it->m_value;
	if (file->m_zipIndex != -1)
	{
		sl::Array<char> contents = file->m_zipReader->extractFileToMem(file->m_zipIndex);
		file->m_contents = sl::StringRef(contents.getHdr(), contents.cp(), contents.getCount());
		file->m_zipReader = NULL;
		file->m_zipIndex = -1;
	}

	*lib = file->m_lib;
	*contents = file->m_contents;
	return true;
}

FindModuleItemResult
ExtensionLibMgr::findItem(
	const sl::StringRef& name,
	const sl::Guid& libGuid,
	size_t cacheSlot
	)
{
	ASSERT(m_module);

	if (cacheSlot == -1) // no caching for this item
		return m_module->m_namespaceMgr.getGlobalNamespace()->findItem(name);

	ItemCacheEntry* entry;
	ItemCacheMap::Iterator it = m_itemCacheMap.visit(libGuid);
	if (it->m_value)
	{
		entry = it->m_value;
	}
	else
	{
		entry = AXL_MEM_NEW(ItemCacheEntry);
		m_itemCache.insertTail(entry);
		it->m_value = entry;
	}

	size_t count = entry->m_itemArray.getCount();
	if (count <= cacheSlot)
		entry->m_itemArray.setCountZeroConstruct(cacheSlot + 1);

	if (entry->m_itemArray[cacheSlot])
		return FindModuleItemResult(entry->m_itemArray[cacheSlot]);

	FindModuleItemResult findResult = m_module->m_namespaceMgr.getGlobalNamespace()->findItem(name);
	if (findResult.m_item)
		entry->m_itemArray[cacheSlot] = findResult.m_item;

	return findResult;
}

void
ExtensionLibMgr::addSource(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& contents
	)
{
	SourceFile* file = AXL_MEM_NEW(SourceFile);
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
