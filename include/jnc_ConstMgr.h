// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"
#include "jnc_LeanDataPtrValidator.h"

namespace jnc {

//.............................................................................

class Const: public UserModuleItem
{
	friend class ConstMgr;
	
protected:
	Value m_value;
	
public:
	Const ()
	{
		m_itemKind = ModuleItemKind_Const;
	}

	Value
	getValue ()
	{
		return m_value;
	}
};

//.............................................................................

class ConstMgr
{
	friend class Module;

protected:
	Module* m_module;

	ref::Ptr <LeanDataPtrValidator> m_unsafeLeanDataPtrValidator;
	rtl::BoxList <Value> m_valueList;
	rtl::StdList <Const> m_constList;

public:
	ConstMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	Const*
	createConst (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		const Value& value
		);

	const Value& 
	saveValue (const Value& value)
	{
		rtl::BoxIterator <Value> it = m_valueList.insertTail (value);
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
