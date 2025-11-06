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

#include "jnc_ct_Pragma.h"

namespace jnc {
namespace ct {

//..............................................................................

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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
PragmaConfig::reset() {
	m_fieldAlignment = PragmaDefault_Alignment;
	m_pointerModifiers = 0;
	m_enumFlags = 0;
	m_regexFlags = 0;
	m_regexFlagMask = 0;
}

//..............................................................................

class PragmaMgr {
protected:
	sl::DuckTypeHashTable<PragmaConfig, bool> m_snapshotDb;

public:
	const PragmaConfig*
	getConfigSnapshot(const PragmaConfig& config) {
		return &m_snapshotDb.visit(config)->getKey();
	}

	void
	clear() {
		m_snapshotDb.clear();
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
