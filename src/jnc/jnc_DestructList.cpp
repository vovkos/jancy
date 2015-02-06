#include "pch.h"
#include "jnc_DestructList.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void 
DestructList::addDestructor (
	Function* destructor,
	const Value& argValue,
	Variable* flagVariable
	)
{
	Entry* entry = AXL_MEM_NEW (Entry);
	entry->m_destructor = destructor;
	entry->m_argValue = argValue;
	entry->m_flagVariable = flagVariable;

	m_list.insertTail (entry);
}

void
DestructList::runDestructors ()
{
	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "process destruct list");

	rtl::Iterator <Entry> entryIt = m_list.getTail ();
	for (; entryIt; entryIt--)
	{
		Entry* entry = *entryIt;
		if (!entry->m_flagVariable) // unconditional destructor
		{
			m_module->m_llvmIrBuilder.createCall (
				entry->m_destructor, 
				entry->m_destructor->getType (), 
				&entry->m_argValue, 
				entry->m_argValue ? 1 : 0,
				NULL
				);
			continue;
		}

		BasicBlock* destructBlock = m_module->m_controlFlowMgr.createBlock ("destruct_block");
		BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock ("follow_block");

		Value cmpValue;

		m_module->m_operatorMgr.binaryOperator (
			BinOpKind_Ne, 
			entry->m_flagVariable, 
			entry->m_flagVariable->getType ()->getZeroValue (),
			&cmpValue
			);

		m_module->m_controlFlowMgr.conditionalJump (cmpValue, destructBlock, followBlock);

		m_module->m_llvmIrBuilder.createCall (
			entry->m_destructor, 
			entry->m_destructor->getType (), 
			&entry->m_argValue, 
			entry->m_argValue ? 1 : 0,
			NULL
			);

		m_module->m_controlFlowMgr.follow (followBlock);
	}
}

//.............................................................................

} // namespace jnc {
