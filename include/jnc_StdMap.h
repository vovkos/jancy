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

#define _JNC_STDMAP_H

#include "jnc_Variant.h"

typedef struct jnc_StdMapEntry jnc_StdMapEntry;
typedef struct jnc_StdMap      jnc_StdMap;

//..............................................................................

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMapEntry_getNext(jnc_StdMapEntry* entry);

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMapEntry_getPrev(jnc_StdMapEntry* entry);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdMapEntry
{
	jnc_DataPtr m_nextPtr;
	jnc_DataPtr m_prevPtr;
	jnc_Variant m_key;
	jnc_Variant m_value;
	jnc_StdMap* m_map;

	// followed by map-specific impl details

#ifdef __cplusplus
	jnc_StdMapEntry*
	getNext()
	{
		return jnc_StdMapEntry_getNext(this);
	}

	jnc_StdMapEntry*
	getPrev()
	{
		return jnc_StdMapEntry_getPrev(this);
	}
#endif
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMapEntry_getNext(jnc_StdMapEntry* entry)
{
	return (jnc_StdMapEntry*)entry->m_nextPtr.m_p;
}

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMapEntry_getPrev(jnc_StdMapEntry* entry)
{
	return (jnc_StdMapEntry*)entry->m_prevPtr.m_p;
}

//..............................................................................

JNC_INLINE
size_t
jnc_StdMap_getItemCount(jnc_StdMap* map);

JNC_INLINE
bool_t
jnc_StdMap_isEmpty(jnc_StdMap* map);

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMap_getHead(jnc_StdMap* map);

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMap_getTail(jnc_StdMap* map);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StdMap
{
	jnc_DataPtr m_headPtr;
	jnc_DataPtr m_tailPtr;
	size_t m_count;

	// followed by map-specific impl details

#ifdef __cplusplus
	JNC_INLINE
	size_t
	getItemCount()
	{
		return jnc_StdMap_getItemCount(this);
	}

	JNC_INLINE
	bool_t
	isEmpty()
	{
		return jnc_StdMap_isEmpty(this);
	}

	JNC_INLINE
	jnc_StdMapEntry*
	getHead()
	{
		return jnc_StdMap_getHead(this);
	}

	JNC_INLINE
	jnc_StdMapEntry*
	getTail()
	{
		return jnc_StdMap_getTail(this);
	}
#endif
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_INLINE
size_t
jnc_StdMap_getItemCount(jnc_StdMap* map)
{
	return map->m_count;
}

JNC_INLINE
bool_t
jnc_StdMap_isEmpty(jnc_StdMap* map)
{
	return map->m_count == 0;
}

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMap_getHead(jnc_StdMap* map)
{
	return (jnc_StdMapEntry*)map->m_headPtr.m_p;
}

JNC_INLINE
jnc_StdMapEntry*
jnc_StdMap_getTail(jnc_StdMap* map)
{
	return (jnc_StdMapEntry*)map->m_tailPtr.m_p;
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

typedef jnc_StdMapEntry StdMapEntry;
typedef jnc_StdMap      StdMap;

//..............................................................................

} // namespace jnc

#endif // __cplusplus
