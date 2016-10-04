// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"
#include "jnc_RuntimeStructs.h"

namespace jnc {
namespace ct {

//.............................................................................

class Const: 
	public ModuleItem,
	public ModuleItemDecl
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
	struct ConstDataPtrValidatorEntry: sl::ListLink
	{
		StaticDataBox m_box;
		DataPtrValidator m_validator;
	};

protected:
	Module* m_module;

	sl::BoxList <Value> m_valueList;
	sl::StdList <Const> m_constList;
	sl::StdList <ConstDataPtrValidatorEntry> m_constDataPtrValidatorList;

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
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		const Value& value
		);

	const Value& 
	saveValue (const Value& value)
	{
		sl::BoxIterator <Value> it = m_valueList.insertTail (value);
		return *it;
	}

	const Value& 
	saveLiteral (const sl::StringRef& string);

	DataPtrValidator*
	createConstDataPtrValidator (
		const void* p,
		Type* type
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
