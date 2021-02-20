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

class Module;

//..............................................................................

enum ImportKind
{
	ImportKind_File,
	ImportKind_Source
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Import: sl::ListLink
{
	ImportKind m_importKind;
	ExtensionLib* m_lib;
	sl::String m_filePath;
	sl::StringRef m_source;
};

//..............................................................................

enum LazyImportFlag
{
	LazyImportFlag_Used = 0x010000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LazyImport: public ModuleItem
{
	friend class ImportMgr;

protected:
	ExtensionLib* m_lib;
	sl::String m_fileName;
	sl::StringRef m_source;

public:
	LazyImport()
	{
		m_itemKind = ModuleItemKind_LazyImport;
	}

	ExtensionLib*
	getLib()
	{
		return m_lib;
	}

	const sl::String&
	getFileName()
	{
		return m_fileName;
	}

	const sl::StringRef&
	getSource()
	{
		return m_source;
	}
};

//..............................................................................

class ImportMgr
{
	friend class Module;

protected:
	enum FindResult
	{
		FindResult_NotFound,
		FindResult_Found,
		FindResult_AlreadyImported,
	};

protected:
	Module* m_module;

	sl::List<Import> m_importList;
	sl::List<LazyImport> m_lazyImportList;
	sl::StringHashTable<bool> m_ignoredImportSet;
	sl::StringHashTable<bool> m_importFilePathMap;
	sys::CodeAuthenticator* m_codeAuthenticator;

public:
	sl::BoxList<sl::String> m_importDirList;

public:
	ImportMgr();

	~ImportMgr()
	{
		clear();
	}

	Module*
	getModule()
	{
		return m_module;
	}

	void
	clear();

	void
	setDynamicExtensionAuthenticatorConfig(const CodeAuthenticatorConfig* config);

	bool
	addImport(const sl::StringRef& fileName);

	void
	addImport(
		ExtensionLib* lib,
		const sl::StringRef& filePath,
		const sl::StringRef& source
		);

	void
	addIgnoredImport(const sl::StringRef& fileName)
	{
		m_ignoredImportSet.visit(fileName);
	}

	void
	takeOverImports(sl::List<Import>* list)
	{
		sl::takeOver(list, &m_importList);
	}

	LazyImport*
	createLazyImport(
		ExtensionLib* lib,
		const sl::StringRef& fileName,
		const sl::StringRef& source
		);

	bool
	parseLazyImport(LazyImport* import);

protected:
	FindResult
	findImportFile(
		const sl::StringRef& fileName,
		sl::String* filePath
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
