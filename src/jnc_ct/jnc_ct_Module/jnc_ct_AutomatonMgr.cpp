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
#include "jnc_ct_AutomatonMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_rtl_Recognizer.h"

namespace jnc {
namespace ct {

//.............................................................................

bool
Dfa::build (fsm::RegExp* regExp)
{
	sl::Array <fsm::DfaState*> stateArray = regExp->getDfaStateArray ();
	size_t stateCount = stateArray.getCount ();

	m_stateInfoTable.setCount (stateCount);
	m_transitionTable.setCount (stateCount * 256);
	memset (m_stateInfoTable, 0, stateCount * sizeof (DfaStateInfo));
	memset (m_transitionTable, -1, stateCount * 256 * sizeof (uintptr_t));

	m_stateCount = stateCount;

	DfaStateInfo* stateInfo = m_stateInfoTable;
	uintptr_t* transitionRow = m_transitionTable;

	for (size_t i = 0; i < stateCount; i++)
	{
		fsm::DfaState* state = stateArray [i];
		if (state->m_isAccept)
		{
			stateInfo->m_flags |= rtl::RecognizerStateFlag_Accept;
			if (state->m_transitionList.isEmpty ())
				stateInfo->m_flags |= rtl::RecognizerStateFlag_Final;
		}

		size_t j = state->m_openCaptureIdSet.findBit (0);
		size_t k = state->m_closeCaptureIdSet.findBit (0);

		if (j != -1 || k != -1)
		{
			DfaGroupSet* groupSet = AXL_MEM_NEW (DfaGroupSet);

			while (j != -1)
			{
				if (j >= m_groupCount)
					m_groupCount = j + 1;

				groupSet->m_openArray.append (j);
				j = state->m_openCaptureIdSet.findBit (j + 1);
			}

			while (k != -1)
			{
				groupSet->m_closeArray.append (k);
				k = state->m_closeCaptureIdSet.findBit (k + 1);
			}

			m_groupSetList.insertTail (groupSet);
			stateInfo->m_groupSet = groupSet;
		}

		sl::Iterator <fsm::DfaTransition> transitionIt = state->m_transitionList.getHead ();
		for (; transitionIt; transitionIt++)
		{
			fsm::DfaTransition* transition = *transitionIt;
			switch (transition->m_matchCondition.m_conditionKind)
			{
			case fsm::MatchConditionKind_Char:
				ASSERT (transition->m_matchCondition.m_char < 256);
				transitionRow [transition->m_matchCondition.m_char] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_CharSet:
				for (size_t j = 0; j < 256; j++)
					if (transition->m_matchCondition.m_charSet.getBit (j))
						transitionRow [j] = transition->m_outState->m_id;
				break;

			case fsm::MatchConditionKind_Any:
				for (size_t j = 0; j < 256; j++)
					transitionRow [j] = transition->m_outState->m_id;
				break;
			}
		}

		stateInfo++;
		transitionRow += 256;
	}

	return true;
}

//..............................................................................

AutomatonMgr::AutomatonMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

Dfa*
AutomatonMgr::createDfa ()
{
	Dfa* attributeBlock = AXL_MEM_NEW (Dfa);
	m_dfaList.insertTail (attributeBlock);
	return attributeBlock;
}

//..............................................................................

} // namespace ct
} // namespace jnc
