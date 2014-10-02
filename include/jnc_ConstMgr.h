// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"
#include "jnc_LeanDataPtrValidator.h"

namespace jnc {

//.............................................................................

class ConstMgr
{
	friend class Module;

protected:
	Module* m_module;

	ref::Ptr <LeanDataPtrValidator> m_unsafeLeanDataPtrValidator;
	rtl::BoxList <Value> m_constList;

public:
	ConstMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	const Value& 
	saveValue (const Value& value)
	{
		rtl::BoxIterator <Value> it = m_constList.insertTail (value);
		return *it;
	}

	const Value& 
	saveLiteral (
		const char* p,
		size_t length = -1
		);

	LeanDataPtrValidator*
	getUnsafeLeanDataPtrValidator ();
};

//.............................................................................

} // namespace jnc {
