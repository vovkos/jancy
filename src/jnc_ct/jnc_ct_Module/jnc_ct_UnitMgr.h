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

#include "jnc_Def.h"

namespace jnc {
namespace ct {

//..............................................................................

class Unit: public sl::ListLink {
	friend class UnitMgr;

protected:
	Module* m_module;

	ExtensionLib* m_lib;
	sl::String m_filePath;
	sl::String m_fileName;
	sl::String m_dir;

	llvm::DIFile_vn m_llvmDiFile;

public:
	Unit() {
		m_module = NULL;
		m_lib = NULL;
	}

	inline
	bool
	isRootUnit();

	Module*
	getModule() const {
		return m_module;
	}

	ExtensionLib*
	getLib() const {
		return m_lib;
	}

	const sl::String&
	getFilePath() const {
		return m_filePath;
	}

	const sl::String&
	getFileName() const {
		return m_fileName;
	}

	const sl::String&
	getDir() const {
		return m_dir;
	}

	llvm::DIFile_vn
	getLlvmDiFile() const {
		return m_llvmDiFile;
	}
};

//..............................................................................

class UnitMgr {
protected:
	Module* m_module;
	sl::List<Unit> m_unitList;
	Unit* m_currentUnit;
	Unit* m_coreLibUnit;
	Unit* m_introspectionLibUnit;

public:
	UnitMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	const sl::List<Unit>&
	getUnitList() {
		return m_unitList;
	}

	Unit*
	getRootUnit() {
		return *m_unitList.getHead();
	}

	Unit*
	getCurrentUnit() {
		return m_currentUnit;
	}

	Unit*
	setCurrentUnit(Unit* unit);

	Unit*
	getCoreLibUnit();

	Unit*
	getIntrospectionLibUnit();

	Unit*
	createUnit(
		ExtensionLib* lib,
		const sl::StringRef& filePath
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
