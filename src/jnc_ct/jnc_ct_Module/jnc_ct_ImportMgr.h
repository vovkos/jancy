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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ImportMgr
{
protected:
	enum FindResult
	{
		FindResult_NotFound,
		FindResult_Found,
		FindResult_AlreadyImported,
	};

protected:
	Module* m_module;

	sl::List <Import> m_importList;
	sl::StringHashTable <bool> m_ignoredImportSet;
	sl::StringHashTable <bool> m_importFilePathMap;

public:
	sl::BoxList <sl::String> m_importDirList;

public:
	ImportMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	sl::ConstList <Import>
	getImportList ()
	{
		return m_importList;
	}

	bool
	addImport (const sl::StringRef& fileName);

	void
	addImport (
		ExtensionLib* lib,
		const sl::StringRef& filePath,
		const sl::StringRef& source
		);

	void
	addIgnoredImport (const sl::StringRef& fileName)
	{
		m_ignoredImportSet.visit (fileName);
	}

protected:
	FindResult
	findImportFile (
		const sl::StringRef& fileName,
		sl::String* filePath
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
