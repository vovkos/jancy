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
	friend class RegexState;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(RegexMatch)

public:
	re::Match m_match; // maps to public fields

protected:
	DataPtr m_textPtr; // cache

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
	Runtime* m_runtime;
	re::State m_state;
	RegexMatch* m_match;
	sl::Array<RegexMatch*> m_captureArray;

public:
	RegexState();

	RegexState(
		uint_t execFlags,
		size_t offset
	);

	void
	JNC_CDECL
	markOpaqueGcRoots(GcHeap* gcHeap);

	uint_t
	JNC_CDECL
	getExecFlags() {
		return m_state.getExecFlags();
	}

	re::ExecResult
	JNC_CDECL
	getLastExecResult() {
		return m_state.getLastExecResult();
	}

	size_t
	JNC_CDECL
	getMatchAcceptId() {
		return m_state.getMatchAcceptId();
	}

	RegexMatch*
	JNC_CDECL
	getMatch();

	size_t
	JNC_CDECL
	getCaptureCount() {
		return m_state.getCaptureCount();
	}

	RegexMatch*
	JNC_CDECL
	getCapture(size_t i);

	RegexMatch*
	JNC_CDECL
	getGroup(size_t i) {
		return i == 0 ? getMatch() : getCapture(i - 1);
	}

	void
	JNC_CDECL
	initialize(
		uint_t execFlags,
		size_t offset
	) {
		clearCache();
		m_state.initialize(re::StateInit(execFlags, offset));
	}

	void
	JNC_CDECL
	reset(size_t offset) {
		clearCache();
		m_state.reset(offset);
	}

	void
	JNC_CDECL
	resume() {
		clearCache();
		m_state.resume();
	}

protected:
	void
	clearCache() {
		m_match = NULL;
		m_captureArray.clear();
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
	eof(
		RegexState* state,
		bool isLastExecDataAvailable
	);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
