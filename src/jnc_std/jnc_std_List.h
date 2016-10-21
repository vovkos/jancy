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

class List;

JNC_DECLARE_TYPE (ListEntry)
JNC_DECLARE_CLASS_TYPE (List)

//..............................................................................

struct ListEntry
{
	JNC_DECLARE_TYPE_STATIC_METHODS (ListEntry)

	DataPtr m_nextPtr;
	DataPtr m_prevPtr;
	List* m_list;

	Variant m_data;
};

//..............................................................................

class List: public IfaceHdr
{
public:

public:
	DataPtr m_headPtr;
	DataPtr m_tailPtr;
	size_t m_count;

public:
	void
	JNC_CDECL
	clear ();

	void
	JNC_CDECL
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
	JNC_CDECL
	moveToHead (DataPtr entryPtr);

	void
	JNC_CDECL
	moveToTail (DataPtr entryPtr);

	void
	JNC_CDECL
	moveBefore (
		DataPtr entryPtr,
		DataPtr beforePtr
		);

	void
	JNC_CDECL
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

//..............................................................................

} // namespace std
} // namespace jnc
