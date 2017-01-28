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
#include "jnc_rtl_Recognizer.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_CLASS_TYPE (
	Recognizer,
	"jnc.Recognizer",
	sl::g_nullGuid,
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Recognizer)
	JNC_MAP_CONSTRUCTOR (&Recognizer::construct)
	JNC_MAP_AUTOGET_PROPERTY ("m_automatonFunc", &Recognizer::setAutomatonFunc)
	JNC_MAP_AUTOGET_PROPERTY ("m_lexemeLengthLimit", &Recognizer::setLexemeLengthLimit)
	JNC_MAP_AUTOGET_PROPERTY ("m_currentOffset", &Recognizer::setCurrentOffset)
	JNC_MAP_FUNCTION ("reset", &Recognizer::reset)
	JNC_MAP_FUNCTION ("write", &Recognizer::write)
	JNC_MAP_FUNCTION ("eof", &Recognizer::eof)
	JNC_MAP_FUNCTION ("setDfa", &Recognizer::setDfa)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
JNC_CDECL
Recognizer::construct (FunctionPtr automatonFuncPtr)
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lexemeLengthLimit = 256;
	m_currentOffset = 0;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	m_lexeme.m_textPtr = gcHeap->allocateBuffer (m_lexemeLengthLimit);
	setAutomatonFunc (automatonFuncPtr);
}

void
JNC_CDECL
Recognizer::setAutomatonFunc (FunctionPtr automatonFuncPtr)
{
	m_automatonFuncPtr = automatonFuncPtr;
	m_stateId = 0;
	m_lastAcceptStateId = -1;

	if (automatonFuncPtr.m_p)
		((AutomatonFunc*) automatonFuncPtr.m_p) (automatonFuncPtr.m_closure, this, -1);
}

void
JNC_CDECL
Recognizer::setLexemeLengthLimit (size_t length)
{
	if (length <= m_lexemeLengthLimit)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr ptr = gcHeap->allocateBuffer (m_lexemeLengthLimit);
	if (m_lexeme.m_length)
		memcpy (ptr.m_p, m_lexeme.m_textPtr.m_p, m_lexeme.m_length);

	m_lexeme.m_textPtr = ptr;
	m_lexemeLengthLimit = length;
}

void
JNC_CDECL
Recognizer::setCurrentOffset (size_t offset)
{
	if (offset == m_currentOffset)
		return;

	intptr_t delta = offset - m_currentOffset;
	m_currentOffset = offset;
	m_lexeme.m_offset += delta;
}

void
JNC_CDECL
Recognizer::setDfa (ct::Dfa* dfa)
{
	if (m_dfa == dfa)
		return;

	m_dfa = dfa;
	m_groupCount = dfa->getGroupCount ();
	m_maxSubLexemeCount = dfa->getMaxSubLexemeCount ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* lexemeType = module->m_typeMgr.getStdType (StdType_AutomatonLexeme);
	Type* sizeType = module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_groupOffsetArrayPtr = m_groupCount ? gcHeap->allocateArray (sizeType, m_groupCount * 2) : g_nullPtr;
	m_subLexemeArrayPtr = m_maxSubLexemeCount ? gcHeap->allocateArray (lexemeType, m_maxSubLexemeCount) : g_nullPtr;

	softReset ();
}

void
JNC_CDECL
Recognizer::reset ()
{
	softReset ();

	m_currentOffset = 0;
	m_lexeme.m_offset = 0;
}

void
Recognizer::softReset ()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptLexemeLength = 0;
	m_lexeme.m_offset = m_currentOffset;
	m_lexeme.m_length = 0;
	m_subLexemeCount = 0;

	memset (m_groupOffsetArrayPtr.m_p, -1, m_groupCount * sizeof (size_t) * 2);
	memset (m_subLexemeArrayPtr.m_p, 0, m_maxSubLexemeCount * sizeof (AutomatonLexeme));

	if (m_dfa)
	{
		ct::DfaStateInfo* state = m_dfa->getStateInfo (0);
		if (state->m_groupSet)
			processGroupSet (state->m_groupSet);
	}
}

bool
JNC_CDECL
Recognizer::write (
	DataPtr ptr,
	size_t length
	)
{
	if (!m_automatonFuncPtr.m_p)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	if (length == -1)
		length = strLen (ptr);

	if (!length)
		return true;

	AutomatonResult result = writeData ((uchar_t*) ptr.m_p, length);
	return result != AutomatonResult_Error;
}

bool
JNC_CDECL
Recognizer::eof ()
{
	for (;;)
	{
		if (!m_lexeme.m_length) // just matched
			return true;

		if (m_lastAcceptStateId == -1)
		{
			err::setError ("unrecognized lexeme");
			return false;
		}

		if (m_lastAcceptLexemeLength >= m_lexeme.m_length)
			return match (m_lastAcceptStateId) != AutomatonResult_Error;

		AutomatonResult result = rollback ();
		if (result != AutomatonResult_Continue)
			return result != AutomatonResult_Error;
	}
}

AutomatonResult
Recognizer::writeData (
	uchar_t* p,
	size_t length
	)
{
	AutomatonResult result;

	uchar_t* end = p + length;

	while (p < end)
	{
		uchar_t c = *p++;

		// while it might seem counter-intuitive to increment offset first,
		// it leads to cleaner logic in automaton actions (offset points PAST lexeme)

		m_currentOffset++;

		result = writeChar (c);
		if (result != AutomatonResult_Continue)
			return result;
	}

	return AutomatonResult_Continue;
}

AutomatonResult
Recognizer::writeChar (uint_t c)
{
	AutomatonResult result;

	if (c < 256) // not a pseudo-char
	{
		((uchar_t*) m_lexeme.m_textPtr.m_p) [m_lexeme.m_length++] = c;
		if (m_lexeme.m_length >= m_lexemeLengthLimit)
		{
			err::setError ("lexeme too long");
			return AutomatonResult_Error;
		}
	}

	uintptr_t targetStateId = m_dfa->getTransition (m_stateId, c);
	if (targetStateId != -1)
	{
		result = gotoState (targetStateId);
	}
	else if (m_lastAcceptStateId != -1)
	{
		result = rollback ();
	}
	else
	{
		err::setError ("unrecognized lexeme");
		return AutomatonResult_Error;
	}

	return result;
}

AutomatonResult
Recognizer::gotoState (size_t stateId)
{
	m_stateId = stateId;

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	if (!(state->m_flags & RecognizerStateFlag_Accept))
		return AutomatonResult_Continue;

	if (state->m_flags & RecognizerStateFlag_Final)
		return match (stateId);

	m_lastAcceptStateId = stateId;
	m_lastAcceptLexemeLength = m_lexeme.m_length;
	return AutomatonResult_Continue;
}

void
Recognizer::processGroupSet (ct::DfaGroupSet* groupSet)
{
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;

	size_t count = groupSet->m_openArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_openArray [i];
		ASSERT (groupId < m_groupCount);

		size_t offset = m_currentOffset - m_lexeme.m_offset;
		offsetArray [groupId * 2] = offset;
		offsetArray [groupId * 2 + 1] = offset;
	}

	count = groupSet->m_closeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t groupId = groupSet->m_closeArray [i];
		ASSERT (groupId < m_groupCount);

		offsetArray [groupId * 2 + 1] = m_currentOffset - m_lexeme.m_offset;
	}
}

AutomatonResult
Recognizer::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptLexemeLength);

	uchar_t* chunk = (uchar_t*) m_lexeme.m_textPtr.m_p + m_lastAcceptLexemeLength;
	size_t chunkLength = m_lexeme.m_length - m_lastAcceptLexemeLength;

	m_currentOffset = m_lexeme.m_offset + m_lastAcceptLexemeLength;
	m_lexeme.m_length = m_lastAcceptLexemeLength;

	// rollback groups

	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t j = i * 2;
		if (offsetArray [j] == -1)
			continue;

		if (offsetArray [j] >= m_lexeme.m_length)
		{
			offsetArray [j] = -1;
			offsetArray [j + 1] = -1;
		}
		else if (offsetArray [j + 1] > m_lexeme.m_length)
		{
			ASSERT (offsetArray [j] < m_lexeme.m_length);
			offsetArray [j + 1] = m_lexeme.m_length;
		}
	}

	AutomatonResult result = match (m_lastAcceptStateId);
	return
		result != AutomatonResult_Continue ? result :
		chunkLength ? writeData (chunk, chunkLength) :
		AutomatonResult_Continue;
}

AutomatonResult
Recognizer::match (size_t stateId)
{
	ASSERT (m_automatonFuncPtr.m_p);

	// save last char and ensure null-termination for the whole lexeme

	uchar_t savedChar = ((uchar_t*) m_lexeme.m_textPtr.m_p) [m_lexeme.m_length];

	((uchar_t*) m_lexeme.m_textPtr.m_p) [m_lexeme.m_length] = 0;

	// create null-terminated sub-lexemes

	ct::DfaStateInfo* state = m_dfa->getStateInfo (stateId);
	ASSERT (state->m_acceptInfo);

	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);

	AutomatonLexeme* subLexemeArray = (AutomatonLexeme*) m_subLexemeArrayPtr.m_p;
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	m_subLexemeCount = state->m_acceptInfo->m_groupCount;

	size_t j = state->m_acceptInfo->m_firstGroupId * 2;
	for (size_t i = 0; i < state->m_acceptInfo->m_groupCount; i++)
	{
		AutomatonLexeme* subLexeme = &subLexemeArray [i];

		size_t offset = offsetArray [j++];
		size_t length = offsetArray [j++] - offset;
		if (offset != -1 && length != 0)
		{
			ASSERT (offset + length <= m_lexeme.m_length);
			subLexeme->m_textPtr = jnc::strDup ((char*) m_lexeme.m_textPtr.m_p + offset, length);
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

	AutomatonResult result = ((AutomatonFunc*) m_automatonFuncPtr.m_p) (m_automatonFuncPtr.m_closure, this, stateId);

	// that's not a mistake below! even though we immediatly reset after that, we DO need to
	// restore the lexeme buffer -- in case `match` is called from `rollback`

	((uchar_t*) m_lexeme.m_textPtr.m_p) [m_lexeme.m_length] = savedChar;

	if (result != AutomatonResult_Continue)
		reset ();
	else
		softReset ();

	return result;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
