// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_AttributeMgr.h"

namespace jnc {
namespace ct {

class Module;

//.............................................................................

class Unit: public sl::ListLink
{
	friend class UnitMgr;

protected:
	Module* m_module;

	sl::String m_filePath;
	sl::String m_fileName;
	sl::String m_dir;

	llvm::DIFile m_llvmDiFile;

public:
	Module*
	getModule ()
	{
		return m_module;
	}

	sl::String
	getFilePath ()
	{
		return m_filePath;
	}

	sl::String
	getFileName ()
	{
		return m_fileName;
	}

	sl::String
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
	sl::StdList <Unit> m_unitList;
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
	createUnit (const sl::String& filePath);
};

//.............................................................................

} // namespace ct
} // namespace jnc
