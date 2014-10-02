// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_AttributeMgr.h"

namespace jnc {

class Module;

//.............................................................................

class Unit: public rtl::ListLink
{
	friend class UnitMgr;

protected:
	Module* m_module;

	rtl::String m_filePath;
	rtl::String m_fileName;
	rtl::String m_dir;

	llvm::DIFile m_llvmDiFile;

public:
	Module*
	getModule ()
	{
		return m_module;
	}

	rtl::String
	getFilePath ()
	{
		return m_filePath;
	}

	rtl::String
	getFileName ()
	{
		return m_fileName;
	}

	rtl::String
	getDir ()
	{
		return m_dir;
	}

	llvm::DIFile
	getLlvmDiFile ()
	{
		return m_llvmDiFile;
	}
};

//.............................................................................

class UnitMgr
{
protected:
	Module* m_module;
	rtl::StdList <Unit> m_unitList;
	Unit* m_currentUnit;

public:
	UnitMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	Unit*
	getCurrentUnit ()
	{
		return m_currentUnit;
	}

	Unit*
	setCurrentUnit (Unit* unit);

	Unit*
	createUnit (const rtl::String& filePath);
};

//.............................................................................

} //namespace jnc {
