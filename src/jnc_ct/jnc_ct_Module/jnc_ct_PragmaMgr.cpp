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
#include "jnc_ct_PragmaMgr.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_EnumType.h"

namespace jnc {
namespace ct {

const char*
getPragmaName(Pragma pragmaKind) {
	static const char* stringTable[Pragma__Count] = {
		"undefined-pragma",     // Pragma_Undefined,
		"Alignment",            // Pragma_Alignment,
		"ThinPointers",         // Pragma_ThinPointers,
		"ExposedEnums",         // Pragma_ExposedEnums,
		"RegexAnchored",        // Pragma_RegexAnchored,
		"RegexFullMatch",       // Pragma_RegexFullMatch,
		"RegexCaseInsensitive", // Pragma_RegexCaseInsensitive,
		"RegexLatin1",          // Pragma_RegexLatin1,
		"RegexOneLine",         // Pragma_RegexOneLine,
		"RegexUnanchored",      // Pragma_RegexUnanchored,
		"RegexCaseSensitive",   // Pragma_RegexCaseSensitive,
		"RegexUtf8",            // Pragma_RegexUtf8,
		"RegexMultiLine",       // Pragma_RegexMultiLine,
	};

	return (size_t)pragmaKind < Pragma__Count ?
		stringTable[pragmaKind] :
		stringTable[Pragma_Undefined];

}

//..............................................................................

bool
PragmaConfig::setPragma(
	Pragma pragmaKind,
	PragmaState state,
	int64_t value
) {
	switch (state) {
	case PragmaState_Default:
		value = pragmaKind >= Pragma__FirstBoolDefaultTrue;
		break;

	case PragmaState_NoValue:
		if (pragmaKind < Pragma__FirstBool) {
			err::setFormatStringError("pragma '%s' requires a value", getPragmaName(pragmaKind));
			return false;
		}

		value = true;
		break;
	}

	uint_t regexFlagMask = 0;
	switch (pragmaKind) {
	case Pragma_Alignment:
		if (state == PragmaState_Default)
			m_fieldAlignment = PragmaDefault_Alignment;
		else if (sl::isPowerOf2(value) && value <= 16)
			m_fieldAlignment = value;
		else {
			err::setFormatStringError("invalid alignment %d", value);
			return false;
		}
		break;

	case Pragma_ThinPointers:
		m_pointerModifiers = value ? TypeModifier_Thin : 0;
		break;

	case Pragma_ExposedEnums:
		m_enumFlags = value ? EnumTypeFlag_Exposed : 0;
		break;

	case Pragma_RegexUnanchored:
		m_regexFlags &= ~(re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch);
		if (!value)
			m_regexFlags |= re2::ExecFlag_Anchored;

		regexFlagMask = re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch;
		break;

	case Pragma_RegexAnchored:
		m_regexFlags &= ~(re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch);
		if (value)
			m_regexFlags |= re2::ExecFlag_Anchored;

		regexFlagMask = re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch;
		break;

	case Pragma_RegexFullMatch:
		m_regexFlags &= ~(re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch);
		if (value)
			m_regexFlags |= re2::ExecFlag_FullMatch;

		regexFlagMask = re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch;
		break;

	case Pragma_RegexCaseSensitive:
		if (value)
			m_regexFlags &= ~re2::RegexFlag_CaseInsensitive;
		else
			m_regexFlags |= re2::RegexFlag_CaseInsensitive;

		regexFlagMask = re2::RegexFlag_CaseInsensitive;
		break;

	case Pragma_RegexCaseInsensitive:
		if (value)
			m_regexFlags |= re2::RegexFlag_CaseInsensitive;
		else
			m_regexFlags &= ~re2::RegexFlag_CaseInsensitive;

		regexFlagMask = re2::RegexFlag_CaseInsensitive;
		break;

	case Pragma_RegexLatin1:
		if (value)
			m_regexFlags |= re2::RegexFlag_Latin1;
		else
			m_regexFlags &= ~re2::RegexFlag_Latin1;

		regexFlagMask = re2::RegexFlag_Latin1;
		break;

	case Pragma_RegexUtf8:
		if (value)
			m_regexFlags &= ~re2::RegexFlag_Latin1;
		else
			m_regexFlags |= re2::RegexFlag_Latin1;

		regexFlagMask = re2::RegexFlag_Latin1;
		break;

	case Pragma_RegexOneLine:
		if (value)
			m_regexFlags |= re2::RegexFlag_OneLine;
		else
			m_regexFlags &= ~re2::RegexFlag_OneLine;

		regexFlagMask = re2::RegexFlag_OneLine;
		break;

	case Pragma_RegexMultiLine:
		if (value)
			m_regexFlags &= ~re2::RegexFlag_OneLine;
		else
			m_regexFlags |= re2::RegexFlag_OneLine;

		regexFlagMask = re2::RegexFlag_OneLine;
		break;

	default:
		ASSERT(false);
	}

	if (state == PragmaState_Default && regexFlagMask)
		m_regexFlagMask &= ~regexFlagMask;
	else
		m_regexFlagMask |= regexFlagMask;

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
