// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "axl_mt_TlsMgr.h"

namespace axl {
namespace jnc {

class CModule;
	
//.............................................................................

class CThreadModuleMgr
{
protected:
	size_t m_TlsSlot;
	
public:	
	CThreadModuleMgr ()
	{
		m_TlsSlot = mt::GetTlsMgr ()->CreateSlot ();
	}

	CModule* 
	GetModule ()
	{
		return (CModule*) mt::GetTlsMgr ()->GetSlotValue (m_TlsSlot).p ();
	}

	CModule* 
	SetModule (CModule* pModule)
	{
		return (CModule*) mt::GetTlsMgr ()->SetSlotValue (m_TlsSlot, mt::CTlsValue (pModule, NULL)).p ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
CThreadModuleMgr*
GetThreadModuleMgr ()
{
	return rtl::GetSimpleSingleton <CThreadModuleMgr> ();	
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
CModule*
GetCurrentThreadModule ()
{
	return GetThreadModuleMgr ()->GetModule ();	
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
CModule*
SetCurrentThreadModule (CModule* pModule)
{
	return GetThreadModuleMgr ()->SetModule (pModule);	
}

//.............................................................................

class CScopeThreadModule
{
protected:
	CModule* m_pPrevModule;

public:
	CScopeThreadModule (CModule* pModule)
	{
		m_pPrevModule = SetCurrentThreadModule (pModule);
	}

	~CScopeThreadModule ()
	{
		SetCurrentThreadModule (m_pPrevModule);
	}
};

//.............................................................................

} // namespace jnc {
} // namespace axl {
