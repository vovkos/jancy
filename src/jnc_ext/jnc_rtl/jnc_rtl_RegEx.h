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

class RegExState;

JNC_DECLARE_OPAQUE_CLASS_TYPE (RegExState)

//..............................................................................

struct RegExMatch
{
	DataPtr m_textPtr;
	size_t m_offset;
	size_t m_length;
};

enum RegExResult
{
	RegExResult_Error    = -1,
	RegExResult_Continue = -2, // incremental regex result depends on upcoming data
};

//..............................................................................

class RegExState: public IfaceHdr
{
public:
	enum StateFlag
	{
		StateFlag_Accept = 0x01,
		StateFlag_Final  = 0x02,
	};

public:
	bool m_isIncremental;

	size_t m_matchLengthLimit;
	size_t m_currentOffset;
	size_t m_consumedLength;

	RegExMatch m_match;
	DataPtr m_subMatchArrayPtr;
	size_t m_subMatchCount;

protected:
	ct::Dfa* m_dfa;

	uintptr_t m_stateId;
	uintptr_t m_lastAcceptStateId;
	size_t m_lastAcceptMatchLength;

	DataPtr m_matchBufferPtr;
	size_t m_matchOffset;
	size_t m_matchLength;

	DataPtr m_groupOffsetArrayPtr;
	size_t m_groupCount;
	size_t m_maxSubMatchCount;

public:
	void
	JNC_CDECL
	construct (bool isIncremental);

	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	void
	JNC_CDECL
	setMatchLengthLimit (size_t length);

	void
	JNC_CDECL
	setCurrentOffset (size_t offset);

	void
	JNC_CDECL
	reset ();

	size_t
	JNC_CDECL
	exec (
		ct::Dfa* dfa,
		DataPtr ptr,
		size_t length
		);

protected:
	void
	processGroupSet (ct::DfaGroupSet* groupSet);

	void
	setDfa (ct::Dfa* dfa);

	void
	softReset ();

	size_t
	eof ();

	size_t
	writeData (
		uchar_t* p,
		size_t length
		);

	size_t
	writeChar (uint_t c);

	size_t
	gotoState (size_t stateId);

	size_t
	rollback ();

	size_t
	match (size_t stateId);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
