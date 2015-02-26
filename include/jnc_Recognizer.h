#pragma once

#include "jnc_FunctionPtrType.h"
#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {

//.............................................................................

enum RecognizerField
{
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
	JNC_BEGIN_TYPE ("jnc.Recognizer", ApiSlot_jnc_Recognizer)
		JNC_CONSTRUCTOR (&Recognizer::construct)
		JNC_AUTOGET_PROPERTY ("m_automatonFunc", &Recognizer::setAutomatonFunc)
		JNC_FUNCTION ("reset", &Recognizer::reset)
		JNC_FUNCTION ("write", &Recognizer::write)
	JNC_END_TYPE ()

protected:
	size_t m_stateCount;
	uint_t* m_stateFlagTable;
	uintptr_t* m_transitionTable;

public:
	FunctionPtr m_automatonFuncPtr;
	uintptr_t m_stateId;
	size_t m_offset;
	uintptr_t m_lastAcceptStateId;
	size_t m_lastAcceptLexemeLength;
	
	DataPtr m_lexeme;
	size_t m_lexemeOffset;
	size_t m_lexemeLength;
	size_t m_lexemeLengthLimit;

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

protected:
	bool
	writeImpl (
		uchar_t* p, 
		size_t length
		);

	bool
	forceEof ();

	bool
	gotoState (size_t stateId);
	
	bool
	rollback ();

	bool
	match (size_t stateId);
};

//.............................................................................

typedef 
bool
AutomatonFunc (
	IfaceHdr* closure,
	Recognizer* recognizer,
	uint_t request
	);

//.............................................................................

} // namespace jnc
