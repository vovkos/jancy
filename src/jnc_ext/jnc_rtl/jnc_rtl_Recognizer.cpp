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
	m_lexemeLengthLimit = 128;
	m_currentOffset = 0;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	m_lexemePtr = gcHeap->allocateBuffer (m_lexemeLengthLimit);
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
	if (m_lexemeLength)
		memcpy (ptr.m_p, m_lexemePtr.m_p, m_lexemeLength);

	m_lexemePtr = ptr;
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
	m_lexemeOffset += delta;
}

void
JNC_CDECL
Recognizer::setDfa (ct::Dfa* dfa)
{
	if (m_dfa == dfa)
		return;

	m_dfa = dfa;
	m_groupCount = dfa->getGroupCount ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	Module* module = gcHeap->getRuntime ()->getModule ();
	Type* charPtrType = module->m_typeMgr.getPrimitiveType (TypeKind_Char)->getDataPtrType ();
	Type* sizeType = module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);

	m_groupTextArrayPtr = gcHeap->allocateArray (charPtrType, m_groupCount);
	m_groupOffsetArrayPtr = gcHeap->allocateArray (sizeType, m_groupCount);
	m_groupLengthArrayPtr = gcHeap->allocateArray (sizeType, m_groupCount);

	softReset ();
}

void
JNC_CDECL
Recognizer::reset ()
{
	softReset ();
	m_currentOffset = 0;
}

void
Recognizer::softReset ()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptLexemeLength = 0;
	m_lexemeOffset = 0;
	m_lexemeLength = 0;

	if (!m_dfa)
		return;

	m_groupCount = m_dfa->getGroupCount ();
	if (!m_groupCount)
		return;

	size_t size = m_groupCount * sizeof (void*);
	memset (m_groupTextArrayPtr.m_p, 0, size);
	memset (m_groupOffsetArrayPtr.m_p, -1, size);
	memset (m_groupLengthArrayPtr.m_p, 0, size);

	ct::DfaStateInfo* state = m_dfa->getStateInfo (0);
	if (state->m_groupSet)
		processGroupSet (state->m_groupSet);
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
		if (!m_lexemeLength) // just matched
			return true;

		if (m_lastAcceptStateId == -1)
		{
			err::setError ("unrecognized lexeme");
			return false;
		}

		if (m_lastAcceptLexemeLength >= m_lexemeLength)
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
		((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength++] = c;
		if (m_lexemeLength >= m_lexemeLengthLimit)
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
	m_lastAcceptLexemeLength = m_lexemeLength;
	return AutomatonResult_Continue;
}

void
Recognizer::processGroupSet (ct::DfaGroupSet* groupSet)
{
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	size_t* lengthArray = (size_t*) m_groupLengthArrayPtr.m_p;

	size_t count = groupSet->m_openArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t group = groupSet->m_openArray [i];
		ASSERT (group < m_groupCount);

		offsetArray [group] = m_currentOffset - m_lexemeOffset;
		lengthArray [group] = 0;
	}

	count = groupSet->m_closeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		size_t group = groupSet->m_closeArray [i];
		ASSERT (group < m_groupCount);

		size_t startOffset = offsetArray [group];
		size_t endOffset = m_currentOffset - m_lexemeOffset;
		lengthArray [group] = endOffset - startOffset;
	}
}

AutomatonResult
Recognizer::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptLexemeLength);

	uchar_t* chunk = (uchar_t*) m_lexemePtr.m_p + m_lastAcceptLexemeLength;
	size_t chunkLength = m_lexemeLength - m_lastAcceptLexemeLength;

	m_currentOffset = m_lexemeOffset + m_lastAcceptLexemeLength;
	m_lexemeLength = m_lastAcceptLexemeLength;

	// rollback groups

	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	size_t* lengthArray = (size_t*) m_groupLengthArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t offset = offsetArray [i];
		if (offset == -1)
			continue;

		if (offset >= m_currentOffset)
		{
			offsetArray [i] = -1;
			lengthArray [i] = 0;
		}
		else if (offset + lengthArray [i] >= m_currentOffset)
		{
			lengthArray [i] = 0; // will be updated later on a successful re-match
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

	uchar_t savedChar = ((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength];

	((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength] = 0;

	// create null-terminated group texts

	DataPtr* textArray = (DataPtr*) m_groupTextArrayPtr.m_p;
	size_t* offsetArray = (size_t*) m_groupOffsetArrayPtr.m_p;
	size_t* lengthArray = (size_t*) m_groupLengthArrayPtr.m_p;
	for (size_t i = 0; i < m_groupCount; i++)
	{
		size_t offset = offsetArray [i];
		size_t length = lengthArray [i];
		if (offset != -1 && length != 0)
		{
			textArray [i] = jnc::strDup ((char*) m_lexemePtr.m_p + offset, length);
		}
		else // enforce empty group
		{			
			offsetArray [i] = -1;
			lengthArray [i] = 0;
			textArray [i] = g_nullPtr;
		}
	}
			
	AutomatonResult result = ((AutomatonFunc*) m_automatonFuncPtr.m_p) (m_automatonFuncPtr.m_closure, this, stateId);

	// that's not a mistake below! even we immediatly reset after that, we DO need to 
	// restore the lexeme buffer -- in case `match` is called from `rollback`
	
	((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength] = savedChar;

	softReset ();

	if (result != AutomatonResult_Continue)
		m_currentOffset = 0;

	return result;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
