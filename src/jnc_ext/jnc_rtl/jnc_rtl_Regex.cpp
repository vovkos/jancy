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
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	RegexState,
	"jnc.RegexState",
	sl::g_nullGuid,
	-1,
	RegexState,
	&RegexState::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (RegexState)
	JNC_MAP_CONSTRUCTOR (&RegexState::construct)
	JNC_MAP_AUTOGET_PROPERTY ("m_matchLengthLimit", &RegexState::setMatchLengthLimit)
	JNC_MAP_AUTOGET_PROPERTY ("m_currentOffset", &RegexState::setCurrentOffset)
	JNC_MAP_FUNCTION ("reset", &RegexState::reset)
	JNC_MAP_FUNCTION ("exec", &RegexState::exec)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	RegexDfa,
	"jnc.RegexDfa",
	sl::g_nullGuid,
	-1,
	RegexDfa,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (RegexDfa)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <RegexDfa>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <RegexDfa>)
	JNC_MAP_FUNCTION ("clear", &RegexDfa::clear)
	JNC_MAP_FUNCTION ("inrementalCompile", &RegexDfa::inrementalCompile)
	JNC_MAP_FUNCTION ("finalize", &RegexDfa::finalize)
	JNC_MAP_FUNCTION ("incrementalMatch", &RegexDfa::incrementalMatch)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
JNC_CDECL
RegexState::construct (bool isIncremental)
{
	m_lastAcceptStateId = -1;
	m_isIncremental = isIncremental;
	m_matchLengthLimit = 256;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	m_matchBufferPtr = gcHeap->allocateBuffer (m_matchLengthLimit);
}

void
JNC_CDECL
RegexState::markOpaqueGcRoots (GcHeap* gcHeap)
{
	if (m_matchBufferPtr.m_validator)
	{
		gcHeap->weakMark (m_matchBufferPtr.m_validator->m_validatorBox);
		gcHeap->markData (m_matchBufferPtr.m_validator->m_targetBox);
	}

	if (m_groupOffsetArrayPtr.m_validator)
	{
		gcHeap->weakMark (m_groupOffsetArrayPtr.m_validator->m_validatorBox);
		gcHeap->markData (m_groupOffsetArrayPtr.m_validator->m_targetBox);
	}
}

void
JNC_CDECL
RegexState::setMatchLengthLimit (size_t length)
{
	if (length <= m_matchLengthLimit)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr ptr = gcHeap->allocateBuffer (m_matchLengthLimit);
	if (m_matchLength)
		memcpy (ptr.m_p, m_matchBufferPtr.m_p, m_matchLength);

	m_matchBufferPtr = ptr;
	m_matchLengthLimit = length;
}

void
JNC_CDECL
RegexState::setCurrentOffset (size_t offset)
{
	if (offset == m_currentOffset)
		return;

	intptr_t delta = offset - m_currentOffset;
	m_currentOffset = offset;
	m_matchOffset += delta;
}

void
JNC_CDECL
RegexState::reset ()
{
	softReset ();

	m_currentOffset = 0;
	m_matchOffset = 0;
	m_consumedLength = 0;
	m_subMatchCount = 0;

	memset (&m_match, 0, sizeof (RegexMatch));
	memset (m_subMatchArrayPtr.m_p, 0, m_maxSubMatchCount * sizeof (RegexMatch));
}

size_t
JNC_CDECL
RegexState::exec (
	ct::Dfa* dfa,
	DataPtr ptr,
	size_t length
	)
{
	if (dfa != m_dfa)
		setDfa (dfa);

	if (length == -1)
		length = strLen (ptr);

	size_t result;

	size_t prevOffset = m_currentOffset;

	if (!length)
	{
		result = eof ();
	}
	else
	{	
		result = writeData ((uchar_t*) ptr.m_p, length);
		if (result == RegexResult_Continue && !m_isIncremental)
			result = eof ();
	}

	if (result == RegexResult_Error)
	{
		m_currentOffset = m_matchOffset; // rollback offset to the very beginning (not prevOffset!)
		softReset ();
	}

	m_consumedLength = m_currentOffset - prevOffset;
	return result; 
}

void
RegexState::setDfa (ct::Dfa* dfa)
{
	m_dfa = dfa;
	m_groupCount = dfa->getGroupCount ();
	m_maxSubMatchCount = dfa->getMaxSubMatchCount ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* matchType = module->m_typeMgr.getStdType (StdType_RegexMatch);
	Type* sizeType = module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_groupOffsetArrayPtr = m_groupCount ? gcHeap->allocateArray (sizeType, m_groupCount * 2) : g_nullPtr;
	m_subMatchArrayPtr = m_maxSubMatchCount ? gcHeap->allocateArray (matchType, m_maxSubMatchCount) : g_nullPtr;

	softReset ();
}

void
RegexState::processGroupSet (ct::DfaGroupSet* groupSet)
{
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;

	size_t count = groupSet->m_openArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_openArray [i];
		ASSERT (groupId < m_groupCount);

		size_t offset = m_currentOffset - m_matchOffset;
		offsetArray [groupId * 2] = offset;
		offsetArray [groupId * 2 + 1] = offset;
	}

	count = groupSet->m_closeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_closeArray [i];
		ASSERT (groupId < m_groupCount);

		offsetArray [groupId * 2 + 1] = m_currentOffset - m_matchOffset;
	}
}

void
RegexState::softReset ()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptMatchLength = 0;
	m_matchOffset = m_currentOffset;
	m_matchLength = 0;

	memset (m_groupOffsetArrayPtr.m_p, -1, m_groupCount * sizeof (size_t) * 2);

	if (m_dfa)
	{
		ct::DfaStateInfo* state = m_dfa->getStateInfo (0);
		if (state->m_groupSet)
			processGroupSet (state->m_groupSet);
	}
}

size_t
RegexState::eof ()
{
	for (;;)
	{
		if (!m_matchLength) // just matched
			return RegexResult_Continue;

		if (m_lastAcceptStateId == -1)
			return RegexResult_Error;

		if (m_lastAcceptMatchLength >= m_matchLength)
			return match (m_lastAcceptStateId);

		size_t result = rollback ();
		if (result != RegexResult_Continue)
			return result;
	}
}

size_t
RegexState::writeData (
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

		size_t result = writeChar (c);
		if (result != RegexResult_Continue)
			return result;
	}

	return RegexResult_Continue;
}

size_t
RegexState::writeChar (uint_t c)
{
	if (c < 256) // not a pseudo-char
	{
		((uchar_t*) m_matchBufferPtr.m_p) [m_matchLength++] = c;
		if (m_matchLength >= m_matchLengthLimit)
			return RegexResult_Error;
	}

	uintptr_t targetStateId = m_dfa->getTransition (m_stateId, c);
	return
		targetStateId != -1 ? gotoState (targetStateId) : 
		m_lastAcceptStateId != -1 ? rollback () :
		RegexResult_Error;
}

size_t
RegexState::gotoState (size_t stateId)
{
	m_stateId = stateId;

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	if (!(state->m_flags & StateFlag_Accept))
		return RegexResult_Continue;

	if (state->m_flags & StateFlag_Final)
		return match (stateId);

	m_lastAcceptStateId = stateId;
	m_lastAcceptMatchLength = m_matchLength;
	return RegexResult_Continue;
}

size_t
RegexState::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptMatchLength);

	uchar_t* chunk = (uchar_t*) m_matchBufferPtr.m_p + m_lastAcceptMatchLength;
	size_t chunkLength = m_matchLength - m_lastAcceptMatchLength;

	m_currentOffset = m_matchOffset + m_lastAcceptMatchLength;
	m_matchLength = m_lastAcceptMatchLength;

	// rollback groups

	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t j = i * 2;
		if (offsetArray [j] == -1)
			continue;

		if (offsetArray [j] >= m_matchLength)
		{
			offsetArray [j] = -1;
			offsetArray [j + 1] = -1;
		}
		else if (offsetArray [j + 1] > m_matchLength)
		{
			ASSERT (offsetArray [j] < m_matchLength);
			offsetArray [j + 1] = m_matchLength;
		}
	}

	size_t result = match (m_lastAcceptStateId);
	return
		result != RegexResult_Continue ? result :
		chunkLength ? writeData (chunk, chunkLength) :
		RegexResult_Continue;
}

size_t
RegexState::match (size_t stateId)
{
	ASSERT ((intptr_t) stateId >= 0);

	// create null-terminated match and sub-matches
	// save last char and ensure null-termination for the whole lexeme

	m_match.m_textPtr = jnc::strDup ((char*) m_matchBufferPtr.m_p, m_matchLength);
	m_match.m_offset = m_matchOffset;
	m_match.m_length = m_matchLength;

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	ASSERT (state->m_acceptInfo);

	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	RegexMatch* subMatchArray = (RegexMatch*) m_subMatchArrayPtr.m_p;
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	m_subMatchCount = state->m_acceptInfo->m_groupCount;

	size_t j = state->m_acceptInfo->m_firstGroupId * 2;
	for (size_t i = 0; i < state->m_acceptInfo->m_groupCount; i++)
	{
		RegexMatch* subLexeme = &subMatchArray [i];

		size_t offset = offsetArray [j++];
		size_t length = offsetArray [j++] - offset;
		if (offset != -1 && length != 0)
		{
			ASSERT (offset + length <= m_matchLength);
			subLexeme->m_textPtr = jnc::strDup ((char*) m_matchBufferPtr.m_p + offset, length);
			subLexeme->m_offset = offset;
			subLexeme->m_length = length;
		}
		else // enforce empty sub-lexeme
		{
			subLexeme->m_textPtr = g_nullPtr;
			subLexeme->m_offset = -1;
			subLexeme->m_length = 0;
		}
	}

	softReset ();
	return stateId;
}

//..............................................................................

void
JNC_CDECL
RegexDfa::clear ()
{
	m_regex.clear ();
	m_acceptContextList.clear ();
	m_dfa.clear ();
}

bool 
JNC_CDECL
RegexDfa::inrementalCompile (
	DataPtr regexStringPtr,
	size_t length
	)
{
	if (length == -1)
		length = jnc::strLen (regexStringPtr);

	ct::ReSwitchAcceptContext* context = AXL_MEM_NEW (ct::ReSwitchAcceptContext);
	context->m_firstGroupId = m_regex.getGroupCount ();
	context->m_groupCount = 0;
	context->m_actionIdx = m_acceptContextList.getCount ();
	m_acceptContextList.insertTail (context);

	fsm::RegexCompiler compiler (&m_regex);
	return compiler.incrementalCompile (sl::StringRef ((const char*) regexStringPtr.m_p, length), context);
}

bool 
JNC_CDECL
RegexDfa::finalize ()
{
	fsm::RegexCompiler regexCompiler (&m_regex);
	regexCompiler.finalize ();

	return m_dfa.build (&m_regex);
}

size_t 
JNC_CDECL
RegexDfa::incrementalMatch (
	RegexState* state,
	DataPtr ptr,
	size_t length
	)
{
	size_t stateId = state->exec (&m_dfa, ptr, length);
	if ((intptr_t) stateId <= 0)
	{
		err::setError ("regular expression mismatch");
		return -1;
	}

	sl::Array <fsm::DfaState*> stateArray = m_regex.getDfaStateArray ();
	fsm::DfaState* dfaState = stateArray [stateId];
	
	ASSERT (dfaState->m_isAccept);
	ct::ReSwitchAcceptContext* context = (ct::ReSwitchAcceptContext*) dfaState->m_acceptContext; 
	return context->m_actionIdx;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
