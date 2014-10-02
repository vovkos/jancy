// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

class Runtime;
class TlsMgr;
struct TlsPage;
struct GcShadowStackFrame;

//.............................................................................

struct TlsHdr: public rtl::ListLink
{
	Runtime* m_runtime;
	void* m_stackEpoch;
	size_t m_gcLevel;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Tls // struct accessible from jancy
{
	size_t m_scopeLevel;
	GcShadowStackFrame* m_gcShadowStackTop;

	// followed by user TLS variables
};

//.............................................................................

class TlsDirectory: public ref::RefCount
{
protected:
	rtl::Array <TlsHdr*> m_table;

public:
	~TlsDirectory ();

	TlsHdr*
	findTls (Runtime* runtime);

	TlsHdr*
	getTls (Runtime* runtime);

	TlsHdr*
	nullifyTls (Runtime* runtime);
};

//.............................................................................

class TlsMgr
{
protected:
	uint64_t m_mainThreadId;
	size_t m_tlsSlot;
	rtl::BitMap m_slotMap;
	size_t m_slotCount;
	
public:
	TlsMgr ();

	// CreateSlot / DestroySlot should only be called from the main thread;
	// all threads containing tls pages on particular slot (n) should be terminated by 
	// the moment of calling DestroySlot (n)

	size_t 
	createSlot ();

	void
	destroySlot (size_t slot);

	TlsHdr*
	findTls (Runtime* runtime);

	TlsHdr*
	getTls (Runtime* runtime);

	TlsHdr*
	nullifyTls (Runtime* runtime);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
TlsMgr*
getTlsMgr ()
{
	return rtl::getSingleton <TlsMgr> ();
}

//.............................................................................

} // namespace jnc 
