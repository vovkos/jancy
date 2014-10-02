// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "axl_mt_TlsMgr.h"

namespace axl {
namespace jnc {

class Module;
	
//.............................................................................

class ThreadModuleMgr
{
protected:
	size_t m_tlsSlot;
	
public:	
	ThreadModuleMgr ()
	{
		m_tlsSlot = mt::getTlsMgr ()->createSlot ();
	}

	Module* 
	getModule ()
	{
		return (Module*) mt::getTlsMgr ()->getSlotValue (m_tlsSlot).p ();
	}

	Module* 
	setModule (Module* module)
	{
		return (Module*) mt::getTlsMgr ()->setSlotValue (m_tlsSlot, mt::TlsValue (module, NULL)).p ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ThreadModuleMgr*
getThreadModuleMgr ()
{
	return rtl::getSimpleSingleton <ThreadModuleMgr> ();	
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Module*
getCurrentThreadModule ()
{
	return getThreadModuleMgr ()->getModule ();	
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Module*
setCurrentThreadModule (Module* module)
{
	return getThreadModuleMgr ()->setModule (module);	
}

//.............................................................................

class ScopeThreadModule
{
protected:
	Module* m_prevModule;

public:
	ScopeThreadModule (Module* module)
	{
		m_prevModule = setCurrentThreadModule (module);
	}

	~ScopeThreadModule ()
	{
		setCurrentThreadModule (m_prevModule);
	}
};

//.............................................................................

} // namespace jnc {
} // namespace axl {
