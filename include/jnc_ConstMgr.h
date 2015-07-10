// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"
#include "jnc_RuntimeStructs.h"
#include "jnc_DataPtrType.h"

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
	struct ConstDataPtrValidator:
		Box,
		DataPtrValidator
	{
	};

protected:
	Module* m_module;

	rtl::BoxList <Value> m_valueList;
	rtl::StdList <Const> m_constList;
	rtl::BoxList <ConstDataPtrValidator> m_constDataPtrValidatorList;

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

	DataPtrValidator*
	createConstDataPtrValidator (
		void* p,
		Type* type,
		size_t count = 1
		);
};

//.............................................................................

} // namespace jnc {
