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

enum Pragma {
	Pragma_Undefined = 0,
	Pragma_Alignment,
	Pragma_ThinPointers,
	Pragma_ExposedEnums,
	Pragma_RegexFlags,
	Pragma__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(PragmaMap, Pragma)
	AXL_SL_HASH_TABLE_ENTRY("Alignment",    Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("ThinPointers", Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("ExposedEnums", Pragma_ExposedEnums)
	AXL_SL_HASH_TABLE_ENTRY("RegexFlags",   Pragma_RegexFlags)
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
	uint_t m_mask;

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

	void
	clear() {
		m_snapshotDb.clear();
	}

	const PragmaConfig*
	getConfigSnapshot() {
		return &m_snapshotDb.visit(m_config)->getKey();
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
