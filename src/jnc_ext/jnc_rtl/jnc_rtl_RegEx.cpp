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
#include "jnc_rtl_RegEx.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_CLASS_TYPE (
	RegExState,
	"jnc.RegExState",
	sl::g_nullGuid,
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (RegExState)
	JNC_MAP_CONSTRUCTOR (&RegExState::construct)
	JNC_MAP_AUTOGET_PROPERTY ("m_matchLengthLimit", &RegExState::setMatchLengthLimit)
	JNC_MAP_AUTOGET_PROPERTY ("m_currentOffset", &RegExState::setCurrentOffset)
	JNC_MAP_FUNCTION ("reset", &RegExState::reset)
	JNC_MAP_FUNCTION ("exec", &RegExState::exec)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
JNC_CDECL
RegExState::construct (bool isIncremental)
{
	m_isIncremental = isIncremental;
	m_matchSavedChar = 0;

	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_matchLengthLimit = 256;
	m_currentOffset = 0;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	m_match.m_textPtr = gcHeap->allocateBuffer (m_matchLengthLimit);
}

void
JNC_CDECL
RegExState::setMatchLengthLimit (size_t length)
{
	if (length <= m_matchLengthLimit)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr ptr = gcHeap->allocateBuffer (m_matchLengthLimit);
	if (m_match.m_length)
		memcpy (ptr.m_p, m_match.m_textPtr.m_p, m_match.m_length);

	m_match.m_textPtr = ptr;
	m_matchLengthLimit = length;
}

void
JNC_CDECL
RegExState::setCurrentOffset (size_t offset)
{
	if (offset == m_currentOffset)
		return;

	intptr_t delta = offset - m_currentOffset;
	m_currentOffset = offset;
	m_match.m_offset += delta;
}

void
JNC_CDECL
RegExState::reset ()
{
	softReset ();

	m_currentOffset = 0;
	m_match.m_offset = 0;
}

size_t
JNC_CDECL
RegExState::exec (
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

	if (!length)
	{
		result = eof ();
	}
	else
	{	
		result = writeData ((uchar_t*) ptr.m_p, length);
		if (result == RegExResult_Continue && !m_isIncremental)
			result = eof ();
	}

	if (result == RegExResult_Error)
	{
		m_currentOffset = m_match.m_offset; // rollback offset
		softReset ();
	}

	return result; 
}

void
RegExState::setDfa (ct::Dfa* dfa)
{
	m_dfa = dfa;
	m_groupCount = dfa->getGroupCount ();
	m_maxSubMatchCount = dfa->getMaxSubMatchCount ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* matchType = module->m_typeMgr.getStdType (StdType_RegExMatch);
	Type* sizeType = module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_groupOffsetArrayPtr = m_groupCount ? gcHeap->allocateArray (sizeType, m_groupCount * 2) : g_nullPtr;
	m_subMatchArrayPtr = m_maxSubMatchCount ? gcHeap->allocateArray (matchType, m_maxSubMatchCount) : g_nullPtr;

	softReset ();
}

void
RegExState::processGroupSet (ct::DfaGroupSet* groupSet)
{
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;

	size_t count = groupSet->m_openArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_openArray [i];
		ASSERT (groupId < m_groupCount);

		size_t offset = m_currentOffset - m_match.m_offset;
		offsetArray [groupId * 2] = offset;
		offsetArray [groupId * 2 + 1] = offset;
	}

	count = groupSet->m_closeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_closeArray [i];
		ASSERT (groupId < m_groupCount);

		offsetArray [groupId * 2 + 1] = m_currentOffset - m_match.m_offset;
	}
}

void
RegExState::softReset ()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptMatchLength = 0;
	m_match.m_offset = m_currentOffset;
	m_match.m_length = 0;
	m_subMatchCount = 0;

	memset (m_groupOffsetArrayPtr.m_p, -1, m_groupCount * sizeof (size_t) * 2);
	memset (m_subMatchArrayPtr.m_p, 0, m_maxSubMatchCount * sizeof (RegExMatch));

	if (m_dfa)
	{
		ct::DfaStateInfo* state = m_dfa->getStateInfo (0);
		if (state->m_groupSet)
			processGroupSet (state->m_groupSet);
	}
}

size_t
RegExState::eof ()
{
	for (;;)
	{
		if (!m_match.m_length) // just matched
			return RegExResult_Continue;

		if (m_lastAcceptStateId == -1)
			return RegExResult_Error;

		if (m_lastAcceptMatchLength >= m_match.m_length)
			return match (m_lastAcceptStateId);

		size_t result = rollback ();
		if (result != RegExResult_Continue)
			return result;
	}
}

size_t
RegExState::writeData (
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
		if (result != RegExResult_Continue)
			return result;
	}

	return RegExResult_Continue;
}

size_t
RegExState::writeChar (uint_t c)
{
	if (c < 256) // not a pseudo-char
	{
		((uchar_t*) m_match.m_textPtr.m_p) [m_match.m_length++] = c;
		if (m_match.m_length >= m_matchLengthLimit)
			return RegExResult_Error;
	}

	uintptr_t targetStateId = m_dfa->getTransition (m_stateId, c);
	return
		targetStateId != -1 ? gotoState (targetStateId) : 
		m_lastAcceptStateId != -1 ? rollback () :
		RegExResult_Error;
}

size_t
RegExState::gotoState (size_t stateId)
{
	m_stateId = stateId;

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	if (!(state->m_flags & StateFlag_Accept))
		return RegExResult_Continue;

	if (state->m_flags & StateFlag_Final)
		return match (stateId);

	m_lastAcceptStateId = stateId;
	m_lastAcceptMatchLength = m_match.m_length;
	return RegExResult_Continue;
}

size_t
RegExState::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptMatchLength);

	uchar_t* chunk = (uchar_t*) m_match.m_textPtr.m_p + m_lastAcceptMatchLength;
	size_t chunkLength = m_match.m_length - m_lastAcceptMatchLength;

	m_currentOffset = m_match.m_offset + m_lastAcceptMatchLength;
	m_match.m_length = m_lastAcceptMatchLength;

	// rollback groups

	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t j = i * 2;
		if (offsetArray [j] == -1)
			continue;

		if (offsetArray [j] >= m_match.m_length)
		{
			offsetArray [j] = -1;
			offsetArray [j + 1] = -1;
		}
		else if (offsetArray [j + 1] > m_match.m_length)
		{
			ASSERT (offsetArray [j] < m_match.m_length);
			offsetArray [j + 1] = m_match.m_length;
		}
	}

	size_t result = match (m_lastAcceptStateId);
	return
		result != RegExResult_Continue ? result :
		chunkLength ? writeData (chunk, chunkLength) :
		RegExResult_Continue;
}

size_t
RegExState::match (size_t stateId)
{
	// save last char and ensure null-termination for the whole lexeme

	m_matchSavedChar = ((uchar_t*) m_match.m_textPtr.m_p) [m_match.m_length];

	((uchar_t*) m_match.m_textPtr.m_p) [m_match.m_length] = 0;

	// create null-terminated sub-lexemes

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	ASSERT (state->m_acceptInfo);

	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	RegExMatch* subMatchArray = (RegExMatch*) m_subMatchArrayPtr.m_p;
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	m_subMatchCount = state->m_acceptInfo->m_groupCount;

	size_t j = state->m_acceptInfo->m_firstGroupId * 2;
	for (size_t i = 0; i < state->m_acceptInfo->m_groupCount; i++)
	{
		RegExMatch* subLexeme = &subMatchArray [i];

		size_t offset = offsetArray [j++];
		size_t length = offsetArray [j++] - offset;
		if (offset != -1 && length != 0)
		{
			ASSERT (offset + length <= m_match.m_length);
			subLexeme->m_textPtr = jnc::strDup ((char*) m_match.m_textPtr.m_p + offset, length);
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

	ASSERT ((intptr_t) stateId >= 0);
	return stateId;
}

void
JNC_CDECL
RegExState::postMatch ()
{
	// that's not a mistake below! even though we immediatly reset after that, we DO need to
	// restore the lexeme buffer -- in case `match` is called from `rollback`

	((uchar_t*) m_match.m_textPtr.m_p) [m_match.m_length] = m_matchSavedChar;
	softReset ();
}

//..............................................................................

} // namespace rtl
} // namespace jnc
