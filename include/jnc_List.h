#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"
#include "jnc_Variant.h"

namespace jnc {

class List;

//.............................................................................

struct ListEntry
{
	JNC_BEGIN_TYPE ("jnc.ListEntry", StdApiSlot_ListEntry)
	JNC_END_TYPE ()

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
	JNC_BEGIN_TYPE ("jnc.List", StdApiSlot_List)
		JNC_FUNCTION ("clear", &List::clear)
		JNC_FUNCTION ("takeOver", &List::takeOver)
		JNC_FUNCTION ("insertHead", &List::insertHead)
		JNC_FUNCTION ("insertTail", &List::insertTail)
		JNC_FUNCTION ("insertBefore", &List::insertBefore)
		JNC_FUNCTION ("insertAfter", &List::insertAfter)
		JNC_FUNCTION ("moveToHead", &List::moveToHead)
		JNC_FUNCTION ("moveToTail", &List::moveToTail)
		JNC_FUNCTION ("moveBefore", &List::moveBefore)
		JNC_FUNCTION ("moveAfter", &List::moveAfter)
		JNC_FUNCTION ("removeHead", &List::removeHead)
		JNC_FUNCTION ("removeTail", &List::removeTail)
		JNC_FUNCTION ("remove", &List::remove)
	JNC_END_TYPE ()

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
	DataBox* 
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

} // namespace jnc
