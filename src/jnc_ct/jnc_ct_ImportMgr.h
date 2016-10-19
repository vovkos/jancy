// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

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

	sl::StdList <Import> m_importList;
	sl::StringHashTableMap <bool> m_importFilePathMap;
	sl::BoxList <sl::String> m_extensionLibFilePathCache;

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
