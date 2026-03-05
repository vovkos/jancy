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

#include "jnc_ct_Unit.h"

namespace jnc {
namespace ct {

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
