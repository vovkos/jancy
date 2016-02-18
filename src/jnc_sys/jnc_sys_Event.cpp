#include "pch.h"
#include "jnc_std_List.h"

namespace jnc {
namespace std {

//.............................................................................

void
AXL_CDECL
List::clear ()
{
	ListEntry* entry = (ListEntry*) m_headPtr.m_p;
	for (; entry; entry = (ListEntry*) entry->m_nextPtr.m_p)
		entry->m_list = NULL;

	m_headPtr = rt::g_nullPtr;
	m_tailPtr = rt::g_nullPtr;
	m_count = 0;
}

void
AXL_CDECL
List::takeOver (List* list)
{
	if (!list)
	{
		clear ();
		return;
	}

	ListEntry* entry = (ListEntry*) list->m_headPtr.m_p;
	for (; entry; entry = (ListEntry*) entry->m_nextPtr.m_p)
		entry->m_list = this;

	m_headPtr = list->m_headPtr;
	m_tailPtr = list->m_tailPtr;
	m_count = list->m_count;

	list->m_headPtr = rt::g_nullPtr;
	list->m_tailPtr = rt::g_nullPtr;
	list->m_count = 0;
}

rt::DataPtr
List::insertHead (
	List* self,
	rt::Variant data
	)
{
	rt::DataPtr entryPtr = allocateListEntry ();
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertHeadImpl (entryPtr);
	return entryPtr;
}

rt::DataPtr
List::insertTail (
	List* self,
	rt::Variant data
	)
{
	rt::DataPtr entryPtr = allocateListEntry ();
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertTailImpl (entryPtr);
	return entryPtr;
}

rt::DataPtr
List::insertBefore (
	List* self,
	rt::Variant data,
	rt::DataPtr beforePtr
	)
{
	rt::DataPtr entryPtr = allocateListEntry ();
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertBeforeImpl (entryPtr, beforePtr);
	return entryPtr;
}

rt::DataPtr
List::insertAfter (
	List* self,
	rt::Variant data,
	rt::DataPtr afterPtr
	)
{
	rt::DataPtr entryPtr = allocateListEntry ();
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	entry->m_list = self;
	entry->m_data = data;
	self->insertAfterImpl (entryPtr, afterPtr);
	return entryPtr;
}

void
AXL_CDECL
List::moveToHead (rt::DataPtr entryPtr)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl (entry);
	insertHeadImpl (entryPtr);
}

void
AXL_CDECL
List::moveToTail (rt::DataPtr entryPtr)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl (entry);
	insertTailImpl (entryPtr);
}

void
AXL_CDECL
List::moveBefore (
	rt::DataPtr entryPtr,
	rt::DataPtr beforePtr
	)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl (entry);
	insertBeforeImpl (entryPtr, beforePtr);
}

void
AXL_CDECL
List::moveAfter (
	rt::DataPtr entryPtr,
	rt::DataPtr afterPtr
	)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	if (!entry || entry->m_list != this)
		return;

	removeImpl (entry);
	insertAfterImpl (entryPtr, afterPtr);
}

rt::Variant
List::remove (
	List* self,
	rt::DataPtr entryPtr
	)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	if (!entry || entry->m_list != self)
		return rt::g_nullVariant;

	self->removeImpl (entry);

	entry->m_prevPtr = rt::g_nullPtr;
	entry->m_nextPtr = rt::g_nullPtr;
	entry->m_list = NULL;
	
	return entry->m_data;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

rt::DataPtr
List::allocateListEntry ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ct::Module* module = runtime->getModule ();
	ASSERT (module);

	ct::Type* type = ListEntry::getType (module);
	return runtime->m_gcHeap.allocateData (type);
}

void
List::insertHeadImpl (rt::DataPtr entryPtr)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	ASSERT (entry->m_list == this);

	entry->m_prevPtr = rt::g_nullPtr;
	entry->m_nextPtr = m_headPtr;

	if (m_headPtr.m_p)
		((ListEntry*) m_headPtr.m_p)->m_prevPtr = entryPtr;
	else
		m_tailPtr = entryPtr;

	m_headPtr = entryPtr;
	m_count++;
}

void
List::insertTailImpl (rt::DataPtr entryPtr)
{
	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	ASSERT (entry->m_list == this);

	entry->m_prevPtr = m_tailPtr;
	entry->m_nextPtr = rt::g_nullPtr;

	if (m_tailPtr.m_p)
		((ListEntry*) m_tailPtr.m_p)->m_nextPtr = entryPtr;
	else
		m_headPtr = entryPtr;

	m_tailPtr = entryPtr;
	m_count++;
}

void
List::insertBeforeImpl (
	rt::DataPtr entryPtr,
	rt::DataPtr beforePtr
	)
{
	if (!beforePtr.m_p)
		return insertTailImpl (entryPtr);

	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	ListEntry* before = (ListEntry*) beforePtr.m_p;
	ListEntry* prev = (ListEntry*) before->m_prevPtr.m_p;

	ASSERT (entry->m_list == this);

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
List::insertAfterImpl (
	rt::DataPtr entryPtr,
	rt::DataPtr afterPtr
	)
{
	if (!afterPtr.m_p)
		return insertHeadImpl (entryPtr);

	ListEntry* entry = (ListEntry*) entryPtr.m_p;
	ListEntry* after = (ListEntry*) afterPtr.m_p;
	ListEntry* next = (ListEntry*) after->m_nextPtr.m_p;

	ASSERT (entry->m_list == this);

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
List::removeImpl (ListEntry* entry)
{
	ASSERT (entry->m_list == this);

	ListEntry* next = (ListEntry*) entry->m_nextPtr.m_p;
	ListEntry* prev = (ListEntry*) entry->m_prevPtr.m_p;

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

//.............................................................................

} // namespace std
} // namespace jnc
