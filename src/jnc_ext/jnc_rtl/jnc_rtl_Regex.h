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
#include "jnc_ct_RegexMgr.h"

namespace jnc {
namespace rtl {

class RegexState;
class RegexDfa;

JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexState)
JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexDfa)

//..............................................................................

struct RegexMatch
{
	DataPtr m_textPtr;
	size_t m_offset;
	size_t m_length;
};

enum RegexResult
{
	RegexResult_Error    = -1,
	RegexResult_Continue = -2, // incremental regex result depends on upcoming data
};

//..............................................................................

class RegexState: public IfaceHdr
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
	size_t m_replayLength;

	RegexMatch m_match;
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
	size_t m_replayBufferOffset;

	DataPtr m_groupOffsetArrayPtr;
	size_t m_groupCount;
	size_t m_maxSubMatchCount;

public:
	void
	JNC_CDECL
	construct(bool isIncremental);

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	void
	JNC_CDECL
	setMatchLengthLimit(size_t length);

	void
	JNC_CDECL
	setCurrentOffset(size_t offset);

	void
	JNC_CDECL
	reset();

	size_t
	JNC_CDECL
	exec(
		ct::Dfa* dfa,
		DataPtr ptr,
		size_t length
		);

protected:
	void
	processGroupSet(ct::DfaGroupSet* groupSet);

	void
	setDfa(ct::Dfa* dfa);

	void
	softReset();

	size_t
	eof();

	size_t
	writeData(
		uchar_t* p,
		size_t length
		);

	size_t
	writeChar(uchar_t c);

	size_t
	gotoState(size_t stateId);

	void
	match(size_t stateId);

	void
	rollback();
};

//..............................................................................

class RegexDfa: public IfaceHdr
{
protected:
	fsm::Regex m_regex;
	sl::List<ct::ReSwitchAcceptContext> m_acceptContextList;
	ct::Dfa m_dfa;

public:
	void
	JNC_CDECL
	clear();

	bool
	JNC_CDECL
	incrementalCompile(
		DataPtr regexStringPtr,
		size_t length
		);

	bool
	JNC_CDECL
	finalize();

	size_t
	JNC_CDECL
	match(
		RegexState* state,
		DataPtr ptr,
		size_t length
		);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
