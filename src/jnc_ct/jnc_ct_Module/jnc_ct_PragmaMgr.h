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

enum Pragma
{
	Pragma_Undefined = 0,
	Pragma_Alignment,
	Pragma_ThinPointers,
	Pragma_ExposedEnums,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SL_BEGIN_STRING_HASH_TABLE(PragmaMap, Pragma)
	AXL_SL_HASH_TABLE_ENTRY("Alignment",    Pragma_Alignment)
	AXL_SL_HASH_TABLE_ENTRY("ThinPointers", Pragma_ThinPointers)
	AXL_SL_HASH_TABLE_ENTRY("ExposedEnums", Pragma_ExposedEnums)
AXL_SL_END_STRING_HASH_TABLE()

//..............................................................................

enum PragmaDefault
{
	PragmaDefault_Alignment        = 8,
	PragmaDefault_PointerModifiers = 0,
	PragmaDefault_EnumFlags        = 0,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct PragmaSettings
{
	size_t m_fieldAlignment;
	uint_t m_pointerModifiers;
	uint_t m_enumFlags;

	PragmaSettings();

	size_t
	hash() const
	{
		return sl::HashDjb2<PragmaSettings>()(this);
	}

	bool
	isEqual(const PragmaSettings& src) const
	{
		return sl::CmpBin<PragmaSettings>()(this, &src) == 0;
	}
};

//..............................................................................

class PragmaMgr
{
protected:
	sl::DuckTypeHashTable<PragmaSettings, bool> m_cache;

public:
	void
	clear()
	{
		m_cache.clear();
	}

	const PragmaSettings*
	getCachedSettings(const PragmaSettings& settings)
	{
		return &m_cache.visit(settings)->getKey();
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
