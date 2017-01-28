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
namespace ct {

struct DfaTransition;
struct DfaGroupSet;
class Dfa;

}  // namespace ct

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

struct AutomatonLexeme
{
	DataPtr m_textPtr;
	size_t m_offset;
	size_t m_length;
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

enum RecognizerStateFlag
{
	RecognizerStateFlag_Accept = 0x01,
	RecognizerStateFlag_Final  = 0x02,
};

//..............................................................................

class Recognizer: public IfaceHdr
{
protected:
	ct::Dfa* m_dfa;

	uintptr_t m_stateId;
	uintptr_t m_lastAcceptStateId;
	size_t m_lastAcceptLexemeLength;

	DataPtr m_groupOffsetArrayPtr;
	size_t m_groupCount;
	size_t m_maxSubLexemeCount;

public:
	FunctionPtr m_automatonFuncPtr;
	size_t m_lexemeLengthLimit;
	size_t m_currentOffset;

	AutomatonLexeme m_lexeme;
	DataPtr m_subLexemeArrayPtr;
	size_t m_subLexemeCount;

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

	void
	JNC_CDECL
	setDfa (ct::Dfa* dfa);

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

	void
	processGroupSet (ct::DfaGroupSet* groupSet);

	void
	softReset ();

	AutomatonResult
	rollback ();

	AutomatonResult
	match (size_t stateId);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
