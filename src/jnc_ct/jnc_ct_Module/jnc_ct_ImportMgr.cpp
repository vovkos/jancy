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
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ImportMgr::ImportMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

void
ImportMgr::clear()
{
	m_importList.clear();
	m_lazyImportList.clear();
	m_importFilePathMap.clear();
	m_ignoredImportSet.clear();
}

bool
ImportMgr::addImport(const sl::StringRef& fileName)
{
	sl::String filePath;

	if (m_ignoredImportSet.find(fileName))
		return true;

	bool isExtensionLib = fileName.isSuffix(".jncx");
	if (isExtensionLib)
	{
		FindResult findResult = findImportFile(fileName, &filePath);
		if (findResult == FindResult_NotFound)
			return false;
		else if (findResult == FindResult_AlreadyImported)
			return true;

		return m_module->m_extensionLibMgr.loadDynamicLib(filePath);
	}

	// source. search extension libs first

	ExtensionLib* lib;
	sl::StringRef source;
	bool isFound = m_module->m_extensionLibMgr.findSourceFileContents(fileName, &lib, &source);
	if (isFound)
	{
		addImport(lib, fileName, source);
		return true;
	}

	// not found, now search the file system

	FindResult findResult = findImportFile(fileName, &filePath);
	if (findResult == FindResult_NotFound)
		return false;
	else if (findResult == FindResult_AlreadyImported)
		return true;

	Import* import = AXL_MEM_NEW(Import);
	import->m_lib = NULL;
	import->m_importKind = ImportKind_File;
	import->m_filePath = filePath;
	m_importList.insertTail(import);
	return true;
}

void
ImportMgr::addImport(
	ExtensionLib* lib,
	const sl::StringRef& filePath,
	const sl::StringRef& source
	)
{
	sl::StringHashTableIterator<bool> it;

	if (!filePath.isEmpty())
	{
		it = m_importFilePathMap.visit(filePath);
		if (it->m_value)
			return; // already added
	}

	Import* import = AXL_MEM_NEW(Import);
	import->m_lib = lib;
	import->m_importKind = ImportKind_Source;
	import->m_filePath = filePath;
	import->m_source = source;
	m_importList.insertTail(import);

	if (it)
		it->m_value = true;
}

ImportMgr::FindResult
ImportMgr::findImportFile(
	const sl::StringRef& fileName,
	sl::String* filePath_o
	)
{
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();

	sl::String filePath = unit ?
		io::findFilePath(fileName, unit->getDir(), &m_importDirList, false) :
		io::findFilePath(fileName, &m_importDirList, false);

	if (filePath.isEmpty())
	{
		err::setFormatStringError("import '%s' not found", fileName.sz());
		return FindResult_NotFound;
	}

	sl::StringHashTableIterator<bool> it = m_importFilePathMap.visit(filePath);
	if (it->m_value) // already
		return FindResult_AlreadyImported;

	it->m_value = true;
	*filePath_o = filePath;
	return FindResult_Found;
}

LazyImport*
ImportMgr::createLazyImport(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& source
	)
{
	LazyImport* import = AXL_MEM_NEW(LazyImport);
	import->m_module = m_module;
	import->m_lib = lib;
	import->m_fileName = fileName;
	import->m_source = source;
	m_lazyImportList.insertTail(import);
	return import;
}

bool
ImportMgr::parseLazyImport(LazyImport* import)
{
	ASSERT(m_module->getCompileState() < ModuleCompileState_Compiled);

	sl::ConstIterator<Variable> lastVariableIt = m_module->m_variableMgr.getVariableList().getTail();
	sl::ConstIterator<Property> lastPropertyIt = m_module->m_functionMgr.getPropertyList().getTail();

	import->m_flags |= LazyImportFlag_Used;

	addImport(import->m_lib, import->m_fileName, import->m_source);

	Unit* prevUnit = m_module->m_unitMgr.getCurrentUnit();
	m_module->m_namespaceMgr.openNamespace(m_module->m_namespaceMgr.getGlobalNamespace());

	bool result =
		m_module->parseImports() &&
		m_module->m_namespaceMgr.getGlobalNamespace()->resolveOrphans() &&
		m_module->m_variableMgr.allocateNamespaceVariables(lastVariableIt) &&
		m_module->m_functionMgr.finalizeNamespaceProperties(lastPropertyIt);

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);
	return result;
}

//..............................................................................

} // namespace ct
} // namespace jnc
