#include "pch.h"
#include "jnc_ConstMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ConstMgr::ConstMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);
}

void
ConstMgr::clear ()
{
	m_constList.clear ();
	m_unsafeLeanDataPtrValidator = ref::PtrKind_Null;
}

LeanDataPtrValidator*
ConstMgr::getUnsafeLeanDataPtrValidator ()
{
	if (m_unsafeLeanDataPtrValidator)
		return m_unsafeLeanDataPtrValidator;

	void* rangeBegin = NULL;

	m_unsafeLeanDataPtrValidator = AXL_REF_NEW (LeanDataPtrValidator);
	m_unsafeLeanDataPtrValidator->m_sizeValue.setConstSizeT (-1);
	m_unsafeLeanDataPtrValidator->m_scopeValidatorValue.setConstSizeT (0);
	m_unsafeLeanDataPtrValidator->m_rangeBeginValue.createConst (&rangeBegin, m_module->getSimpleType (StdTypeKind_BytePtr));

	return m_unsafeLeanDataPtrValidator;
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
	value.setCharArray (p, length + 1);
	return saveValue (value);
}

//.............................................................................

} // namespace jnc {
