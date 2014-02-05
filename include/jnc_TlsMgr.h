// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

class CRuntime;
class CTlsMgr;
struct TTlsPage;
struct TGcShadowStackFrame;

//.............................................................................

struct TTlsHdr: public rtl::TListLink
{
	CRuntime* m_pRuntime;
	void* m_pStackEpoch;
	size_t m_GcLevel;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct TTls // struct accessible from jancy
{
	size_t m_ScopeLevel;
	TGcShadowStackFrame* m_pGcShadowStackTop;

	// followed by user TLS variables
};

//.............................................................................

class CTlsDirectory: public ref::CRefCount
{
protected:
	rtl::CArrayT <TTlsHdr*> m_Table;

public:
	~CTlsDirectory ();

	TTlsHdr*
	FindTls (CRuntime* pRuntime);

	TTlsHdr*
	GetTls (CRuntime* pRuntime);

	TTlsHdr*
	NullifyTls (CRuntime* pRuntime);
};

//.............................................................................

class CTlsMgr
{
protected:
	uint64_t m_MainThreadId;
	size_t m_TlsSlot;
	rtl::CBitMap m_SlotMap;
	size_t m_SlotCount;
	
public:
	CTlsMgr ();

	// CreateSlot / DestroySlot should only be called from the main thread;
	// all threads containing tls pages on particular slot (n) should be terminated by 
	// the moment of calling DestroySlot (n)

	size_t 
	CreateSlot ();

	void
	DestroySlot (size_t Slot);

	TTlsHdr*
	FindTls (CRuntime* pRuntime);

	TTlsHdr*
	GetTls (CRuntime* pRuntime);

	TTlsHdr*
	NullifyTls (CRuntime* pRuntime);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
CTlsMgr*
GetTlsMgr ()
{
	return rtl::GetSingleton <CTlsMgr> ();
}

//.............................................................................

} // namespace jnc 
