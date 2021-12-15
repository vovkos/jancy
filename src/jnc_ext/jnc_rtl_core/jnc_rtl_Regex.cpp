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
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<RegexState, uint_t>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegexState>)
	JNC_MAP_CONST_PROPERTY("m_execFlags", &RegexState::getExecFlags)
	JNC_MAP_CONST_PROPERTY("m_offset", &RegexState::getOffset)
	JNC_MAP_CONST_PROPERTY("m_lastExecResult", &RegexState::getLastExecResult)
	JNC_MAP_CONST_PROPERTY("m_matchSwitchCaseId", &RegexState::getMatchSwitchCaseId)
	JNC_MAP_CONST_PROPERTY("m_match", &RegexState::getMatch)
	JNC_MAP_CONST_PROPERTY("m_subMatchCount", &RegexState::getSubMatchCount)
	JNC_MAP_CONST_PROPERTY("m_subMatch", &RegexState::getSubMatch)
	JNC_MAP_FUNCTION("initialize", &RegexState::initialize)
	JNC_MAP_FUNCTION("reset", &RegexState::reset)
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
	if (self->m_textPtr.m_p)
		return self->m_textPtr;

	err::setError("RegexMatch::getText is not yet implemented");
	return g_nullDataPtr;
}

//..............................................................................

void
JNC_CDECL
RegexState::markOpaqueGcRoots(GcHeap* gcHeap) {
	gcHeap->markClassPtr(m_match);
	size_t count = m_subMatchArray.getCount();
	for (size_t i = 0; i < count; i++)
		gcHeap->markClassPtr(m_subMatchArray[i]);
}

RegexMatch*
JNC_CDECL
RegexState::getMatch() {
	err::setError("Regex::getMatch is not yet implemented");
	return NULL;
}

RegexMatch*
JNC_CDECL
RegexState::getSubMatch(size_t i) {
	err::setError("Regex::getSubMatch is not yet implemented");
	return NULL;
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
) {
	return m_regex.exec(
		&state->m_state,
		ptr.m_p,
		length == -1 ? strLen(ptr) : length
	);
}

//..............................................................................

} // namespace rtl
} // namespace jnc
