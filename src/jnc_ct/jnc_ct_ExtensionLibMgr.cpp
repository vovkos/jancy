#include "pch.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ExtensionLibMgr::ExtensionLibMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	m_dynamicLibraryDir = io::getTempDir ();
	ASSERT (m_module);
}

void
ExtensionLibMgr::clear ()
{
	m_libArray.clear ();	
	m_itemCache.clear ();

	while (!m_dynamicLibList.isEmpty ())
	{
		DynamicLibEntry* entry = m_dynamicLibList.removeHead ();
		entry->m_dynamicLib.close ();
		io::deleteFile (entry->m_dynamicLibFilePath);
		AXL_MEM_DELETE (entry);
	}
}

bool
ExtensionLibMgr::addStaticLib (ExtensionLib* lib)
{
	LibEntry entry;
	entry.m_extensionLib = lib;
	entry.m_dynamicLibEntry = NULL;
	m_libArray.append (entry);
	
	return lib->forcedExport (m_module);
}

bool
ExtensionLibMgr::loadDynamicLib (const char* fileName)
{
	static char jncExt [] = ".jnc";
	static char binExt [] = ".bin";

	bool result;

	DynamicLibEntry* entry = AXL_MEM_NEW (DynamicLibEntry);
	m_dynamicLibList.insertTail (entry);	
	result = entry->m_zipReader.openFile (fileName);
	if (!result)
		return false;

	size_t dynamicLibFileIdx;
	sl::String dynamicLibFileName;

	size_t count = entry->m_zipReader.getFileCount ();
	for (size_t i = 0; i < count; i++)
	{
		sl::String fileName = entry->m_zipReader.getFileName (i);
		size_t length = fileName.getLength ();

		if (length > lengthof (binExt) && strcmp (fileName.cc () + length - lengthof (binExt), binExt) == 0)
		{
			dynamicLibFileName = fileName;
			dynamicLibFileIdx = i;
		}
		else if (length > lengthof (jncExt) && strcmp (fileName.cc () + length - lengthof (jncExt), jncExt) == 0)
		{
			DynamicLibSourceFile* sourceFile = AXL_MEM_NEW (DynamicLibSourceFile);
			sourceFile->m_fileName = fileName;
			sourceFile->m_index = i;
			entry->m_sourceFileList.insertTail (sourceFile);
			entry->m_sourceFileMap [fileName] = sourceFile;
		}
	}

	if (dynamicLibFileIdx == -1)
	{
		err::setFormatStringError ("'%s' does not contain a dynamic library binary (*.bin)", fileName);
		return false;
	}

	entry->m_dynamicLibFilePath.format ("%s/%llx-%s", m_dynamicLibraryDir.cc (), sys::getTimestamp (), dynamicLibFileName.cc ());

	result = 
		entry->m_zipReader.extractFileToFile (dynamicLibFileIdx, entry->m_dynamicLibFilePath);
		entry->m_dynamicLib.open (entry->m_dynamicLibFilePath);

	if (!result)
		return false;

	DynamicExtensionLibMainFunc* mainFunc = (DynamicExtensionLibMainFunc*) entry->m_dynamicLib.getFunction (jnc_g_dynamicExtensionLibMainFuncName);
	if (!mainFunc)
		return false;

	ExtensionLib* extensionLib = mainFunc (&jnc_g_dynamicExtensionLibHostImpl);
	if (!extensionLib)
	{
		err::setFormatStringError ("cannot get extension lib in '%s'", fileName);
		return false;
	}

	LibEntry libEntry;
	libEntry.m_extensionLib = extensionLib;
	libEntry.m_dynamicLibEntry = entry;
	m_libArray.append (libEntry);

	return extensionLib->forcedExport (m_module);
}

bool
ExtensionLibMgr::mapFunctions ()
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		bool result = m_libArray [i].m_extensionLib->mapFunctions (m_module);
		if (!result)
			return false;
	}

	return true;
}

sl::StringRef
ExtensionLibMgr::findSourceFileContents (const char* fileName)
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		LibEntry* libEntry = &m_libArray [i];

		// do findSourceFileContents even for dynamic libs (chance to override source)

		StringSlice contents = libEntry->m_extensionLib->findSourceFileContents (fileName);
		if (contents.m_p)
			return sl::StringRef (contents.m_p, contents.m_length);

		if (!libEntry->m_dynamicLibEntry)
			continue;

		sl::StringHashTableMapIterator <DynamicLibSourceFile*> it = libEntry->m_dynamicLibEntry->m_sourceFileMap.find (fileName);
		if (!it)
			continue;

		DynamicLibSourceFile* sourceFile = it->m_value;
		if (sourceFile->m_contents.isEmpty ())
		{
			sourceFile->m_contents = libEntry->m_dynamicLibEntry->m_zipReader.extractFileToMem (sourceFile->m_index);
			sourceFile->m_contents.append (0); // ensure zero-termination
		}

		return sl::StringRef (sourceFile->m_contents, sourceFile->m_contents.getCount () - 1);
	}

	return sl::StringRef ();
}

const OpaqueClassTypeInfo*
ExtensionLibMgr::findOpaqueClassTypeInfo (const char* qualifiedName)
{
	size_t count = m_libArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		const OpaqueClassTypeInfo* typeInfo = m_libArray [i].m_extensionLib->findOpaqueClassTypeInfo (qualifiedName);
		if (typeInfo)
			return typeInfo;
	}

	return NULL;
}

#if 0

ct::ModuleItem*
ExtensionLibMgr::findItem (
	const char* name,
	const sl::Guid& libGuid,
	size_t itemCacheSlot
	)
{
	ASSERT (m_module);

	if (itemCacheSlot == -1) // no caching for this item
		return m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);

	size_t count = m_itemCache.getCount ();
	if (count <= libCacheSlot)
		m_itemCache.setCount (libCacheSlot + 1);

	sl::Array <ModuleItem*>* itemArray = m_itemCache [libCacheSlot];
	if (!itemArray)
	{
		itemArray = AXL_MEM_NEW (sl::Array <ModuleItem*>);
		m_itemCache [libCacheSlot] = itemArray;
	}

	count = itemArray->getCount ();
	if (count <= itemCacheSlot)
		itemArray->setCount (itemCacheSlot + 1);

	ModuleItem* item = (*itemArray) [itemCacheSlot];
	if (item)
		return item;

	item = m_module->m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);
	(*itemArray) [itemCacheSlot] = item;
	return item;	
}

#endif

//.............................................................................

} // namespace ct
} // namespace jnc
