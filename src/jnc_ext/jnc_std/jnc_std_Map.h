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
#include "jnc_StdMap.h"

namespace jnc {
namespace std {

struct Map;

JNC_DECLARE_TYPE(MapEntry)

//..............................................................................

struct MapEntry: StdMapEntry
{
	JNC_DECLARE_TYPE_STATIC_METHODS(MapEntry)

	sl::MapEntry<Variant, DataPtr>* m_mapEntry;
};

//..............................................................................

struct Map: StdMap
{
	void
	clear();

	DataPtr
	add(const sl::MapIterator<Variant, DataPtr>& it);

	void
	remove(MapEntry* entry);
};

//..............................................................................

} // namespace std
} // namespace jnc
