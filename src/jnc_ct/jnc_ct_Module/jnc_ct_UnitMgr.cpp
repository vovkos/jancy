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

#include "pch.h"
#include "jnc_ct_UnitMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

UnitMgr::UnitMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_currentUnit = NULL;
	m_coreLibUnit = NULL;
	m_introspectionLibUnit = NULL;
}

void
UnitMgr::clear() {
	m_unitList.clear();
	m_currentUnit = NULL;
	m_coreLibUnit = NULL;
	m_introspectionLibUnit = NULL;
}

Unit*
UnitMgr::setCurrentUnit(Unit* unit) {
	ASSERT(unit);

	Unit* prevUnit = m_currentUnit;
	m_currentUnit = unit;
	return prevUnit;
}

Unit*
UnitMgr::getCoreLibUnit() {
	if (!m_coreLibUnit)
		m_coreLibUnit = createUnit(jnc_CoreLib_getLib(), "jnc_rtl_core.jnc");

	return m_coreLibUnit;
}

Unit*
UnitMgr::getIntrospectionLibUnit() {
	if (!m_introspectionLibUnit)
		m_introspectionLibUnit = createUnit(jnc_IntrospectionLib_getLib(), "jnc_rtl_intro.jnc");

	return m_introspectionLibUnit;
}

Unit*
UnitMgr::createUnit(
	ExtensionLib* lib,
	const sl::StringRef& filePath
) {
	Unit* unit = new Unit;
	unit->m_module = m_module;
	unit->m_lib = lib;
	unit->m_filePath = filePath;
	unit->m_fileName = io::getFileName(filePath);
	unit->m_dir = io::getDir(filePath);

	if (m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo)
		unit->m_llvmDiFile = m_module->m_llvmDiBuilder.createFile(
			!unit->m_fileName.isEmpty() ?
				sl::StringRef(unit->m_fileName) :
				sl::StringRef(".unnamed.jnc"),
			unit->m_dir
		);

	m_unitList.insertTail(unit);
	return unit;
}

//..............................................................................

} // namespace ct
} // namespace jnc
