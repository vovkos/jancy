#include "pch.h"
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ImportMgr::ImportMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
ImportMgr::clear ()
{
	m_importList.clear ();
	m_importFilePathMap.clear ();
	m_extensionLibFilePathCache.clear ();
}

bool
ImportMgr::addImport (const sl::StringRef& fileName)
{
	sl::String filePath;

	bool isExtensionLib = fileName.isSuffix (".jncx");
	if (isExtensionLib)
	{
		FindResult findResult = findImportFile (fileName, &filePath);
		if (findResult == FindResult_NotFound)
			return false;
		else if (findResult == FindResult_AlreadyImported)
			return true;

		m_extensionLibFilePathCache.insertTail (filePath);
		return m_module->m_extensionLibMgr.loadDynamicLib (filePath);
	}

	// source. search extension libs first

	ExtensionLib* lib;
	sl::StringRef source;
	bool isFound = m_module->m_extensionLibMgr.findSourceFileContents (fileName, &lib, &source);
	if (isFound)
	{
		addImport (lib, fileName, source);
		return true;
	}

	// not found, now search the file system

	FindResult findResult = findImportFile (fileName, &filePath);
	if (findResult == FindResult_NotFound)
		return false;
	else if (findResult == FindResult_AlreadyImported)
		return true;

	Import* import = AXL_MEM_NEW (Import);
	import->m_lib = NULL;
	import->m_importKind = ImportKind_File;
	import->m_filePath = filePath;
	m_importList.insertTail (import);
	return true;	
}

void
ImportMgr::addImport (
	ExtensionLib* lib,
	const sl::StringRef& filePath,
	const sl::StringRef& source
	)
{
	sl::StringHashTableMapIterator <bool> it = m_importFilePathMap.visit (filePath);
	if (it->m_value)
		return; // already added

	Import* import = AXL_MEM_NEW (Import);
	import->m_lib = lib;
	import->m_importKind = ImportKind_Source;
	import->m_filePath = filePath;
	import->m_source = source;
	m_importList.insertTail (import);
	it->m_value = true;
}

ImportMgr::FindResult
ImportMgr::findImportFile (
	const sl::StringRef& fileName,
	sl::String* filePath_o
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit ();
	ASSERT (unit);

	sl::String filePath = io::findFilePath (
		fileName, 
		unit->getDir (),
		&m_importDirList,
		false
		);

	if (filePath.isEmpty ())
	{
		err::setFormatStringError ("import '%s' not found", fileName);
		return FindResult_NotFound;
	}

	sl::StringHashTableMapIterator <bool> it = m_importFilePathMap.visit (filePath);
	if (it->m_value) // already
		return FindResult_AlreadyImported;

	it->m_value = true;
	*filePath_o = filePath;
	return FindResult_Found;
}

//.............................................................................

} // namespace ct
} // namespace jnc
