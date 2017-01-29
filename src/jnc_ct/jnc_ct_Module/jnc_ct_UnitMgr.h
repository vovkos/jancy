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

#include "jnc_ct_AttributeMgr.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

class Unit: public sl::ListLink
{
	friend class UnitMgr;

protected:
	Module* m_module;

	ExtensionLib* m_lib;
	sl::String m_filePath;
	sl::String m_fileName;
	sl::String m_dir;

	llvm::DIFile m_llvmDiFile;

	Function* m_constructor;
	Function* m_destructor;

public:
	Module*
	getModule ()
	{
		return m_module;
	}

	ExtensionLib*
	getLib ()
	{
		return m_lib;
	}

	const sl::String&
	getFilePath ()
	{
		return m_filePath;
	}

	const sl::String&
	getFileName ()
	{
		return m_fileName;
	}

	const sl::String&
	getDir ()
	{
		return m_dir;
	}

	llvm::DIFile
	getLlvmDiFile ()
	{
		return m_llvmDiFile;
	}

	Function*
	getConstructor ()
	{
		return m_constructor;
	}

	bool
	setConstructor (Function* function);

	Function*
	getDestructor ()
	{
		return m_destructor;
	}

	bool
	setDestructor (Function* function);
};

//..............................................................................

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

	sl::ConstList <Unit>
	getUnitList ()
	{
		return m_unitList;
	}

	Unit*
	getCurrentUnit ()
	{
		return m_currentUnit;
	}

	Unit*
	setCurrentUnit (Unit* unit);

	Unit*
	createUnit (
		ExtensionLib* lib,
		const sl::StringRef& filePath
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
