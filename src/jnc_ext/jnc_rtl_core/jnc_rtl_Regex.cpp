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
#include "jnc_rtl_Regex.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegexState,
	"jnc.RegexState",
	sl::g_nullGuid,
	-1,
	RegexState,
	&RegexState::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexState)
	JNC_MAP_CONSTRUCTOR(&RegexState::construct)
	JNC_MAP_AUTOGET_PROPERTY("m_matchLengthLimit", &RegexState::setMatchLengthLimit)
	JNC_MAP_AUTOGET_PROPERTY("m_currentOffset", &RegexState::setCurrentOffset)
	JNC_MAP_FUNCTION("reset", &RegexState::reset)
	JNC_MAP_FUNCTION("exec", &RegexState::exec)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegexDfa,
	"jnc.RegexDfa",
	sl::g_nullGuid,
	-1,
	RegexDfa,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexDfa)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<RegexDfa>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexDfa>)
	JNC_MAP_FUNCTION("clear", &RegexDfa::clear)
	JNC_MAP_FUNCTION("incrementalCompile", &RegexDfa::incrementalCompile)
	JNC_MAP_FUNCTION("finalize", &RegexDfa::finalize)
	JNC_MAP_FUNCTION("match", &RegexDfa::match)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
RegexState::construct(uint_t flags)
{
	m_flags = flags;
	m_lastAcceptStateId = -1;
	m_matchLengthLimit = 256;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	m_matchBufferPtr = gcHeap->allocateBuffer(m_matchLengthLimit);
}

void
JNC_CDECL
RegexState::markOpaqueGcRoots(GcHeap* gcHeap)
{
	if (m_matchBufferPtr.m_validator)
	{
		gcHeap->weakMark(m_matchBufferPtr.m_validator->m_validatorBox);
		gcHeap->markData(m_matchBufferPtr.m_validator->m_targetBox);
	}

	if (m_groupOffsetArrayPtr.m_validator)
	{
		gcHeap->weakMark(m_groupOffsetArrayPtr.m_validator->m_validatorBox);
		gcHeap->markData(m_groupOffsetArrayPtr.m_validator->m_targetBox);
	}
}

void
JNC_CDECL
RegexState::setMatchLengthLimit(size_t length)
{
	if (length <= m_matchLengthLimit)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	DataPtr ptr = gcHeap->allocateBuffer(m_matchLengthLimit);
	if (m_matchLength)
		memcpy(ptr.m_p, m_matchBufferPtr.m_p, m_matchLength);

	m_matchBufferPtr = ptr;
	m_matchLengthLimit = length;
}

void
JNC_CDECL
RegexState::setCurrentOffset(size_t offset)
{
	if (offset == m_currentOffset)
		return;

	intptr_t delta = offset - m_currentOffset;
	m_currentOffset = offset;
	m_matchOffset += delta;
}

void
JNC_CDECL
RegexState::reset()
{
	m_currentOffset = 0;
	m_consumedLength = 0;
	m_subMatchArrayPtr = g_nullDataPtr;
	m_subMatchCount = 0;

	memset(&m_match, 0, sizeof(RegexMatch));

	m_dfa = NULL;
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptMatchLength = 0;
	m_matchOffset = 0;
	m_matchLength = 0;
	m_replayBufferOffset = 0;
	m_replayLength = 0;
	m_groupOffsetArrayPtr = g_nullDataPtr;
	m_groupCount = 0;
	m_maxSubMatchCount = 0;
}

size_t
JNC_CDECL
RegexState::exec(
	ct::Dfa* dfa,
	DataPtr ptr,
	size_t length
	)
{
	if (dfa != m_dfa)
		setDfa(dfa);

	if (length == -1)
		length = strLen(ptr);

	size_t prevOffset = m_currentOffset;

	size_t result;

	if (m_replayLength) // replay is only available after a match
	{
		ASSERT(m_flags & Flag_Incremental);

		size_t replayBufferOffset = m_replayBufferOffset;
		size_t replayLength = m_replayLength;

		m_replayBufferOffset = 0;
		m_replayLength = 0;

		result = writeData((uchar_t*)m_matchBufferPtr.m_p + replayBufferOffset, replayLength);
		switch (result)
		{
		case RegexResult_Continue:
			break;

		case RegexResult_Error:
			m_currentOffset = m_matchOffset; // rewind to the very beginning
			m_replayBufferOffset = 0;
			m_replayLength = 0;
			softReset();

			m_consumedLength = 0;
			return RegexResult_Error;

		default: // match
			size_t consumedLength = m_currentOffset + m_replayLength - prevOffset;
			if ((intptr_t)consumedLength < 0)
				consumedLength = 0;

			if (consumedLength < replayLength)
			{
				size_t extraLength = replayLength - consumedLength;

				memmove(
					(uchar_t*)m_matchBufferPtr.m_p + m_replayBufferOffset + m_replayLength,
					(uchar_t*)m_matchBufferPtr.m_p + replayBufferOffset + consumedLength,
					extraLength
					);

				m_replayLength += extraLength;
			}

			m_consumedLength = 0;
			return result;
		}

		ASSERT(!m_replayLength);
		prevOffset = m_currentOffset;
	}

	if (!length)
	{
		result = eof();
	}
	else
	{
		result = writeData((uchar_t*)ptr.m_p, length);
		if (result == RegexResult_Continue && !(m_flags & Flag_Incremental))
			result = eof();
	}

	if (result == RegexResult_Error)
	{
		m_currentOffset = m_matchOffset; // rewind to the very beginning (not prevOffset!)
		m_replayBufferOffset = 0;
		m_replayLength = 0;
		softReset();
	}
	else if (!(m_flags & Flag_Incremental))
	{
		m_replayBufferOffset = 0;
		m_replayLength = 0;
	}

	m_consumedLength = m_currentOffset + m_replayLength - prevOffset;
	if ((intptr_t)m_consumedLength < 0)
		m_consumedLength = 0;

	return result;
}

void
RegexState::setDfa(ct::Dfa* dfa)
{
	m_dfa = dfa;
	m_groupCount = dfa->getGroupCount();
	m_maxSubMatchCount = dfa->getMaxSubMatchCount();

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	Module* module = gcHeap->getRuntime()->getModule();
	Type* matchType = module->m_typeMgr.getStdType(StdType_RegexMatch);
	Type* sizeType = module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);

	m_groupOffsetArrayPtr = m_groupCount ? gcHeap->allocateArray(sizeType, m_groupCount * 2) : g_nullDataPtr;
	m_subMatchArrayPtr = m_maxSubMatchCount ? gcHeap->allocateArray(matchType, m_maxSubMatchCount) : g_nullDataPtr;

	softReset();
}

void
RegexState::processGroupSet(ct::DfaGroupSet* groupSet)
{
	size_t* offsetArray = (size_t*)m_groupOffsetArrayPtr.m_p;

	size_t count = groupSet->m_openArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_openArray[i];
		ASSERT(groupId < m_groupCount);

		size_t offset = m_currentOffset - m_matchOffset;
		offsetArray[groupId * 2] = offset;
		offsetArray[groupId * 2 + 1] = offset;
	}

	count = groupSet->m_closeArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_closeArray[i];
		ASSERT(groupId < m_groupCount);

		offsetArray[groupId * 2 + 1] = m_currentOffset - m_matchOffset;
	}
}

void
RegexState::softReset()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptMatchLength = 0;
	m_matchOffset = m_currentOffset;
	m_matchLength = 0;

	memset(m_groupOffsetArrayPtr.m_p, -1, m_groupCount * sizeof(size_t)* 2);

	if (m_dfa)
	{
		ct::DfaStateInfo* state = m_dfa->getStateInfo(0);
		if (state->m_groupSet)
			processGroupSet(state->m_groupSet);
	}
}

size_t
RegexState::eof()
{
	if (!m_matchLength) // just matched
		return RegexResult_Continue;

	if (m_lastAcceptStateId == -1)
		return RegexResult_Error;

	if (m_lastAcceptMatchLength == m_matchLength)
	{
		size_t result = m_lastAcceptStateId;
		match(m_lastAcceptStateId);
		return result;
	}

	ASSERT(m_lastAcceptMatchLength < m_matchLength);

	if (!(m_flags & Flag_Lexer))
		return RegexResult_Error;

	size_t result = m_lastAcceptStateId;
	rollback();
	return result;
}

size_t
RegexState::writeData(
	uchar_t* p,
	size_t length
	)
{
	uchar_t* end = p + length;

	while (p < end)
	{
		uchar_t c = *p++;

		// while it might seem counter-intuitive to increment offset first,
		// it leads to cleaner logic in regex switch actions (offset points PAST lexeme)

		m_currentOffset++;

		size_t result = writeChar(c);
		if (result != RegexResult_Continue)
			return result;
	}

	return RegexResult_Continue;
}

size_t
RegexState::writeChar(uchar_t c)
{
	((uchar_t*)m_matchBufferPtr.m_p) [m_matchLength++] = c;
	if (m_matchLength >= m_matchLengthLimit)
		return RegexResult_Error;

	uintptr_t targetStateId = m_dfa->getTransition(m_stateId, c);
	if (targetStateId != -1)
		return gotoState(targetStateId);

	if (m_lastAcceptStateId == -1 || !(m_flags & Flag_Lexer))
		return RegexResult_Error;

	size_t result = m_lastAcceptStateId;
	rollback();
	return result;
}

size_t
RegexState::gotoState(size_t stateId)
{
	m_stateId = stateId;

	ct::DfaStateInfo* state = m_dfa->getStateInfo(stateId);
	if (state->m_groupSet)
		processGroupSet(state->m_groupSet);

	if (!(state->m_flags & ct::DfaStateFlag_Accept))
		return RegexResult_Continue;

	if ((state->m_flags & ct::DfaStateFlag_Final) && (m_flags & Flag_Lexer))
	{
		match(stateId);
		return stateId;
	}

	m_lastAcceptStateId = stateId;
	m_lastAcceptMatchLength = m_matchLength;
	return RegexResult_Continue;
}

void
RegexState::match(size_t stateId)
{
	ASSERT((intptr_t)stateId >= 0);

	size_t replayBufferOffset = m_matchLength;

	// create null-terminated match and sub-matches
	// save last char and ensure null-termination for the whole lexeme

	m_match.m_textPtr = jnc::strDup((char*)m_matchBufferPtr.m_p, m_matchLength);
	m_match.m_offset = m_matchOffset;
	m_match.m_length = m_matchLength;

	ct::DfaStateInfo* state = m_dfa->getStateInfo(stateId);
	ASSERT(state->m_acceptInfo);

	if (state->m_groupSet)
		processGroupSet(state->m_groupSet);

	RegexMatch* subMatchArray = (RegexMatch*)m_subMatchArrayPtr.m_p;
	size_t* offsetArray = (size_t*)m_groupOffsetArrayPtr.m_p;
	m_subMatchCount = state->m_acceptInfo->m_groupCount;

	size_t j = state->m_acceptInfo->m_firstGroupId * 2;
	for (size_t i = 0; i < state->m_acceptInfo->m_groupCount; i++)
	{
		RegexMatch* subLexeme = &subMatchArray[i];

		size_t offset = offsetArray[j++];
		size_t length = offsetArray[j++] - offset;
		if (offset != -1 && length != 0)
		{
			ASSERT(offset + length <= m_matchLength);
			subLexeme->m_textPtr = jnc::strDup((char*)m_matchBufferPtr.m_p + offset, length);
			subLexeme->m_offset = offset;
			subLexeme->m_length = length;
		}
		else // enforce empty sub-lexeme
		{
			subLexeme->m_textPtr = g_nullDataPtr;
			subLexeme->m_offset = -1;
			subLexeme->m_length = 0;
		}
	}

	softReset();

	m_replayBufferOffset = replayBufferOffset;
}

void
RegexState::rollback()
{
	ASSERT(
		(m_flags & Flag_Lexer) &&
		m_lastAcceptStateId != -1 &&
		m_lastAcceptMatchLength);

	size_t replayLength = m_matchLength - m_lastAcceptMatchLength;
	m_currentOffset = m_matchOffset + m_lastAcceptMatchLength;
	m_matchLength = m_lastAcceptMatchLength;

	// rollback groups

	size_t* offsetArray = (size_t*)m_groupOffsetArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t j = i * 2;
		if (offsetArray[j] == -1)
			continue;

		if (offsetArray[j] >= m_matchLength)
		{
			offsetArray[j] = -1;
			offsetArray[j + 1] = -1;
		}
		else if (offsetArray[j + 1] > m_matchLength)
		{
			ASSERT(offsetArray[j] < m_matchLength);
			offsetArray[j + 1] = m_matchLength;
		}
	}

	match(m_lastAcceptStateId);

	m_replayLength = replayLength;
}

//..............................................................................

void
JNC_CDECL
RegexDfa::clear()
{
	m_regex.clear();
	m_acceptContextList.clear();
	m_dfa.clear();
}

bool
JNC_CDECL
RegexDfa::incrementalCompile(
	DataPtr regexStringPtr,
	size_t length
	)
{
	if (!m_dfa.isEmpty())
	{
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	if (length == -1)
		length = jnc::strLen(regexStringPtr);

	ct::ReSwitchAcceptContext* context = AXL_MEM_NEW(ct::ReSwitchAcceptContext);
	context->m_firstGroupId = m_regex.getGroupCount();
	context->m_groupCount = 0;
	context->m_actionIdx = m_acceptContextList.getCount();
	m_acceptContextList.insertTail(context);

	re::RegexCompiler compiler(&m_regex);
	return compiler.incrementalCompile(sl::StringRef((const char*) regexStringPtr.m_p, length), context);
}

bool
JNC_CDECL
RegexDfa::finalize()
{
	if (!m_dfa.isEmpty())
	{
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	re::RegexCompiler regexCompiler(&m_regex);
	regexCompiler.finalize();

	sl::Iterator<ct::ReSwitchAcceptContext> prev = m_acceptContextList.getHead();
	sl::Iterator<ct::ReSwitchAcceptContext> next = prev.getNext();
	while (next)
	{
		prev->m_groupCount = next->m_firstGroupId - prev->m_firstGroupId;
		prev = next++;
	}

	if (!prev)
	{
		err::setError("empty regular expression");
		return false;
	}

	prev->m_groupCount = m_regex.getGroupCount() - prev->m_firstGroupId;

	return m_dfa.build(&m_regex);
}

size_t
JNC_CDECL
RegexDfa::match(
	RegexState* state,
	DataPtr ptr,
	size_t length
	)
{
	size_t stateId = state->exec(&m_dfa, ptr, length);
	if ((intptr_t)stateId <= 0)
	{
		err::setError("regular expression mismatch");
		return -1;
	}

	sl::Array<re::DfaState*> stateArray = m_regex.getDfaStateArray();
	re::DfaState* dfaState = stateArray[stateId];

	ASSERT(dfaState->m_isAccept);
	ct::ReSwitchAcceptContext* context = (ct::ReSwitchAcceptContext*)dfaState->m_acceptContext;
	return context->m_actionIdx;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
