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

namespace jnc {
namespace ct {

//..............................................................................

enum PragmaState {
	PragmaState_Default = 0,
	PragmaState_NoValue,
	PragmaState_CustomValue,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum Pragma {
	Pragma_Undefined = 0,
	Pragma_Alignment,
	Pragma__FirstBoolDefaultFalse,
	Pragma__FirstBool             = Pragma__FirstBoolDefaultFalse,
	Pragma_ThinPointers           = Pragma__FirstBoolDefaultFalse,
	Pragma_ExposedEnums,
	Pragma_RegexAnchored,
	Pragma_RegexFullMatch,
	Pragma_RegexCaseInsensitive,
	Pragma_RegexLatin1,
	Pragma_RegexOneLine,
	Pragma__FirstBoolDefaultTrue,
	Pragma_RegexUnanchored        = Pragma__FirstBoolDefaultTrue,
	Pragma_RegexCaseSensitive,
	Pragma_RegexUtf8,
	Pragma_RegexMultiLine,
	Pragma__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getPragmaName(Pragma pragmaKind);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(PragmaMap, Pragma)
	AXL_SL_HASH_TABLE_ENTRY("Alignment",            Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("ThinPointers",         Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("ExposedEnums",         Pragma_ExposedEnums)
	AXL_SL_HASH_TABLE_ENTRY("RegexUnanchored",      Pragma_RegexUnanchored)
	AXL_SL_HASH_TABLE_ENTRY("RegexAnchored",        Pragma_RegexAnchored)
	AXL_SL_HASH_TABLE_ENTRY("RegexFullMatch",       Pragma_RegexFullMatch)
	AXL_SL_HASH_TABLE_ENTRY("RegexCaseSensitive",   Pragma_RegexCaseSensitive)
	AXL_SL_HASH_TABLE_ENTRY("RegexCaseInsensitive", Pragma_RegexCaseInsensitive)
	AXL_SL_HASH_TABLE_ENTRY("RegexUtf8",            Pragma_RegexUtf8)
	AXL_SL_HASH_TABLE_ENTRY("RegexLatin1",          Pragma_RegexLatin1)
	AXL_SL_HASH_TABLE_ENTRY("RegexMultiLine",       Pragma_RegexMultiLine)
	AXL_SL_HASH_TABLE_ENTRY("RegexOneLine",         Pragma_RegexOneLine)
AXL_SL_END_STRING_HASH_TABLE()

//..............................................................................

enum PragmaDefault {
	PragmaDefault_Alignment = 8,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PragmaConfig {
	size_t m_fieldAlignment;
	uint_t m_pointerModifiers;
	uint_t m_enumFlags;
	uint_t m_regexFlags;
	uint_t m_regexFlagMask;

	PragmaConfig() {
		reset();
	}

	void
	reset();

	size_t
	hash() const {
		return sl::HashDjb2<PragmaConfig>()(this);
	}

	bool
	isEqual(const PragmaConfig& src) const {
		return sl::CmpBin<PragmaConfig>()(this, &src) == 0;
	}

	bool
	setPragma(
		Pragma pragmaKind,
		PragmaState state,
		int64_t value = 0
	);
};

//..............................................................................

class PragmaMgr {
protected:
	sl::DuckTypeHashTable<PragmaConfig, bool> m_snapshotDb;

public:
	PragmaConfig m_config;

public:
	PragmaConfig*
	operator -> () {
		return &m_config;
	}

	const PragmaConfig*
	getConfigSnapshot() {
		return &m_snapshotDb.visit(m_config)->getKey();
	}

	void
	clear() {
		m_snapshotDb.clear();
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
