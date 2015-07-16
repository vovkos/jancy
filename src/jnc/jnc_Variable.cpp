#include "pch.h"
#include "jnc_Variable.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Variable::Variable ()
{
	m_itemKind = ModuleItemKind_Variable;
	m_type = NULL;
	m_type_i = NULL;
	m_ptrTypeFlags = 0;
	m_scope = NULL;
	m_tlsField = NULL;
	m_llvmValue = NULL;
}

LeanDataPtrValidator*
Variable::getLeanDataPtrValidator ()
{
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;
	
	Value originValue (this);

	m_leanDataPtrValidator = AXL_REF_NEW (LeanDataPtrValidator);
	m_leanDataPtrValidator->m_originValue = originValue;
	m_leanDataPtrValidator->m_rangeBeginValue = originValue;
	m_leanDataPtrValidator->m_rangeLength = m_type->getSize ();
	return m_leanDataPtrValidator;
}

bool
Variable::calcLayout ()
{
	if (m_type_i)
		m_type = m_type_i->getActualType ();

	return m_type->ensureLayout ();
}

//.............................................................................

} // namespace jnc {
