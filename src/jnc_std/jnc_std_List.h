#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"

namespace jnc {
namespace std {

class List;

//.............................................................................

struct ListEntry
{
	JNC_BEGIN_TYPE_MAP ("std.ListEntry", g_stdLibCacheSlot, StdLibCacheSlot_ListEntry)
	JNC_END_TYPE_MAP ()

public:
	DataPtr m_nextPtr;
	DataPtr m_prevPtr;
	List* m_list;

	Variant m_data;
};

//.............................................................................

class List: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("std.List", g_stdLibCacheSlot, StdLibCacheSlot_List)
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
	DataPtr m_headPtr;
	DataPtr m_tailPtr;
	size_t m_count;

public:
	void
	AXL_CDECL
	clear ();

	void
	AXL_CDECL
	takeOver (List* list);

	static
	DataPtr
	insertHead (
		List* self,
		Variant data
		);

	static
	DataPtr
	insertTail (
		List* self,
		Variant data
		);

	static
	DataPtr
	insertBefore (
		List* self,
		Variant data,
		DataPtr beforePtr
		);

	static
	DataPtr
	insertAfter (
		List* self,
		Variant data,
		DataPtr afterPtr
		);

	void
	AXL_CDECL
	moveToHead (DataPtr entryPtr);

	void
	AXL_CDECL
	moveToTail (DataPtr entryPtr);
		
	void
	AXL_CDECL
	moveBefore (
		DataPtr entryPtr,
		DataPtr beforePtr
		);

	void
	AXL_CDECL
	moveAfter (
		DataPtr entryPtr,
		DataPtr afterPtr
		);

	static
	Variant
	removeHead (List* self)
	{
		return remove (self, self->m_headPtr);
	}

	static
	Variant
	removeTail (List* self)
	{
		return remove (self, self->m_tailPtr);
	}

	static
	Variant
	remove (
		List* self,
		DataPtr entryPtr
		);

protected:
	static
	DataPtr
	allocateListEntry ();

	void
	insertHeadImpl (DataPtr entryPtr);

	void
	insertTailImpl (DataPtr entryPtr);

	void
	insertBeforeImpl (
		DataPtr entryPtr,
		DataPtr beforePtr
		);

	void
	insertAfterImpl (
		DataPtr entryPtr,
		DataPtr afterPtr
		);

	void
	removeImpl (ListEntry* entry);
};

//.............................................................................

} // namespace std
} // namespace jnc
