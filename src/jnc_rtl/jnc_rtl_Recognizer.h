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

#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

class Recognizer;

JNC_DECLARE_CLASS_TYPE (Recognizer)

//..............................................................................

enum AutomatonResult
{
	AutomatonResult_Error    = -1,
	AutomatonResult_Continue = 0,
	AutomatonResult_Stop     = 1,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef
AutomatonResult
AutomatonFunc (
	IfaceHdr* closure,
	Recognizer* recognizer,
	uint_t request
	);

//..............................................................................

enum RecognizerField
{
	RecognizerField_InternalState,
	RecognizerField_StateCount,
	RecognizerField_StateFlagTable,
	RecognizerField_TransitionTable,
	RecognizerField__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum RecognizerStateFlag
{
	RecognizerStateFlag_Accept = 0x01,
	RecognizerStateFlag_Final  = 0x02,
};

//..............................................................................

class Recognizer: public IfaceHdr
{
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
	size_t m_lexemeLengthLimit;
	size_t m_currentOffset;

	DataPtr m_lexemePtr;
	size_t m_lexemeOffset;
	size_t m_lexemeLength;

public:
	void
	JNC_CDECL
	construct (FunctionPtr automatonFuncPtr);

	void
	JNC_CDECL
	setAutomatonFunc (FunctionPtr automatonFuncPtr);

	void
	JNC_CDECL
	setLexemeLengthLimit (size_t length);

	void
	JNC_CDECL
	setCurrentOffset (size_t offset);

	void
	JNC_CDECL
	reset ();

	bool
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t length
		);

	bool
	JNC_CDECL
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

//..............................................................................

} // namespace rtl
} // namespace jnc
