#pragma once

#include "jnc_FunctionPtrType.h"
#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {

class Recognizer;

//.............................................................................

enum AutomatonResult
{
	AutomatonResult_Error    = -1,
	AutomatonResult_Continue = 0,
	AutomatonResult_Stop     = 1,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef 
AutomatonResult
AutomatonFunc (
	IfaceHdr* closure,
	Recognizer* recognizer,
	uint_t request
	);

//.............................................................................

enum RecognizerField
{
	RecognizerField_InternalState,
	RecognizerField_StateCount,
	RecognizerField_StateFlagTable,
	RecognizerField_TransitionTable,
	RecognizerField__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum RecognizerStateFlag
{
	RecognizerStateFlag_Accept = 0x01,
	RecognizerStateFlag_Final  = 0x02,
};

//.............................................................................

class Recognizer: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE ("jnc.Recognizer", StdApiSlot_Recognizer)
		JNC_CONSTRUCTOR (&Recognizer::construct)
		JNC_AUTOGET_PROPERTY ("m_automatonFunc", &Recognizer::setAutomatonFunc)
		JNC_FUNCTION ("reset", &Recognizer::reset)
		JNC_FUNCTION ("write", &Recognizer::write)
		JNC_FUNCTION ("eof", &Recognizer::eof)
	JNC_END_TYPE ()

public:
	enum InternalState
	{
		InternalState_Idle = 0,
		InternalState_Started,
	};

protected:
	uintptr_t m_internalState;
	size_t m_stateCount;
	uint_t* m_stateFlagTable;
	uintptr_t* m_transitionTable;
	uintptr_t m_stateId;
	uintptr_t m_lastAcceptStateId;
	size_t m_lastAcceptLexemeLength;
	
public:
	FunctionPtr m_automatonFuncPtr;
	DataPtr m_lexeme;
	size_t m_lexemeOffset;
	size_t m_lexemeLength;
	size_t m_lexemeLengthLimit;
	size_t m_currentOffset;

public:
	void
	AXL_CDECL
	construct (FunctionPtr automatonFuncPtr);

	void
	AXL_CDECL
	setAutomatonFunc (FunctionPtr automatonFuncPtr);

	void
	AXL_CDECL
	reset ();
	
	bool 
	AXL_CDECL
	write (
		DataPtr ptr,
		size_t length
		);

	bool
	AXL_CDECL
	eof ();

protected:
	AutomatonResult
	writeData (
		uchar_t* p, 
		size_t length
		);

	AutomatonResult
	writeChar (uint_t c);

	AutomatonResult
	gotoState (size_t stateId);
	
	AutomatonResult
	rollback ();

	AutomatonResult
	match (size_t stateId);
};

//.............................................................................

} // namespace jnc
