//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Value.h"
#include "jnc_RuntimeStructs.h"

namespace jnc {
namespace ct {

//..............................................................................

class Const:
	public ModuleItem,
	public ModuleItemDecl {
	friend class ConstMgr;

protected:
	Value m_value;

public:
	Const() {
		m_itemKind = ModuleItemKind_Const;
	}

	const Value&
	getValue() {
		return m_value;
	}
};

//..............................................................................

class ConstMgr {
	friend class Module;

protected:
	Module* m_module;

	sl::BoxList<Value> m_valueList;
	sl::List<Const> m_constList;
	sl::BoxList<DetachedDataBox> m_constBoxList;

	DataPtr m_emptyLiteralPtr;

public:
	ConstMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	Const*
	createConst(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		const Value& value
	);

	const Value&
	saveValue(const Value& value) {
		sl::BoxIterator<Value> it = m_valueList.insertTail(value);
		return *it;
	}

	const Value&
	saveLiteral(const sl::StringRef& string);

	DataPtr
	getEmptyLiteralPtr() {
		return m_emptyLiteralPtr.m_p ? m_emptyLiteralPtr : createEmptyLiteralPtr();
	}

	DataPtrValidator*
	createConstDataPtrValidator(
		const void* p,
		Type* type
	);

protected:
	DataPtr
	createEmptyLiteralPtr();
};

//..............................................................................

} // namespace ct
} // namespace jnc
