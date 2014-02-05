#include "pch.h"
#include "jnc_AttributeMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CAttributeMgr::CAttributeMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);
}

CAttributeBlock*
CAttributeMgr::CreateAttributeBlock ()
{
	CAttributeBlock* pAttributeBlock = AXL_MEM_NEW (CAttributeBlock);
	pAttributeBlock->m_pModule = m_pModule;
	m_AttributeBlockList.InsertTail (pAttributeBlock);
	return pAttributeBlock;
}

//.............................................................................

} // namespace jnc {
