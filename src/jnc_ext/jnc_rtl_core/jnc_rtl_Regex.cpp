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

#include "pch.h"
#include "jnc_rtl_Regex.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegexCapture,
	"jnc.RegexCapture",
	sl::g_nullGuid,
	-1,
	RegexCapture,
	&RegexCapture::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexCapture)
	JNC_MAP_CONST_PROPERTY("m_text", &RegexCapture::getText)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_CLASS_TYPE(
	RegexMatch,
	"jnc.RegexMatch",
	sl::g_nullGuid,
	-1
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexMatch)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegexState,
	"jnc.RegexState",
	sl::g_nullGuid,
	-1,
	RegexState,
	&RegexState::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexState)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexState>)
	JNC_MAP_CONST_PROPERTY("m_anchor", &RegexState::getAnchor)
	JNC_MAP_CONST_PROPERTY("m_baseOffset", &RegexState::getBaseOffset)
	JNC_MAP_CONST_PROPERTY("m_eofOffset", &RegexState::getEofOffset)
	JNC_MAP_CONST_PROPERTY("m_match", &RegexState::getMatch)
	JNC_MAP_CONST_PROPERTY("m_baseChar", &RegexState::getBaseChar)
	JNC_MAP_CONST_PROPERTY("m_eofChar", &RegexState::getEofChar)
	JNC_MAP_FUNCTION("reset", &RegexState::reset)
	JNC_MAP_FUNCTION("init", &RegexState::init)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Regex,
	"jnc.Regex",
	sl::g_nullGuid,
	-1,
	Regex,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Regex)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Regex>)
	JNC_MAP_FUNCTION("clear", &Regex::clear)
	JNC_MAP_FUNCTION("load", &Regex::load)
	JNC_MAP_FUNCTION("save", &Regex::save)
	JNC_MAP_FUNCTION("compile", &Regex::compile)
	JNC_MAP_FUNCTION("createSwitch", &Regex::createSwitch)
	JNC_MAP_FUNCTION("compileSwitchCase", &Regex::compileSwitchCase)
	JNC_MAP_FUNCTION("finalizeSwitch", &Regex::finalizeSwitch)
	JNC_MAP_FUNCTION("exec", &Regex::exec)
	JNC_MAP_FUNCTION("execEof", &Regex::execEof)
	JNC_MAP_FUNCTION("captureSubmatches", &Regex::captureSubmatches)
	JNC_MAP_FUNCTION("captureSwitchCaseSubmatches", &Regex::captureSwitchCaseSubmatches)
	JNC_MAP_FUNCTION("init", &Regex::init)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
RegexCapture::markOpaqueGcRoots(GcHeap* gcHeap)  {
	ASSERT(!m_text.m_ptr.m_validator || m_text.m_ptr.m_validator == m_lastChunk.m_ptr.m_validator);
	gcHeap->markString(m_text);
	gcHeap->markString(m_lastChunk);
}

String
RegexCapture::getText(RegexMatch* self) {
	if (self->m_text.m_ptr.m_p)
		return self->m_text;

	if (!self->m_capture.hasText())
		return g_nullString;

	const sl::StringRef& text = self->m_capture.getText();

	ASSERT(
		self->m_lastChunk.m_ptr.m_validator &&
		text.cp() >= (char*)self->m_lastChunk.m_ptr.m_validator->m_rangeBegin &&
		text.getEnd() <= (char*)self->m_lastChunk.m_ptr.m_validator->m_rangeEnd
	);

	DataPtr ptr;
	ptr.m_p = (void*)text.cp();
	ptr.m_validator = self->m_lastChunk.m_ptr.m_validator;
	self->m_text.setPtr(ptr, text.getLength());
	return self->m_text;
}

//..............................................................................

void
JNC_CDECL
RegexState::markOpaqueGcRoots(GcHeap* gcHeap) {
	gcHeap->markClassPtr(m_match);
	gcHeap->markString(m_lastChunk);
}

const RegexMatch*
JNC_CDECL
RegexState::getMatch() {
	if (m_match || !m_state.isMatch())
		return m_match;

	m_match = jnc::createClass<RegexMatch>(jnc::getCurrentThreadRuntime());
	m_match->m_capture = m_state.getMatch();
	m_match->m_id = m_state.getMatch().getId();
	m_match->m_lastChunk = m_lastChunk;
	return m_match;
}

void
JNC_CDECL
RegexState::reset(
	re2::Anchor anchor,
	uint64_t baseOffset,
	int baseChar,
	uint64_t eofOffset,
	int eofChar
) {
	m_state.reset(anchor, baseOffset, baseChar, eofOffset, eofChar);
	m_match = NULL;
	m_lastChunk = g_nullString;
}

//..............................................................................

void
JNC_CDECL
Regex::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_switchCasePatternArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markString(m_switchCasePatternArray[i]);
}

void
JNC_CDECL
Regex::clear() {
	m_regexKind = re2::RegexKind_Undefined;
	m_flags = 0;
	m_captureCount = 0;
	m_pattern = g_nullString;
	m_switchCaseCount = 0;
	m_regex.clear();
	m_switchCasePatternArray.clear();
}

bool
JNC_CDECL
Regex::compile(
	uint_t flags,
	String source
) {
	clear();

	bool result = m_regex.compile(flags, source >> toAxl);
	if (!result)
		return false;

	finalize();
	return true;
}

bool
JNC_CDECL
Regex::finalizeSwitch() {
	bool result = m_regex.finalizeSwitch();
	if (!result)
		return false;

	finalize();
	return true;
}

size_t
JNC_CDECL
Regex::load(
	DataPtr ptr,
	size_t size
) {
	clear();

	size_t size = m_regex.load(ptr.m_p, size);
	if (size == -1)
		return -1;

	finalize();
	return size;
}

re2::ExecResult
JNC_CDECL
Regex::exec(
	RegexState* state,
	String chunk
) {
	state->m_match = NULL;
	state->m_lastChunk = chunk;
	return m_regex.exec(&state->m_state, chunk >> toAxl);
}

re2::ExecResult
JNC_CDECL
Regex::execEof(
	RegexState* state,
	String lastChunk,
	int eofChar
) {
	state->m_match = NULL;
	state->m_lastChunk = lastChunk;
	return m_regex.execEof(&state->m_state, lastChunk >> toAxl, eofChar);
}

void
JNC_CDECL
Regex::init() {
	new (&m_regex) Regex();
	new (&m_switchCasePatternArray) sl::Array<String>();
}

/*
void
Regex::finalize() {
	m_regexKind = m_regex.getRegexKind();
	m_flags = m_regex.getFlags();
	m_captureCount = m_regex.getCaptureCount();
	m_pattern = m_regex.getPattern();
	m_switchCaseCount = m_regex.getSwitchCaseCount();
	m_switchCasePatternArray.clear();
}
*/

//..............................................................................

} // namespace rtl
} // namespace jnc
