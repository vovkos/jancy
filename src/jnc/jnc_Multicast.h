#pragma once

#include "jnc_Value.h"
#include "jnc_MulticastClassType.h"

namespace jnc {
		
//.............................................................................

class MulticastImpl: public Multicast
{
public:
	void
	clear ();

	handle_t
	setHandler (FunctionPtr ptr);

	handle_t
	setHandler_t (void* pf);

	handle_t
	addHandler (FunctionPtr ptr)
	{
		return ptr.m_pf ? addHandlerImpl (ptr) : NULL;
	}

	handle_t
	addHandler_t (void* pf)
	{
		return pf ? addHandlerImpl (pf) : NULL;
	}

	FunctionPtr
	removeHandler (handle_t handle)
	{
		return removeHandlerImpl <FunctionPtr> (handle);
	}

	void* 
	removeHandler_t (handle_t handle)
	{
		return removeHandlerImpl <void*> (handle);
	}

	FunctionPtr
	getSnapshot ();

protected:
	rtl::HandleTable <size_t>*
	getHandleTable ();

	bool
	setCount (
		size_t count,
		size_t ptrSize
		);

	template <typename T>
	handle_t
	setHandlerImpl (T ptr)
	{
		setCount (1, sizeof (T));
		*(T*) m_ptrArray = ptr;
		rtl::HandleTable <size_t>* handleTable = getHandleTable ();
		handleTable->clear ();
		return handleTable->add (0);
	}

	template <typename T>
	handle_t
	addHandlerImpl (T ptr)
	{
		size_t i = m_count;
		setCount (i + 1, sizeof (T));
		*((T*) m_ptrArray + i) = ptr;
		return getHandleTable ()->add (i);
	}

	template <typename T>
	T
	removeHandlerImpl (handle_t handle)
	{
		T ptr = { 0 };

		if (!m_handleTable)
			return ptr;

		rtl::HandleTable <size_t>* handleTable = (rtl::HandleTable <size_t>*) m_handleTable;
		rtl::HandleTable <size_t>::MapIterator mapIt = handleTable->find (handle);
		if (!mapIt)
			return ptr;
	
		rtl::HandleTable <size_t>::ListIterator listIt = mapIt->m_value;	

		size_t i = listIt->m_value;
		ASSERT (i < m_count);

		ptr = *((T*) m_ptrArray + i);

		memmove (((T*) m_ptrArray + i), ((T*) m_ptrArray + i + 1),  (m_count - i) * sizeof (T));
		m_count--;

		for (listIt++; listIt; listIt++) 
			listIt->m_value--;

		handleTable->remove (mapIt);
		return ptr;
	}
};

//.............................................................................

} // namespace jnc
