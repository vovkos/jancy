#include "pch.h"
#include "jnc_TlsMgr.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

TlsDirectory::~TlsDirectory ()
{
	size_t count = m_table.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		TlsHdr* tls = m_table [i];
		if (tls)
			tls->m_runtime->destroyTls (tls);
	}
}

TlsHdr* 
TlsDirectory::findTls (Runtime* runtime)
{
	size_t slot = runtime->getTlsSlot ();
	return slot < m_table.getCount () ? m_table [slot] : NULL;
}

TlsHdr* 
TlsDirectory::getTls (Runtime* runtime)
{
	size_t slot = runtime->getTlsSlot ();

	if (slot >= m_table.getCount ())
		m_table.setCount (slot + 1);

	TlsHdr* tls = m_table [slot];
	if (tls)
	{
		ASSERT (tls->m_runtime == runtime);
		return tls;
	}

	tls = runtime->createTls ();
	m_table [slot] = tls;
	return tls;
}

TlsHdr*
TlsDirectory::nullifyTls (Runtime* runtime)
{
	size_t slot = runtime->getTlsSlot ();
	if (slot >= m_table.getCount ()) 
		return NULL;
	
	TlsHdr* tls = m_table [slot];
	m_table [slot] = NULL;
	return tls;
}

//.............................................................................

TlsMgr::TlsMgr ()
{
	m_mainThreadId = mt::getCurrentThreadId ();
	m_tlsSlot = mt::getTlsMgr ()->createSlot ();
	m_slotCount = 0;
}

size_t 
TlsMgr::createSlot ()
{
	ASSERT (mt::getCurrentThreadId () == m_mainThreadId);

	size_t slot;

	if (m_slotCount < m_slotMap.getPageCount () * _AXL_PTR_BITNESS)
	{
		slot = m_slotMap.findBit (0, false);
		ASSERT (slot != -1);
	}
	else
	{
		slot = m_slotCount;
	}

	m_slotMap.setBitResize (slot, true);
	return slot;
}

void
TlsMgr::destroySlot (size_t slot)
{
	ASSERT (mt::getCurrentThreadId () == m_mainThreadId);
	ASSERT (m_slotMap.getBit (slot));

	m_slotMap.setBit (slot, false);
}

TlsHdr*
TlsMgr::findTls (Runtime* runtime)
{
	TlsDirectory* directory = (TlsDirectory*) mt::getTlsMgr ()->getSlotValue (m_tlsSlot).p ();
	return directory ? directory->findTls (runtime) : NULL;
}

TlsHdr*
TlsMgr::getTls (Runtime* runtime)
{
	TlsDirectory* directory = (TlsDirectory*) mt::getTlsMgr ()->getSlotValue (m_tlsSlot).p ();
	if (directory)
		return directory->getTls (runtime);

	ref::Ptr <TlsDirectory> newDirectory = AXL_REF_NEW (TlsDirectory);
	mt::getTlsMgr ()->setSlotValue (m_tlsSlot, newDirectory);
	directory = newDirectory;

	return directory->getTls (runtime);
}

TlsHdr*
TlsMgr::nullifyTls (Runtime* runtime)
{
	TlsDirectory* directory = (TlsDirectory*) mt::getTlsMgr ()->getSlotValue (m_tlsSlot).p ();
	return directory ? directory->nullifyTls (runtime) : NULL;
}

//.............................................................................

} // namespace jnc 
