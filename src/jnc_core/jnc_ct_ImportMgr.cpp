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
ImportMgr::addImport (const char* fileName)
{
	sl::String filePath;

	// extension lib or source?

	static char extensionLibSuffix [] = ".jncx";
	
	enum
	{
		ExtensionLibSuffixLength = lengthof (extensionLibSuffix)
	};

	size_t length = strlen (fileName);
	bool isExtensionLib = length >= ExtensionLibSuffixLength && memcmp (
		fileName + length - ExtensionLibSuffixLength, 
		extensionLibSuffix, 
		ExtensionLibSuffixLength
		) == 0;
	
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

	sl::StringSlice source = m_module->m_extensionLibMgr.findSourceFileContents (fileName);
	if (!source.isEmpty ())
	{
		addSource (fileName, source);
		return true;
	}

	// not found, now search the file system

	FindResult findResult = findImportFile (fileName, &filePath);
	if (findResult == FindResult_NotFound)
		return false;
	else if (findResult == FindResult_AlreadyImported)
		return true;

	Import* import = AXL_MEM_NEW (Import);
	import->m_importKind = ImportKind_File;
	import->m_filePath = filePath;
	m_importList.insertTail (import);
	return true;	
}

void
ImportMgr::addSource (
	const sl::String& filePath,
	const sl::StringSlice& source
	)
{
	sl::StringHashTableMapIterator <bool> it = m_importFilePathMap.visit (filePath);
	if (it->m_value)
		return; // already added

	Import* import = AXL_MEM_NEW (Import);
	import->m_importKind = ImportKind_Source;
	import->m_filePath = filePath;
	import->m_source = source;
	m_importList.insertTail (import);
	it->m_value = true;
}

ImportMgr::FindResult
ImportMgr::findImportFile (
	const char* fileName,
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
