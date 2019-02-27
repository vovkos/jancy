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
#include "jnc_ct_RegexMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_rtl_Regex.h"

namespace jnc {
namespace ct {

//.............................................................................

Dfa::Dfa()
{
	m_stateCount = 0;
	m_groupCount = 0;
	m_maxSubMatchCount = 0;
}

void
Dfa::clear()
{
	m_stateCount = 0;
	m_groupCount = 0;
	m_maxSubMatchCount = 0;
	m_transitionTable.clear();
	m_stateInfoTable.clear();
	m_acceptInfoList.clear();
	m_groupSetList.clear();
}

bool
Dfa::build(fsm::Regex* regex)
{
	sl::Array<fsm::DfaState*> stateArray = regex->getDfaStateArray();
	m_stateCount = stateArray.getCount();
	m_groupCount = regex->getGroupCount();
	m_maxSubMatchCount = 0;

	m_stateInfoTable.setCount(m_stateCount);
	m_transitionTable.setCount(m_stateCount * 256);
	memset(m_stateInfoTable, 0, m_stateCount * sizeof(DfaStateInfo));
	memset(m_transitionTable, -1, m_stateCount * 256 * sizeof(uintptr_t));

	DfaStateInfo* stateInfo = m_stateInfoTable;
	uintptr_t* transitionRow = m_transitionTable;

	for (size_t i = 0; i < m_stateCount; i++)
	{
		fsm::DfaState* state = stateArray[i];
		if (state->m_isAccept)
		{
			ReSwitchAcceptContext* context = (ReSwitchAcceptContext*)state->m_acceptContext;
			ASSERT(context->m_firstGroupId + context->m_groupCount <= m_groupCount);

			DfaAcceptInfo* acceptInfo = AXL_MEM_NEW(DfaAcceptInfo);
			acceptInfo->m_firstGroupId = context->m_firstGroupId;
			acceptInfo->m_groupCount = context->m_groupCount;
			m_acceptInfoList.insertTail(acceptInfo);

			if (context->m_groupCount > m_maxSubMatchCount)
				m_maxSubMatchCount = context->m_groupCount;

			stateInfo->m_acceptInfo = acceptInfo;
			stateInfo->m_flags |= rtl::RegexState::StateFlag_Accept;

			if (state->m_transitionList.isEmpty())
				stateInfo->m_flags |= rtl::RegexState::StateFlag_Final;
		}

		size_t j = state->m_openCaptureIdSet.findBit(0);
		size_t k = state->m_closeCaptureIdSet.findBit(0);

		if (j != -1 || k != -1)
		{
			DfaGroupSet* groupSet = AXL_MEM_NEW(DfaGroupSet);

			while (j != -1)
			{
				ASSERT(j < m_groupCount);
				groupSet->m_openArray.append(j);
				j = state->m_openCaptureIdSet.findBit(j + 1);
			}

			while (k != -1)
			{
				ASSERT(k < m_groupCount);
				groupSet->m_closeArray.append(k);
				k = state->m_closeCaptureIdSet.findBit(k + 1);
			}

			m_groupSetList.insertTail(groupSet);
			stateInfo->m_groupSet = groupSet;
		}

		sl::Iterator<fsm::DfaTransition> transitionIt = state->m_transitionList.getHead();
		for (; transitionIt; transitionIt++)
		{
			fsm::DfaTransition* transition = *transitionIt;
			switch(transition->m_matchCondition.m_conditionKind)
			{
			case fsm::MatchConditionKind_Char:
				ASSERT(transition->m_matchCondition.m_char < 256);
				transitionRow[transition->m_matchCondition.m_char] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_CharSet:
				for (size_t j = 0; j < 256; j++)
					if (transition->m_matchCondition.m_charSet.getBit(j))
						transitionRow[j] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_Any:
				for (size_t j = 0; j < 256; j++)
					transitionRow[j] = transition->m_outState->m_id;
				break;
			}
		}

		stateInfo++;
		transitionRow += 256;
	}

	return true;
}

//..............................................................................

RegexMgr::RegexMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
}

Dfa*
RegexMgr::createDfa()
{
	Dfa* attributeBlock = AXL_MEM_NEW(Dfa);
	m_dfaList.insertTail(attributeBlock);
	return attributeBlock;
}

//..............................................................................

} // namespace ct
} // namespace jnc
