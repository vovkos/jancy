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

#include "jnc_RuntimeStructs.h"

namespace jnc {
namespace rtl {

//..............................................................................

class MulticastImpl: public Multicast
{
public:
	void
	destruct();

	void
	clear();

	handle_t
	setHandler(FunctionPtr ptr);

	handle_t
	setHandler_t(void* p);

	handle_t
	addHandler(FunctionPtr ptr)
	{
		return ptr.m_p ? addHandlerImpl(ptr) : NULL;
	}

	handle_t
	addHandler_t(void* p)
	{
		return p ? addHandlerImpl(p) : NULL;
	}

	FunctionPtr
	removeHandler(handle_t handle)
	{
		return removeHandlerImpl<FunctionPtr> (handle);
	}

	void*
	removeHandler_t(handle_t handle)
	{
		return removeHandlerImpl<void*> (handle);
	}

	FunctionPtr
	getSnapshot();

protected:
	sl::HandleTable<size_t>*
	getHandleTable();

	bool
	setCount(
		size_t count,
		size_t ptrSize
		);

	template <typename T>
	handle_t
	setHandlerImpl(T ptr)
	{
		bool result = setCount(1, sizeof(T));
		if (!result)
			return NULL;

		*(T*)m_ptr.m_p = ptr;
		sl::HandleTable<size_t>* handleTable = getHandleTable();
		handleTable->clear();
		return (handle_t)handleTable->add(0);
	}

	template <typename T>
	handle_t
	addHandlerImpl(T ptr)
	{
		size_t i = m_count;
		bool result = setCount(i + 1, sizeof(T));
		if (!result)
			return NULL;

		*((T*)m_ptr.m_p + i) = ptr;
		return (handle_t)getHandleTable()->add(i);
	}

	template <typename T>
	T
	removeHandlerImpl(handle_t handle)
	{
		T ptr = { 0 };

		if (!m_handleTable)
			return ptr;

		sl::HandleTable<size_t>* handleTable = (sl::HandleTable<size_t>*) m_handleTable;
		sl::HandleTableIterator<size_t> it = handleTable->find((uintptr_t)handle);
		if (!it)
			return ptr;

		size_t i = it->m_value;
		ASSERT(i < m_count);

		ptr = *((T*)m_ptr.m_p + i);

		size_t moveSize = (m_count - i - 1) * sizeof(T);
		if (moveSize)
			memmove((T*)m_ptr.m_p + i, (T*)m_ptr.m_p + i + 1, moveSize);

		m_count--;
		memset((T*)m_ptr.m_p + m_count, 0, sizeof(T));

		// adjust following indices

		sl::HandleTableIterator<size_t> it0 = it;
		for (it++; it; it++)
			it->m_value--;

		handleTable->erase(it0);
		return ptr;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
