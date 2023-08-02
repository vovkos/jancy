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
	RegexMatch,
	"jnc.RegexMatch",
	sl::g_nullGuid,
	-1,
	RegexMatch,
	&RegexMatch::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegexMatch)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<RegexMatch>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexMatch>)
	JNC_MAP_CONST_PROPERTY("m_text", &RegexMatch::getText)
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
	JNC_MAP_CONSTRUCTOR(&jnc::construct<RegexState>)
	JNC_MAP_OVERLOAD(&(jnc::construct<RegexState, uint_t, size_t>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexState>)
	JNC_MAP_CONST_PROPERTY("m_execFlags", &RegexState::getExecFlags)
	JNC_MAP_CONST_PROPERTY("m_lastExecResult", &RegexState::getLastExecResult)
	JNC_MAP_CONST_PROPERTY("m_matchAcceptId", &RegexState::getMatchAcceptId)
	JNC_MAP_CONST_PROPERTY("m_match", &RegexState::getMatch)
	JNC_MAP_CONST_PROPERTY("m_captureCount", &RegexState::getCaptureCount)
	JNC_MAP_CONST_PROPERTY("m_captureArray", &RegexState::getCapture)
	JNC_MAP_CONST_PROPERTY("m_groupArray", &RegexState::getGroup)
	JNC_MAP_FUNCTION("initialize", &RegexState::initialize)
	JNC_MAP_FUNCTION("reset", &RegexState::reset)
	JNC_MAP_FUNCTION("resume", &RegexState::resume)
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
	JNC_MAP_CONSTRUCTOR(&jnc::construct<Regex>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Regex>)
	JNC_MAP_FUNCTION("clear", &Regex::clear)
	JNC_MAP_FUNCTION("load", &Regex::load)
	JNC_MAP_FUNCTION("save", &Regex::save)
	JNC_MAP_FUNCTION("compile", &Regex::compile)
	JNC_MAP_FUNCTION("createSwitch", &Regex::createSwitch)
	JNC_MAP_FUNCTION("compileSwitchCase", &Regex::compileSwitchCase)
	JNC_MAP_FUNCTION("finalizeSwitch", &Regex::finalizeSwitch)
	JNC_MAP_FUNCTION("exec", &Regex::exec)
	JNC_MAP_FUNCTION("eof", &Regex::eof)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
RegexMatch::markOpaqueGcRoots(GcHeap* gcHeap)  {
	gcHeap->markDataPtr(m_textPtr);
}

DataPtr
RegexMatch::getText(RegexMatch* self) {
	if (!self->m_textPtr.m_p)
		self->m_textPtr = strDup(self->m_match.getText());

	return self->m_textPtr;
}

//..............................................................................

RegexState::RegexState() {
	m_runtime = getCurrentThreadRuntime();
}

RegexState::RegexState(
	uint_t execFlags,
	uint64_t offset
):
	m_state(re::StateInit(execFlags, offset, enc::CharCodecKind_Utf8)) {
	m_runtime = getCurrentThreadRuntime();
}

void
JNC_CDECL
RegexState::markOpaqueGcRoots(GcHeap* gcHeap) {
	gcHeap->markClassPtr(m_match);
	size_t count = m_captureArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_captureArray[i]);
}

RegexMatch*
JNC_CDECL
RegexState::getMatch() {
	const re::Match* match = m_state.getMatch();
	if (!match)
		return NULL;

	if (!m_match)
		m_match = jnc::createClass<RegexMatch>(m_runtime);

	if (!m_match->m_match.isEqual(*match)) {
		m_match->m_match = *match;
		m_match->m_textPtr = g_nullDataPtr;
	}

	return m_match;
}

RegexMatch*
JNC_CDECL
RegexState::getCapture(size_t i) {
	const re::Match* match = m_state.getCapture(i);
	if (!match)
		return NULL;

	size_t count = m_captureArray.getCount();
	if (i >= count)
		m_captureArray.setCountZeroConstruct(i + 1);

	RegexMatch* matchObject = m_captureArray[i];
	if (!matchObject) {
		matchObject = jnc::createClass<RegexMatch>(m_runtime);
		m_captureArray[i] = matchObject;
	}

	matchObject->m_match = *match;
	return matchObject;
}

//..............................................................................

size_t
JNC_CDECL
Regex::save(IfaceHdr* buffer) {
	err::setError("Regex::save is not yet implemented");
	return -1;
}

bool
JNC_CDECL
Regex::compile(
	uint_t flags,
	DataPtr sourcePtr,
	size_t length
) {
	if (length == -1)
		length = strLen(sourcePtr);

	return m_regex.compile(
		flags,
		sl::String((char*)sourcePtr.m_p, length)
	);
}

bool
JNC_CDECL
Regex::compileSwitchCase(
	uint_t flags,
	DataPtr sourcePtr,
	size_t length
) {
	if (length == -1)
		length = strLen(sourcePtr);

	return m_regex.compileSwitchCase(
		flags,
		sl::String((char*)sourcePtr.m_p, length)
	);
}

re::ExecResult
JNC_CDECL
Regex::exec(
	RegexState* state,
	DataPtr ptr,
	size_t length
)  {
	if (length == -1)
		length = strLen(ptr);

	state->clearCache();
	return m_regex.exec(&state->m_state, ptr.m_p, length);
}

re::ExecResult
JNC_CDECL
Regex::eof(
	RegexState* state,
	bool isLastExecDataAvailable
) {
	state->clearCache();
	return m_regex.eof(&state->m_state, isLastExecDataAvailable);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
