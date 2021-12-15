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

class RegexMatch;
class RegexState;
class Regex;

JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexMatch)
JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexState)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Regex)

//..............................................................................

typedef re::MatchPos RegexMatchPos;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class RegexMatch: public IfaceHdr {
public:
	RegexMatchPos m_pos; // maps to public fields

protected:
	DataPtr m_textPtr;
	enc::CharCodec* m_codec;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	static
	DataPtr
	getText(RegexMatch* self);
};

//..............................................................................

class RegexState: public IfaceHdr {
	friend class Regex;

protected:
	re::State m_state;
	RegexMatch* m_match;
	sl::Array<RegexMatch*> m_subMatchArray;

public:
	RegexState(uint_t execFlags):
		m_state(execFlags) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	uint_t
	JNC_CDECL
	getExecFlags() {
		return m_state.getExecFlags();
	}

	size_t
	JNC_CDECL
	getOffset() {
		return m_state.getOffset();
	}

	re::ExecResult
	JNC_CDECL
	getLastExecResult() {
		return m_state.getLastExecResult();
	}

	size_t
	JNC_CDECL
	getMatchSwitchCaseId() {
		return m_state.getMatchSwitchCaseId();
	}

	RegexMatch*
	JNC_CDECL
	getMatch();

	size_t
	JNC_CDECL
	getSubMatchCount() {
		return m_state.getSubMatchCount();
	}

	RegexMatch*
	JNC_CDECL
	getSubMatch(size_t i);

	void
	JNC_CDECL
	initialize(uint_t execFlags) {
		m_state.initialize(execFlags);
	}

	void
	JNC_CDECL
	reset(size_t offset) {
		m_state.reset(offset);
	}
};

//..............................................................................

class Regex: public IfaceHdr {
protected:
	re::Regex m_regex;

public:
	void
	JNC_CDECL
	clear() {
		m_regex.clear();
	}

	size_t
	JNC_CDECL
	load(
		DataPtr ptr,
		size_t size
	) {
		return m_regex.load(ptr.m_p, size);
	}

	size_t
	JNC_CDECL
	save(IfaceHdr* buffer);

	bool
	JNC_CDECL
	compile(
		uint_t flags,
		DataPtr sourcePtr,
		size_t length
	);

	void
	JNC_CDECL
	createSwitch() {
		m_regex.createSwitch();
	}

	bool
	JNC_CDECL
	compileSwitchCase(
		uint_t flags,
		DataPtr sourcePtr,
		size_t length
	);

	void
	JNC_CDECL
	finalizeSwitch(uint_t flags)  {
		m_regex.finalizeSwitch(flags);
	}

	re::ExecResult
	JNC_CDECL
	exec(
		RegexState* state,
		DataPtr ptr,
		size_t length
	);

	re::ExecResult
	JNC_CDECL
	eof(RegexState* state) {
		return m_regex.eof(&state->m_state);
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
