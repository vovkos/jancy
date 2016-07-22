#include "pch.h"
#include "jnc_rtl_Recognizer.h"

#ifdef _JNC_CORE
#	include "jnc_rt_Runtime.h"
#endif

#include "jnc_Runtime.h"

namespace jnc {
namespace rtl {

//.............................................................................

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
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
AXL_CDECL
Recognizer::construct (FunctionPtr automatonFuncPtr)
{
	m_internalState = InternalState_Idle;
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
AXL_CDECL
Recognizer::setAutomatonFunc (FunctionPtr automatonFuncPtr)
{
	m_automatonFuncPtr = automatonFuncPtr;
	m_stateId = 0;
	m_lastAcceptStateId = -1;

	if (automatonFuncPtr.m_p)
		((AutomatonFunc*) automatonFuncPtr.m_p) (automatonFuncPtr.m_closure, this, -1);
}

void
AXL_CDECL
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
AXL_CDECL
Recognizer::setCurrentOffset (size_t offset)
{
	intptr_t delta = offset - m_currentOffset;

	if (delta)
	{
		m_currentOffset = offset;
		m_lexemeOffset += delta;
	}
}

void
AXL_CDECL
Recognizer::reset ()
{
	m_internalState = InternalState_Idle;
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptLexemeLength = 0;
	m_lexemeOffset = 0;
	m_lexemeLength = 0;
	m_currentOffset = 0;
}

bool 
AXL_CDECL
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

	if (m_internalState == InternalState_Idle)
		m_internalState = InternalState_Started;

	AutomatonResult result = writeData ((uchar_t*) ptr.m_p, length);
	return result != AutomatonResult_Error;
}

bool
AXL_CDECL
Recognizer::eof ()
{
	for (;;)
	{
		if (!m_lexemeLength) // just matched
			return true;

		if (m_lastAcceptStateId == -1)
		{
			err::setStringError ("unrecognized lexeme");
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
			err::setStringError ("lexeme too long");
			return AutomatonResult_Error;
		}
	}

	size_t newStateId = m_transitionTable [m_stateId * 256 + c];
	if (newStateId != -1)
	{
		result = gotoState (newStateId);
	}
	else if (m_lastAcceptStateId != -1)
	{
		result = rollback ();
	}
	else
	{
		err::setStringError ("unrecognized lexeme");
		return AutomatonResult_Error;
	}

	return result;
}

AutomatonResult
Recognizer::gotoState (size_t stateId)
{
	m_stateId = stateId;
			
	uint_t flags = m_stateFlagTable [stateId];
	if (!(flags & RecognizerStateFlag_Accept))
		return AutomatonResult_Continue;

	if (flags & RecognizerStateFlag_Final)
		return match (stateId);

	m_lastAcceptStateId = stateId;
	m_lastAcceptLexemeLength = m_lexemeLength;
	return AutomatonResult_Continue;
}

AutomatonResult
Recognizer::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptLexemeLength);
	
	uchar_t* chunk = (uchar_t*) m_lexemePtr.m_p + m_lastAcceptLexemeLength;
	size_t chunkLength = m_lexemeLength - m_lastAcceptLexemeLength;

	m_currentOffset = m_lexemeOffset + m_lastAcceptLexemeLength;
	m_lexemeLength = m_lastAcceptLexemeLength;

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

	uchar_t savedChar = ((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength];
	((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength] = 0;

	AutomatonResult result = ((AutomatonFunc*) m_automatonFuncPtr.m_p) (m_automatonFuncPtr.m_closure, this, stateId);
	
	((uchar_t*) m_lexemePtr.m_p) [m_lexemeLength] = savedChar;

	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptLexemeLength = 0;
	m_lexemeOffset = m_currentOffset;
	m_lexemeLength = 0;

	if (result != AutomatonResult_Continue)
		m_currentOffset = 0;

	return result;
}

//.............................................................................

} // namespace rtl
} // namespace jnc
