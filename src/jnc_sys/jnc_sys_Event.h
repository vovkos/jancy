#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"

namespace jnc {
namespace std {

class List;

//.............................................................................

struct ListEntry
{
	JNC_BEGIN_TYPE_MAP ("std.ListEntry", g_stdLibCacheSlot, StdLibTypeCacheSlot_ListEntry)
	JNC_END_TYPE_MAP ()

public:
	rt::DataPtr m_nextPtr;
	rt::DataPtr m_prevPtr;
	List* m_list;

	rt::Variant m_data;
};

//.............................................................................

class List: public rt::IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("std.List", g_stdLibCacheSlot, StdLibTypeCacheSlot_List)
		JNC_MAP_FUNCTION ("clear", &List::clear)
		JNC_MAP_FUNCTION ("takeOver", &List::takeOver)
		JNC_MAP_FUNCTION ("insertHead", &List::insertHead)
		JNC_MAP_FUNCTION ("insertTail", &List::insertTail)
		JNC_MAP_FUNCTION ("insertBefore", &List::insertBefore)
		JNC_MAP_FUNCTION ("insertAfter", &List::insertAfter)
		JNC_MAP_FUNCTION ("moveToHead", &List::moveToHead)
		JNC_MAP_FUNCTION ("moveToTail", &List::moveToTail)
		JNC_MAP_FUNCTION ("moveBefore", &List::moveBefore)
		JNC_MAP_FUNCTION ("moveAfter", &List::moveAfter)
		JNC_MAP_FUNCTION ("removeHead", &List::removeHead)
		JNC_MAP_FUNCTION ("removeTail", &List::removeTail)
		JNC_MAP_FUNCTION ("remove", &List::remove)
	JNC_END_TYPE_MAP ()

public:
	rt::DataPtr m_headPtr;
	rt::DataPtr m_tailPtr;
	size_t m_count;

public:
	void
	AXL_CDECL
	clear ();

	void
	AXL_CDECL
	takeOver (List* list);

	static
	rt::DataPtr
	insertHead (
		List* self,
		rt::Variant data
		);

	static
	rt::DataPtr
	insertTail (
		List* self,
		rt::Variant data
		);

	static
	rt::DataPtr
	insertBefore (
		List* self,
		rt::Variant data,
		rt::DataPtr beforePtr
		);

	static
	rt::DataPtr
	insertAfter (
		List* self,
		rt::Variant data,
		rt::DataPtr afterPtr
		);

	void
	AXL_CDECL
	moveToHead (rt::DataPtr entryPtr);

	void
	AXL_CDECL
	moveToTail (rt::DataPtr entryPtr);
		
	void
	AXL_CDECL
	moveBefore (
		rt::DataPtr entryPtr,
		rt::DataPtr beforePtr
		);

	void
	AXL_CDECL
	moveAfter (
		rt::DataPtr entryPtr,
		rt::DataPtr afterPtr
		);

	static
	rt::Variant
	removeHead (List* self)
	{
		return remove (self, self->m_headPtr);
	}

	static
	rt::Variant
	removeTail (List* self)
	{
		return remove (self, self->m_tailPtr);
	}

	static
	rt::Variant
	remove (
		List* self,
		rt::DataPtr entryPtr
		);

protected:
	static
	rt::DataPtr
	allocateListEntry ();

	void
	insertHeadImpl (rt::DataPtr entryPtr);

	void
	insertTailImpl (rt::DataPtr entryPtr);

	void
	insertBeforeImpl (
		rt::DataPtr entryPtr,
		rt::DataPtr beforePtr
		);

	void
	insertAfterImpl (
		rt::DataPtr entryPtr,
		rt::DataPtr afterPtr
		);

	void
	removeImpl (ListEntry* entry);
};

//.............................................................................

} // namespace std
} // namespace jnc
