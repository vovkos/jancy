// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class Module;

//.............................................................................

enum ImportKind
{
	ImportKind_File,
	ImportKind_Source
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Import: sl::ListLink
{
	ImportKind m_importKind;
	sl::String m_filePath;
	sl::StringRef m_source;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	addImport (const char* fileName);

	void
	addSource (
		const sl::String& filePath,
		const sl::StringRef& source
		);

protected:
	FindResult
	findImportFile (
		const char* fileName,
		sl::String* filePath
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
