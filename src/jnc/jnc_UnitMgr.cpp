#include "pch.h"
#include "jnc_UnitMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CUnitMgr::CUnitMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_pCurrentUnit = NULL;
}

void
CUnitMgr::Clear ()
{
	m_UnitList.Clear ();
	m_pCurrentUnit = NULL;
}

CUnit*
CUnitMgr::SetCurrentUnit (CUnit* pUnit)
{
	ASSERT (pUnit);

	CUnit* pPrevUnit = m_pCurrentUnit;
	m_pCurrentUnit = pUnit;
	return pPrevUnit;
}

CUnit*
CUnitMgr::CreateUnit (const rtl::CString& FilePath)
{
	CUnit* pUnit = AXL_MEM_NEW (CUnit);

	pUnit->m_FilePath = FilePath;
	pUnit->m_FileName = io::GetFileName (FilePath);
	pUnit->m_Dir = io::GetDir  (FilePath);

	if (m_pModule->GetFlags () & EModuleFlag_DebugInfo)
		pUnit->m_LlvmDiFile = m_pModule->m_LlvmDiBuilder.CreateFile (pUnit->m_FileName, pUnit->m_Dir);

	m_UnitList.InsertTail (pUnit);
	m_pCurrentUnit = pUnit;

	return pUnit;
}

//.............................................................................

} // namespace jnc {
