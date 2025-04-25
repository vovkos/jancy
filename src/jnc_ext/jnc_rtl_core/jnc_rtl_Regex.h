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

class RegexCapture;
class RegexMatch;
class RegexState;
class Regex;

JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexCapture)
JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexMatch)
JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexState)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Regex)

//..............................................................................

class RegexCapture: public IfaceHdr {
	friend class Regex;
	friend class RegexState;
	friend class RegexMatch;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(RegexCapture)

public:
	re2::Capture m_capture; // offsets maps to public fields

protected:
	String m_text; // cache
	String m_lastChunk;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	bool
	JNC_CDECL
	hasText() {
		return m_capture.hasText();
	}

	static
	String
	JNC_CDECL
	getText(RegexCapture* self);
};

//..............................................................................

class RegexMatch: public RegexCapture {
	friend class RegexState;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(RegexMatch)

public:
	uint_t m_id;

protected:
	rc::Ptr<re2::Regex> m_regex;
	sl::Array<re2::Capture> m_submatchArray_axl;
	sl::Array<RegexCapture*> m_submatchArray_jnc;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	RegexCapture*
	JNC_CDECL
	getGroup(size_t i);

protected:
	bool
	ensureSubmatchesCaptured();
};

//..............................................................................

class RegexState: public IfaceHdr {
	friend class Regex;

public:
	re2::ExecResult m_lastExecResult;

protected:
	rc::Ptr<re2::Regex> m_regex;
	re2::State m_state;
	RegexMatch* m_match;
	String m_lastChunk;

public:
	RegexState(
		uint_t execFlags,
		uint64_t baseOffset,
		int baseChar,
		uint64_t eofOffset,
		int eofChar
	):
		m_state(execFlags, baseOffset, baseChar, eofOffset, eofChar) {}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	uint_t
	JNC_CDECL
	getExecFlags() {
		return m_state.getExecFlags();
	}

	uint64_t
	JNC_CDECL
	getBaseOffset() {
		return m_state.getBaseOffset();
	}

	int
	JNC_CDECL
	getBaseChar() {
		return m_state.getBaseChar();
	}

	uint64_t
	JNC_CDECL
	getEofOffset() {
		return m_state.getEofOffset();
	}

	int
	JNC_CDECL
	getEofChar() {
		return m_state.getEofChar();
	}

	const RegexMatch*
	JNC_CDECL
	getMatch();

	void
	JNC_CDECL
	reset(
		uint_t execFlags,
		uint64_t baseOffset,
		int baseChar,
		uint64_t eofOffset,
		int eofChar
	);

	void
	JNC_CDECL
	setEofOffset(
		uint64_t offset,
		int eofChar
	) {
		m_state.setEofOffset(offset, eofChar);
	}

	void
	JNC_CDECL
	setEof(int eofChar) {
		m_state.setEof(eofChar);
	}
};

//..............................................................................

class Regex: public IfaceHdr {
public:
	re2::RegexKind m_regexKind;
	uint_t m_flags;
	size_t m_captureCount;
	size_t m_switchCaseCount;

protected:
	rc::Ptr<re2::Regex> m_regex;
	String m_pattern;
	sl::Array<String> m_switchCasePatternArray;

public:
	void
	JNC_CDECL
	init() {
		m_regex = AXL_RC_NEW(rc::Box<re2::Regex>);
		new (&m_switchCasePatternArray) sl::Array<String>();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	static
	String
	JNC_CDECL
	getPattern(Regex* self);

	static
	String
	JNC_CDECL
	getSwitchCasePattern(
		Regex* self,
		uint_t id
	);

	size_t
	JNC_CDECL
	getSwitchCaseCaptureCount(uint_t id) {
		return m_regex->getSwitchCaseCaptureCount(id);
	}

	// compilation

	void
	JNC_CDECL
	clear();

	bool
	JNC_CDECL
	compile(
		String source,
		uint_t flags
	);

	void
	JNC_CDECL
	createSwitch(uint_t flags) {
		clear();
		m_regex->createSwitch(flags);
	}

	uint_t
	JNC_CDECL
	compileSwitchCase(String source) {
		return m_regex->compileSwitchCase(source >> toAxl);
	}

	bool
	JNC_CDECL
	finalizeSwitch();

	// serialization

	size_t
	JNC_CDECL
	load(
		DataPtr ptr,
		size_t size
	);

	size_t
	JNC_CDECL
	save(StdBuffer* buffer_jnc) {
		sl::Array<char> buffer_axl = m_regex->save();
		return buffer_jnc->copy(buffer_axl.cp(), buffer_axl.getCount());
	}

	// execution

	re2::ExecResult
	JNC_CDECL
	exec(
		RegexState* state,
		String chunk
	);

	re2::ExecResult
	JNC_CDECL
	execEof(
		RegexState* state,
		String lastChunk,
		int eofChar
	);

	size_t
	JNC_CDECL
	captureSubmatches(
		uint64_t matchOffset,
		String matchText,
		DataPtr submatchArrayPtr,
		size_t count
	);

	size_t
	JNC_CDECL
	captureSwitchCaseSubmatches(
		uint_t switchCaseId,
		uint64_t matchOffset,
		String matchText,
		DataPtr submatchArrayPtr,
		size_t count
	);

protected:
	void
	finalize();

	void
	saveExecResult(
		RegexState* state,
		re2::ExecResult result,
		String chunk
	);

	void
	createSubmatchCaptureArray(
		String matchText,
		RegexCapture** captureArray_jnc,
		const re2::Capture* const captureArray_axl,
		size_t count
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
Regex::finalize() {
	m_regexKind = m_regex->getRegexKind();
	m_flags = m_regex->getFlags();

	switch (m_regexKind) {
	case re2::RegexKind_Single:
		m_captureCount = m_regex->getCaptureCount();
		break;

	case re2::RegexKind_Switch:
		m_switchCaseCount = m_regex->getSwitchCaseCount();
		break;
	}
}

inline
void
Regex::saveExecResult(
	RegexState* state,
	re2::ExecResult result,
	String chunk
) {
	state->m_lastExecResult = result;
	state->m_match = NULL;

	if (result == re2::ExecResult_Match) {
		state->m_regex = m_regex;
		state->m_lastChunk = chunk;
	} else {
		state->m_regex = rc::g_nullPtr;
		state->m_lastChunk = g_nullString;
	}
}

//..............................................................................

} // namespace rtl
} // namespace jnc
