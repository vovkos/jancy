#include "pch.h"
#include "jnc_DestructList.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CDestructList::CDestructList ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);
}

void 
CDestructList::AddDestructor (
	CFunction* pDestructor,
	const CValue& ArgValue,
	CVariable* pFlagVariable
	)
{
	TEntry* pEntry = AXL_MEM_NEW (TEntry);
	pEntry->m_pDestructor = pDestructor;
	pEntry->m_ArgValue = ArgValue;
	pEntry->m_pFlagVariable = pFlagVariable;

	m_List.InsertTail (pEntry);
}

void
CDestructList::RunDestructors ()
{
	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "process destruct list");

	rtl::CIteratorT <TEntry> Entry = m_List.GetHead ();
	for (; Entry; Entry++)
	{
		TEntry* pEntry = *Entry;
		if (!pEntry->m_pFlagVariable) // unconditional destructor
		{
			m_pModule->m_LlvmIrBuilder.CreateCall (
				pEntry->m_pDestructor, 
				pEntry->m_pDestructor->GetType (), 
				&pEntry->m_ArgValue, 
				pEntry->m_ArgValue ? 1 : 0,
				NULL
				);
			continue;
		}

		CBasicBlock* pDestructBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("destruct_block");
		CBasicBlock* pFollowBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("follow_block");

		CValue CmpValue;

		m_pModule->m_OperatorMgr.BinaryOperator (
			EBinOp_Ne, 
			pEntry->m_pFlagVariable, 
			pEntry->m_pFlagVariable->GetType ()->GetZeroValue (),
			&CmpValue
			);

		m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pDestructBlock, pFollowBlock);

		m_pModule->m_LlvmIrBuilder.CreateCall (
			pEntry->m_pDestructor, 
			pEntry->m_pDestructor->GetType (), 
			&pEntry->m_ArgValue, 
			pEntry->m_ArgValue ? 1 : 0,
			NULL
			);

		m_pModule->m_ControlFlowMgr.Follow (pFollowBlock);
	}
}

//.............................................................................

} // namespace jnc {
