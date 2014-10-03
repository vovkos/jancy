#include "pch.h"
#include "jnc_UnitMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

UnitMgr::UnitMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	m_currentUnit = NULL;
}

void
UnitMgr::clear ()
{
	m_unitList.clear ();
	m_currentUnit = NULL;
}

Unit*
UnitMgr::setCurrentUnit (Unit* unit)
{
	ASSERT (unit);

	Unit* prevUnit = m_currentUnit;
	m_currentUnit = unit;
	return prevUnit;
}

Unit*
UnitMgr::createUnit (const rtl::String& filePath)
{
	Unit* unit = AXL_MEM_NEW (Unit);

	unit->m_filePath = filePath;
	unit->m_fileName = io::getFileName (filePath);
	unit->m_dir = io::getDir  (filePath);

	if (m_module->getFlags () & ModuleFlag_DebugInfo)
		unit->m_llvmDiFile = m_module->m_llvmDiBuilder.createFile (unit->m_fileName, unit->m_dir);

	m_unitList.insertTail (unit);
	m_currentUnit = unit;

	return unit;
}

//.............................................................................

} // namespace jnc {
