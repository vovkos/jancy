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

#include "jnc_Def.h"

namespace jnc {
namespace ct {

//..............................................................................

struct BaseTypeCoord {
protected:
	char m_buffer[256];

public:
	sl::Array<int32_t> m_llvmIndexArray;
	size_t m_offset;
	size_t m_vtableIndex;

	BaseTypeCoord();

	void
	reset();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
BaseTypeCoord::BaseTypeCoord():
	m_llvmIndexArray(rc::BufKind_Field, m_buffer, sizeof(m_buffer)) {
	m_offset = 0;
	m_vtableIndex = 0;
}

inline
void
BaseTypeCoord::reset() {
	m_llvmIndexArray.clear();
	m_offset = 0;
	m_vtableIndex = 0;
}

//..............................................................................

enum MemberCoordFlag {
	MemberCoordFlag_Member = 0x01,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// unfortunately, LLVM does not natively support unions
// therefore, unnamed unions on the way to a member need special handling

struct UnionCoord {
	UnionType* m_type;
	intptr_t m_level; // signed for simplier comparisons
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class MemberCoord: public BaseTypeCoord {
protected:
	char m_buffer[256];

public:
	sl::Array<UnionCoord> m_unionCoordArray;
	Variable* m_variable; // alias can reroute to a static
	uint_t m_flags;

	MemberCoord();

	void
	reset();

	void
	append(const MemberCoord& coord);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
MemberCoord::MemberCoord():
	m_unionCoordArray(rc::BufKind_Field, m_buffer, sizeof(m_buffer)) {
	m_variable = NULL;
	m_flags = 0;
}

inline
void
MemberCoord::reset() {
	BaseTypeCoord::reset();
	m_unionCoordArray.clear();
	m_variable = NULL;
	m_flags = 0;
}

inline
void
MemberCoord::append(const MemberCoord& coord) {
	if (coord.m_variable || !(m_flags & coord.m_flags & MemberCoordFlag_Member))
		*this = coord;
	else {
		m_offset += coord.m_offset;
		m_vtableIndex = coord.m_vtableIndex; // vtables can be non-related
		m_llvmIndexArray.append(coord.m_llvmIndexArray);
		m_unionCoordArray.append(coord.m_unionCoordArray);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
