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
	friend class Parser;

protected:
	Value m_value;
	Type* m_dynamicArrayElementType; // we use Const-s for dynamic array fields in dynamic layout

public:
	Const() {
		m_itemKind = ModuleItemKind_Const;
		m_dynamicArrayElementType = NULL;
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
	saveValue(const Value& value);

	const Value&
	saveLiteral(const sl::StringRef& string);

	DataPtr
	getEmptyLiteralPtr() {
		ASSERT(m_emptyLiteralPtr.m_p);
		return m_emptyLiteralPtr;
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const Value&
ConstMgr::saveLiteral(const sl::StringRef& string) {
	Value value;
	value.setCharArray(string, m_module);
	return saveValue(value);
}

//..............................................................................

} // namespace ct
} // namespace jnc
