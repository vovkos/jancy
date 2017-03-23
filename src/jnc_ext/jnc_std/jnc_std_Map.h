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

#include "jnc_ExtensionLib.h"
#include "jnc_Variant.h"

namespace jnc {
namespace std {

struct Map;

JNC_DECLARE_TYPE (MapEntry)

//..............................................................................

struct MapEntry
{
	JNC_DECLARE_TYPE_STATIC_METHODS (MapEntry)

	DataPtr m_nextPtr;
	DataPtr m_prevPtr;

	Variant m_key;
	Variant m_value;

	Map* m_map;
	sl::MapEntry <Variant, DataPtr>* m_mapEntry;
};

//..............................................................................

struct Map
{
	DataPtr m_headPtr;
	DataPtr m_tailPtr;
	size_t m_count;

	void
	clear ();

	DataPtr
	add (const sl::MapIterator <Variant, DataPtr>& it);

	void
	remove (MapEntry* entry);
};

//..............................................................................

} // namespace std
} // namespace jnc
