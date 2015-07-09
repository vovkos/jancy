#include "pch.h"
#include "jnc_ConstMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ConstMgr::ConstMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
ConstMgr::clear ()
{
	m_valueList.clear ();
	m_constList.clear ();
	m_constDataPtrValidatorList.clear ();
}

Const*
ConstMgr::createConst (
	const rtl::String& name,
	const rtl::String& qualifiedName,
	const Value& value
	)
{
	Const* cnst = AXL_MEM_NEW (Const);
	cnst->m_name = name;
	cnst->m_qualifiedName = qualifiedName;
	cnst->m_tag = qualifiedName;
	cnst->m_value = value;
	m_constList.insertTail (cnst);

	return cnst;
}

const Value& 
ConstMgr::saveLiteral (
	const char* p,
	size_t length
	)
{
	if (length == -1)
		length = strlen (p);

	Value value;
	value.setCharArray (p, length + 1, m_module);
	return saveValue (value);
}

//.............................................................................

} // namespace jnc {
