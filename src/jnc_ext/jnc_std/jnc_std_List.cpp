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

#include "pch.h"
#include "jnc_std_List.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_TYPE(
	ListEntry,
	"std.ListEntry",
	g_stdLibGuid,
	StdLibCacheSlot_ListEntry
)

JNC_BEGIN_TYPE_FUNCTION_MAP(ListEntry)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_CLASS_TYPE(
	List,
	"std.List",
	g_stdLibGuid,
	StdLibCacheSlot_List
)

JNC_BEGIN_TYPE_FUNCTION_MAP(List)
	JNC_MAP_FUNCTION("clear", &List::clear)
	JNC_MAP_FUNCTION("takeOver", &List::takeOver)
	JNC_MAP_FUNCTION("insertHead", &List::insertHead)
	JNC_MAP_FUNCTION("insertTail", &List::insertTail)
	JNC_MAP_FUNCTION("insertBefore", &List::insertBefore)
	JNC_MAP_FUNCTION("insertAfter", &List::insertAfter)
	JNC_MAP_FUNCTION("moveToHead", &List::moveToHead)
	JNC_MAP_FUNCTION("moveToTail", &List::moveToTail)
	JNC_MAP_FUNCTION("moveBefore", &List::moveBefore)
	JNC_MAP_FUNCTION("moveAfter", &List::moveAfter)
	JNC_MAP_FUNCTION("removeHead", &List::removeHead)
	JNC_MAP_FUNCTION("removeTail", &List::removeTail)
	JNC_MAP_FUNCTION("remove", &List::remove)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
List::clear() {
	ListEntry* entry = (ListEntry*)m_headPtr.m_p;
	for (; entry; entry = (ListEntry*)entry->m_nextPtr.m_p)
		entry->m_list = NULL;

	m_headPtr = g_nullDataPtr;
	m_tailPtr = g_nullDataPtr;
	m_count = 0;
}

void
JNC_CDECL
List::takeOver(List* list) {
	if (!list) {
		clear();
		return;
	}

	ListEntry* entry = (ListEntry*)list->m_headPtr.m_p;
	for (; entry; entry = (ListEntry*)entry->m_nextPtr.m_p)
		entry->m_list = this;

	m_headPtr = list->m_headPtr;
	m_tailPtr = list->m_tailPtr;
	m_count = list->m_count;

	list->m_headPtr = g_nullDataPtr;
	list->m_tailPtr = g_nullDataPtr;
	list->m_count = 0;
}

DataPtr
List::insertHead(
	List* self,
	Variant data
) {
	DataPtr entryPtr = allocateListEntry();
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertHeadImpl(entryPtr);
	return entryPtr;
}

DataPtr
List::insertTail(
	List* self,
	Variant data
) {
	DataPtr entryPtr = allocateListEntry();
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertTailImpl(entryPtr);
	return entryPtr;
}

DataPtr
List::insertBefore(
	List* self,
	Variant data,
	DataPtr beforePtr
) {
	DataPtr entryPtr = allocateListEntry();
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertBeforeImpl(entryPtr, beforePtr);
	return entryPtr;
}

DataPtr
List::insertAfter(
	List* self,
	Variant data,
	DataPtr afterPtr
) {
	DataPtr entryPtr = allocateListEntry();
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertAfterImpl(entryPtr, afterPtr);
	return entryPtr;
}

void
JNC_CDECL
List::moveToHead(DataPtr entryPtr) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl(entry);
	insertHeadImpl(entryPtr);
}

void
JNC_CDECL
List::moveToTail(DataPtr entryPtr) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl(entry);
	insertTailImpl(entryPtr);
}

void
JNC_CDECL
List::moveBefore(
	DataPtr entryPtr,
	DataPtr beforePtr
) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl(entry);
	insertBeforeImpl(entryPtr, beforePtr);
}

void
JNC_CDECL
List::moveAfter(
	DataPtr entryPtr,
	DataPtr afterPtr
) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl(entry);
	insertAfterImpl(entryPtr, afterPtr);
}

Variant
List::remove(
	List* self,
	DataPtr entryPtr
) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	if (!entry || entry->m_list != self)
		return g_nullVariant;

	self->removeImpl(entry);

	entry->m_prevPtr = g_nullDataPtr;
	entry->m_nextPtr = g_nullDataPtr;
	entry->m_list = NULL;

	return entry->m_data;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
List::allocateListEntry() {
	Runtime* runtime = getCurrentThreadRuntime();
	ASSERT(runtime);

	return createData<ListEntry> (runtime);
}

void
List::insertHeadImpl(DataPtr entryPtr) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	ASSERT(entry->m_list == this);

	entry->m_prevPtr = g_nullDataPtr;
	entry->m_nextPtr = m_headPtr;

	if (m_headPtr.m_p)
		((ListEntry*)m_headPtr.m_p)->m_prevPtr = entryPtr;
	else
		m_tailPtr = entryPtr;

	m_headPtr = entryPtr;
	m_count++;
}

void
List::insertTailImpl(DataPtr entryPtr) {
	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	ASSERT(entry->m_list == this);

	entry->m_prevPtr = m_tailPtr;
	entry->m_nextPtr = g_nullDataPtr;

	if (m_tailPtr.m_p)
		((ListEntry*)m_tailPtr.m_p)->m_nextPtr = entryPtr;
	else
		m_headPtr = entryPtr;

	m_tailPtr = entryPtr;
	m_count++;
}

void
List::insertBeforeImpl(
	DataPtr entryPtr,
	DataPtr beforePtr
) {
	if (!beforePtr.m_p)
		return insertTailImpl(entryPtr);

	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	ListEntry* before = (ListEntry*)beforePtr.m_p;
	ListEntry* prev = (ListEntry*)before->m_prevPtr.m_p;

	ASSERT(entry->m_list == this);

	entry->m_prevPtr = before->m_prevPtr;
	entry->m_nextPtr = beforePtr;
	before->m_prevPtr = entryPtr;

	if (prev)
		prev->m_nextPtr = entryPtr;
	else
		m_headPtr = entryPtr;

	m_count++;
}

void
List::insertAfterImpl(
	DataPtr entryPtr,
	DataPtr afterPtr
) {
	if (!afterPtr.m_p)
		return insertHeadImpl(entryPtr);

	ListEntry* entry = (ListEntry*)entryPtr.m_p;
	ListEntry* after = (ListEntry*)afterPtr.m_p;
	ListEntry* next = (ListEntry*)after->m_nextPtr.m_p;

	ASSERT(entry->m_list == this);

	entry->m_prevPtr = afterPtr;
	entry->m_nextPtr = after->m_nextPtr;
	after->m_nextPtr = entryPtr;

	if (next)
		next->m_prevPtr = entryPtr;
	else
		m_tailPtr = entryPtr;

	m_count++;
}

void
List::removeImpl(ListEntry* entry) {
	ASSERT(entry->m_list == this);

	ListEntry* next = (ListEntry*)entry->m_nextPtr.m_p;
	ListEntry* prev = (ListEntry*)entry->m_prevPtr.m_p;

	if (prev)
		prev->m_nextPtr = entry->m_nextPtr;
	else
		m_headPtr = entry->m_nextPtr;

	if (next)
		next->m_prevPtr = entry->m_prevPtr;
	else
		m_tailPtr = entry->m_prevPtr;

	m_count--;
}

//..............................................................................

} // namespace std
} // namespace jnc
