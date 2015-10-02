// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {

//.............................................................................

enum ImportKind
{
	ImportKind_File,
	ImportKind_Source
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Import: rtl::ListLink
{
	ImportKind m_importKind;
	rtl::String m_filePath;
	rtl::StringSlice m_source;
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

	rtl::StdList <Import> m_importList;
	rtl::StringHashTableMap <bool> m_importFilePathMap;
	rtl::BoxList <rtl::String> m_extensionLibFilePathCache;
	
public:
	rtl::BoxList <rtl::String> m_importDirList;

public:
	ImportMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	rtl::ConstList <Import>
	getImportList ()
	{
		return m_importList;
	}

	bool
	addImport (const char* fileName);

	void
	addSource (
		const rtl::String& filePath,
		const rtl::StringSlice& source
		);

protected:
	FindResult
	findImportFile (
		const char* fileName,
		rtl::String* filePath
		);
};

//.............................................................................

} // namespace jnc
