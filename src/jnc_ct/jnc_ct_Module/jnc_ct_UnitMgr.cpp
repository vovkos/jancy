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

bool
Unit::setConstructor (Function* function)
{
	if (!function->getType ()->getArgArray ().isEmpty ())
	{
		err::setFormatStringError ("unit 'construct' cannot have arguments");
		return false;
	}

	if (m_constructor)
	{
		err::setFormatStringError ("unit already has 'construct' method");
		return false;
	}

	function->m_functionKind = FunctionKind_StaticConstructor;
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.construct";
	m_constructor = function;
	return true;
}

bool
Unit::setDestructor (Function* function)
{
	ASSERT (function->getType ()->getArgArray ().isEmpty ());

	if (m_destructor)
	{
		err::setFormatStringError ("unit already has 'destruct' method");
		return false;
	}

	function->m_functionKind = FunctionKind_StaticDestructor;
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.destruct";
	m_destructor = function;
	return true;
}

//..............................................................................

UnitMgr::UnitMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_currentUnit = NULL;
	m_coreLibUnit = NULL;
}

void
UnitMgr::clear ()
{
	m_unitList.clear ();
	m_currentUnit = NULL;
	m_coreLibUnit = NULL;
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
UnitMgr::getCoreLibUnit ()
{
	if (!m_coreLibUnit)
		m_coreLibUnit = createUnit (jnc_CoreLib_getLib (), "jnc_rtl.jnc");

	return m_coreLibUnit;
}

Unit*
UnitMgr::createUnit (
	ExtensionLib* lib,
	const sl::StringRef& filePath
	)
{
	Unit* unit = AXL_MEM_NEW (Unit);

	unit->m_lib = lib;
	unit->m_filePath = filePath;
	unit->m_fileName = io::getFileName (filePath);
	unit->m_dir = io::getDir  (filePath);

	if (m_module->getCompileFlags () & ModuleCompileFlag_DebugInfo)
		unit->m_llvmDiFile = m_module->m_llvmDiBuilder.createFile (unit->m_fileName, unit->m_dir);

	m_unitList.insertTail (unit);
	m_currentUnit = unit;

	return unit;
}

//..............................................................................

} // namespace ct
} // namespace jnc
