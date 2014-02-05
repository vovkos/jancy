#pragma once

#include "jnc_Value.h"
#include "jnc_MulticastClassType.h"

namespace jnc {

//.............................................................................

class CMulticast: public TMulticast
{
public:
	void
	Clear ();

	handle_t
	SetHandler (TFunctionPtr Ptr);

	handle_t
	SetHandler_t (void* pf);

	handle_t
	AddHandler (TFunctionPtr Ptr)
	{
		return Ptr.m_pf ? AddHandlerImpl (Ptr) : NULL;
	}

	handle_t
	AddHandler_t (void* pf)
	{
		return pf ? AddHandlerImpl (pf) : NULL;
	}

	TFunctionPtr
	RemoveHandler (handle_t Handle)
	{
		return RemoveHandlerImpl <TFunctionPtr> (Handle);
	}

	void* 
	RemoveHandler_t (handle_t Handle)
	{
		return RemoveHandlerImpl <void*> (Handle);
	}

	TFunctionPtr
	GetSnapshot ();

protected:
	rtl::CHandleTableT <size_t>*
	GetHandleTable ();

	bool
	SetCount (
		size_t Count,
		size_t PtrSize
		);

	template <typename T>
	handle_t
	SetHandlerImpl (T Ptr)
	{
		SetCount (1, sizeof (T));
		*(T*) m_pPtrArray = Ptr;
		rtl::CHandleTableT <size_t>* pHandleTable = GetHandleTable ();
		pHandleTable->Clear ();
		return pHandleTable->Add (0);
	}

	template <typename T>
	handle_t
	AddHandlerImpl (T Ptr)
	{
		size_t i = m_Count;
		SetCount (i + 1, sizeof (T));
		*((T*) m_pPtrArray + i) = Ptr;
		return GetHandleTable ()->Add (i);
	}

	template <typename T>
	T
	RemoveHandlerImpl (handle_t Handle)
	{
		T Ptr = { 0 };

		if (!m_pHandleTable)
			return Ptr;

		rtl::CHandleTableT <size_t>* pHandleTable = (rtl::CHandleTableT <size_t>*) m_pHandleTable;
		rtl::CHandleTableT <size_t>::CMapIterator MapIt = pHandleTable->Find (Handle);
		if (!MapIt)
			return Ptr;
	
		rtl::CHandleTableT <size_t>::CListIterator ListIt = MapIt->m_Value;	

		size_t i = ListIt->m_Value;
		ASSERT (i < m_Count);

		Ptr = *((T*) m_pPtrArray + i);

		memmove (((T*) m_pPtrArray + i), ((T*) m_pPtrArray + i + 1),  (m_Count - i) * sizeof (T));
		m_Count--;

		for (ListIt++; ListIt; ListIt++) 
			ListIt->m_Value--;

		pHandleTable->Remove (MapIt);
		return Ptr;
	}
};

//.............................................................................

} // namespace jnc
