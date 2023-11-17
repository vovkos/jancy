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

#include "pch.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ConstMgr::ConstMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);
	m_emptyLiteralPtr = g_nullDataPtr;
}

void
ConstMgr::clear() {
	m_valueList.clear();
	m_constList.clear();
	m_constBoxList.clear();
	m_emptyLiteralPtr = g_nullDataPtr;
}

Const*
ConstMgr::createConst(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	const Value& value
) {
	Const* cnst = new Const;
	cnst->m_name = name;
	cnst->m_qualifiedName = qualifiedName;
	cnst->m_value = value;
	m_constList.insertTail(cnst);

	return cnst;
}

const Value&
ConstMgr::saveLiteral(const sl::StringRef& string) {
	Value value;
	value.setCharArray(string, m_module);
	return saveValue(value);
}

DataPtrValidator*
ConstMgr::createConstDataPtrValidator(
	const void* p,
	Type* type
) {
	DetachedDataBox* box = m_constBoxList.insertTail().p();
	box->m_box.m_type = type;
	box->m_box.m_flags = BoxFlag_Detached | BoxFlag_Static | BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_box.m_rootOffset = 0;
	box->m_validator.m_validatorBox = &box->m_box;
	box->m_validator.m_targetBox = &box->m_box;
	box->m_validator.m_rangeBegin = p;
	box->m_validator.m_rangeEnd = (char*)p + type->getSize();
	box->m_p = (void*)p;

	return &box->m_validator;
}

DataPtr
ConstMgr::createEmptyLiteralPtr() {
	ASSERT(!m_emptyLiteralPtr.m_p);
	const Value& value = saveLiteral(sl::StringRef());
	void* p = (void*)value.getConstData();
	m_emptyLiteralPtr.m_p = p;
	m_emptyLiteralPtr.m_validator = createConstDataPtrValidator(p, value.getType());
	return m_emptyLiteralPtr;
}

//..............................................................................

} // namespace ct
} // namespace jnc
