#include "pch.h"
#include "jnc_TlsMgr.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

CTlsDirectory::~CTlsDirectory ()
{
	size_t Count = m_Table.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		TTlsHdr* pTls = m_Table [i];
		if (pTls)
			pTls->m_pRuntime->DestroyTls (pTls);
	}
}

TTlsHdr* 
CTlsDirectory::FindTls (CRuntime* pRuntime)
{
	size_t Slot = pRuntime->GetTlsSlot ();
	return Slot < m_Table.GetCount () ? m_Table [Slot] : NULL;
}

TTlsHdr* 
CTlsDirectory::GetTls (CRuntime* pRuntime)
{
	size_t Slot = pRuntime->GetTlsSlot ();

	if (Slot >= m_Table.GetCount ())
		m_Table.SetCount (Slot + 1);

	TTlsHdr* pTls = m_Table [Slot];
	if (pTls)
	{
		ASSERT (pTls->m_pRuntime == pRuntime);
		return pTls;
	}

	pTls = pRuntime->CreateTls ();
	m_Table [Slot] = pTls;
	return pTls;
}

TTlsHdr*
CTlsDirectory::NullifyTls (CRuntime* pRuntime)
{
	size_t Slot = pRuntime->GetTlsSlot ();
	if (Slot >= m_Table.GetCount ()) 
		return NULL;
	
	TTlsHdr* pTls = m_Table [Slot];
	m_Table [Slot] = NULL;
	return pTls;
}

//.............................................................................

CTlsMgr::CTlsMgr ()
{
	m_MainThreadId = mt::GetCurrentThreadId ();
	m_TlsSlot = mt::GetTlsMgr ()->CreateSlot ();
	m_SlotCount = 0;
}

size_t 
CTlsMgr::CreateSlot ()
{
	ASSERT (mt::GetCurrentThreadId () == m_MainThreadId);

	size_t Slot;

	if (m_SlotCount < m_SlotMap.GetPageCount () * _AXL_PTR_BITNESS)
	{
		Slot = m_SlotMap.FindBit (0, false);
		ASSERT (Slot != -1);
	}
	else
	{
		Slot = m_SlotCount;
	}

	m_SlotMap.SetBitResize (Slot, true);
	return Slot;
}

void
CTlsMgr::DestroySlot (size_t Slot)
{
	ASSERT (mt::GetCurrentThreadId () == m_MainThreadId);
	ASSERT (m_SlotMap.GetBit (Slot));

	m_SlotMap.SetBit (Slot, false);
}

TTlsHdr*
CTlsMgr::FindTls (CRuntime* pRuntime)
{
	CTlsDirectory* pDirectory = (CTlsDirectory*) mt::GetTlsMgr ()->GetSlotValue (m_TlsSlot).p ();
	return pDirectory ? pDirectory->FindTls (pRuntime) : NULL;
}

TTlsHdr*
CTlsMgr::GetTls (CRuntime* pRuntime)
{
	CTlsDirectory* pDirectory = (CTlsDirectory*) mt::GetTlsMgr ()->GetSlotValue (m_TlsSlot).p ();
	if (pDirectory)
		return pDirectory->GetTls (pRuntime);

	ref::CPtrT <CTlsDirectory> Directory = AXL_REF_NEW (CTlsDirectory);
	mt::GetTlsMgr ()->SetSlotValue (m_TlsSlot, Directory);		
	pDirectory = Directory;

	return pDirectory->GetTls (pRuntime);
}

TTlsHdr*
CTlsMgr::NullifyTls (CRuntime* pRuntime)
{
	CTlsDirectory* pDirectory = (CTlsDirectory*) mt::GetTlsMgr ()->GetSlotValue (m_TlsSlot).p ();
	return pDirectory ? pDirectory->NullifyTls (pRuntime) : NULL;
}

//.............................................................................

} // namespace jnc 
