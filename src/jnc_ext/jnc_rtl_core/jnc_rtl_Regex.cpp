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
	JNC_MAP_CONST_PROPERTY("m_hasText", &RegexCapture::hasText)
	JNC_MAP_CONST_PROPERTY("m_text", &RegexCapture::getText)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegexMatch,
	"jnc.RegexMatch",
	sl::g_nullGuid,
	-1,
	RegexMatch,
	&RegexMatch::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexMatch)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexMatch>)
	JNC_MAP_CONST_PROPERTY("m_groupArray", &RegexMatch::getGroup)
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
	JNC_MAP_CONSTRUCTOR(
		&(jnc::construct<RegexState,
			uint_t,
			uint64_t,
			int,
			uint64_t,
			int
		>)
	)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexState>)
	JNC_MAP_CONST_PROPERTY("m_execFlags", &RegexState::getExecFlags)
	JNC_MAP_CONST_PROPERTY("m_baseOffset", &RegexState::getBaseOffset)
	JNC_MAP_CONST_PROPERTY("m_baseChar", &RegexState::getBaseChar)
	JNC_MAP_CONST_PROPERTY("m_eofOffset", &RegexState::getEofOffset)
	JNC_MAP_CONST_PROPERTY("m_eofChar", &RegexState::getEofChar)
	JNC_MAP_CONST_PROPERTY("m_match", &RegexState::getMatch)
	JNC_MAP_FUNCTION("reset", &RegexState::reset)
	JNC_MAP_FUNCTION("setEofOffset", &RegexState::setEofOffset)
	JNC_MAP_FUNCTION("setEof", &RegexState::setEof)
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
	JNC_MAP_FUNCTION("init", &Regex::init)
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
RegexMatch::markOpaqueGcRoots(GcHeap* gcHeap) {
	size_t count = m_submatchArray_jnc.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_submatchArray_jnc[i]);
}

RegexCapture*
JNC_CDECL
RegexMatch::getGroup(size_t i) {
	bool result = ensureSubmatchesCaptured();
	if (!result)
		return NULL;

	size_t count = m_submatchArray_axl.getCount();
	if (m_submatchArray_jnc.getCount() != count)
		m_submatchArray_jnc.setCountZeroConstruct(count);

	if (i >= count || !m_submatchArray_axl[i])
		return NULL;

	if (m_submatchArray_jnc[i])
		return m_submatchArray_jnc[i];

	RegexCapture* capture = jnc::createClass<RegexCapture>(jnc::getCurrentThreadRuntime());
	capture->m_capture = m_submatchArray_axl[i];
	capture->m_lastChunk = m_lastChunk;
	m_submatchArray_jnc[i] = capture;
	return capture;
}

bool
RegexMatch::ensureSubmatchesCaptured() {
	if (m_regex->getRegexKind() == re2::RegexKind_Switch) {
		size_t count = m_regex->getSwitchCaseCaptureCount(m_id) + 1;
		if (m_submatchArray_axl.getCount() != count) {
			m_submatchArray_axl.setCount(count);
			return m_regex->captureSwitchCaseSubmatches(m_id, m_capture, m_submatchArray_axl, count);
		}
	} else {
		size_t count = m_regex->getCaptureCount() + 1;
		if (m_submatchArray_axl.getCount() != count) {
			m_submatchArray_axl.setCount(count);
			return m_regex->captureSubmatches(m_capture, m_submatchArray_axl, count);
		}
	}

	return true;
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
	m_match->m_regex = m_regex;
	m_match->m_capture = m_state.getMatch();
	m_match->m_id = m_state.getMatch().getId();
	m_match->m_lastChunk = m_lastChunk;
	return m_match;
}

void
JNC_CDECL
RegexState::reset(
	uint_t execFlags,
	uint64_t baseOffset,
	int baseChar,
	uint64_t eofOffset,
	int eofChar
) {
	m_state.reset(execFlags, baseOffset, baseChar, eofOffset, eofChar);
	m_match = NULL;
	m_lastChunk = g_nullString;
}

//..............................................................................

void
JNC_CDECL
Regex::markOpaqueGcRoots(GcHeap* gcHeap) {
	gcHeap->markString(m_pattern);

	size_t count = m_switchCasePatternArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markString(m_switchCasePatternArray[i]);
}

String
JNC_CDECL
Regex::getPattern(Regex* self) {
	return self->m_pattern.m_length ?
		self->m_pattern :
		self->m_pattern = allocateString(self->m_regex->getPattern());
}

String
JNC_CDECL
Regex::getSwitchCasePattern(
	Regex* self,
	uint_t id
) {
	size_t count = self->m_regex->getSwitchCaseCount();
	ASSERT(count == self->m_switchCaseCount);

	if (id > count)
		return g_nullString;

	if (self->m_switchCasePatternArray.isEmpty())
		self->m_switchCasePatternArray.setCount(count);

	if (!self->m_switchCasePatternArray[id].m_length)
		self->m_switchCasePatternArray[id] = allocateString(self->m_regex->getSwitchCasePattern(id));

	return self->m_switchCasePatternArray[id];
}

void
JNC_CDECL
Regex::clear() {
	m_regexKind = re2::RegexKind_Undefined;
	m_flags = 0;
	m_captureCount = 0;
	m_pattern = g_nullString;
	m_switchCaseCount = 0;

	if (m_regex.getRefCount()->getRefCount() == 1)
		m_regex->clear();
	else
		m_regex = AXL_RC_NEW(rc::Box<re2::Regex>);

	m_switchCasePatternArray.clear();
}

bool
JNC_CDECL
Regex::compile(
	String source,
	uint_t flags
) {
	clear();

	bool result = m_regex->compile(source >> toAxl, flags);
	if (!result)
		return false;

	finalize();
	return true;
}

bool
JNC_CDECL
Regex::finalizeSwitch() {
	bool result = m_regex->finalizeSwitch();
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

	size_t result = m_regex->load(ptr.m_p, size);
	if (result == -1)
		return -1;

	finalize();
	return result;
}

re2::ExecResult
JNC_CDECL
Regex::exec(
	RegexState* state,
	String chunk
) {
	re2::ExecResult result = m_regex->exec(&state->m_state, chunk >> toAxl);
	saveExecResult(state, result, chunk);
	return result;
}

re2::ExecResult
JNC_CDECL
Regex::execEof(
	RegexState* state,
	String lastChunk,
	int eofChar
) {
	re2::ExecResult result = m_regex->execEof(&state->m_state, lastChunk >> toAxl, eofChar);
	saveExecResult(state, result, lastChunk);
	return result;
}

size_t
JNC_CDECL
Regex::captureSubmatches(
	uint64_t matchOffset,
	String matchText,
	DataPtr submatchArrayPtr,
	size_t count
) {
	memset(submatchArrayPtr.m_p, 0, count * sizeof(RegexCapture*));

	size_t submatchCount = m_regex->getCaptureCount() + 1;
	if (count > submatchCount)
		count = submatchCount;

	char buffer[256];
	sl::Array<re2::Capture> submatchArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	submatchArray.setCount(count);
	count = m_regex->captureSubmatches(matchText >> toAxl, submatchArray, count);
	if (count == -1)
		return -1;

	createSubmatchCaptureArray(
		matchText,
		(RegexCapture**)submatchArrayPtr.m_p,
		submatchArray,
		count
	);

	return count;
}

size_t
JNC_CDECL
Regex::captureSwitchCaseSubmatches(
	uint_t switchCaseId,
	uint64_t matchOffset,
	String matchText,
	DataPtr submatchArrayPtr,
	size_t count
) {
	memset(submatchArrayPtr.m_p, 0, count * sizeof(RegexCapture*));

	size_t submatchCount = m_regex->getSwitchCaseCaptureCount(switchCaseId) + 1;
	if (count > submatchCount)
		count = submatchCount;

	char buffer[256];
	sl::Array<re2::Capture> submatchArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	submatchArray.setCount(count);
	count = m_regex->captureSwitchCaseSubmatches(switchCaseId, matchText >> toAxl, submatchArray, count);
	if (count == -1)
		return -1;

	createSubmatchCaptureArray(
		matchText,
		(RegexCapture**)submatchArrayPtr.m_p,
		submatchArray,
		count
	);

	return count;
}

void
Regex::createSubmatchCaptureArray(
	String matchText,
	RegexCapture** captureArray_jnc,
	const re2::Capture* const captureArray_axl,
	size_t count
) {
	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	for (size_t i = 0; i < count; i++) {
		if (!captureArray_axl[i])
			continue;

		RegexCapture* capture = createClass<RegexCapture>(runtime);
		capture->m_capture = captureArray_axl[i];
		capture->m_lastChunk = matchText;
		captureArray_jnc[i] = capture;
	}
}

//..............................................................................

} // namespace rtl
} // namespace jnc
