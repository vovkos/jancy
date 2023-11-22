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
JNC_DECLARE_CLASS_TYPE(RegexMatch)
JNC_DECLARE_OPAQUE_CLASS_TYPE(RegexState)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Regex)

//..............................................................................

class RegexCapture: public IfaceHdr {
	friend class RegexState;

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

	static
	String
	getText(RegexMatch* self);
};

//..............................................................................

class RegexMatch: public RegexCapture {
	friend class RegexState;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(RegexMatch)

public:
	uint_t m_id;
};

//..............................................................................

class RegexState: public IfaceHdr {
	friend class Regex;

protected:
	re2::State m_state;
	RegexMatch* m_match;
	String m_lastChunk;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	re2::Anchor
	JNC_CDECL
	getAnchor() {
		return m_state.getAnchor();
	}

	uint64_t
	JNC_CDECL
	getBaseOffset() {
		return m_state.getBaseOffset();
	}

	uint64_t
	JNC_CDECL
	getEofOffset() {
		return m_state.getEofOffset();
	}

	int
	JNC_CDECL
	getBaseChar() {
		return m_state.getBaseChar();
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
		re2::Anchor anchor,
		uint64_t baseOffset,
		int baseChar,
		uint64_t eofOffset,
		int eofChar
	);

	void
	JNC_CDECL
	setEof(
		uint64_t offset,
		int eofChar
	) {
		m_state.setEof(offset, eofChar);
	}

	void
	JNC_CDECL
	init(
		re2::Anchor anchor,
		uint64_t baseOffset,
		int baseChar,
		uint64_t eofOffset,
		int eofChar
	) {
		new (&m_state) re2::State(anchor, baseOffset, baseChar, eofOffset, eofChar);
	}
};

//..............................................................................

class Regex: public IfaceHdr {
public:
	re2::RegexKind m_regexKind;
	uint_t m_flags;
	size_t m_captureCount;
	String m_pattern;
	size_t m_switchCaseCount;

protected:
	re2::Regex m_regex;
	sl::Array<String> m_switchCasePatternArray;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	// compilation

	void
	JNC_CDECL
	clear();

	bool
	JNC_CDECL
	compile(
		uint_t flags,
		String source
	);

	void
	JNC_CDECL
	createSwitch(uint_t flags) {
		clear();
		m_regex.createSwitch(flags);
	}

	uint_t
	JNC_CDECL
	compileSwitchCase(String source) {
		return m_regex.compileSwitchCase(source >> toAxl);
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
		sl::Array<char> buffer_axl = m_regex.save();
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

	bool
	JNC_CDECL
	captureSubmatches(
		uint64_t matchOffset,
		String matchText,
		DataPtr submatchArrayPtr,
		size_t count
	);

	bool
	JNC_CDECL
	captureSwitchCaseSubmatches(
		uint_t switchCaseId,
		uint64_t matchOffset,
		String matchText,
		DataPtr submatchArrayPtr,
		size_t count
	);

	void
	JNC_CDECL
	init();

protected:
	void
	finalize();
};

//..............................................................................

} // namespace rtl
} // namespace jnc
