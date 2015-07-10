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
	m_llvmBoxValue = NULL;
	m_llvmDataPtrValidatorValue = NULL;
}

Value
Variable::getBox ()
{
	if (!m_llvmBoxValue)
	{
		m_module->m_variableMgr.allocateVariableBox (this);
		ASSERT (m_llvmBoxValue);
	}

	return Value (m_llvmBoxValue, m_module->m_typeMgr.getStdType (StdType_BoxPtr));
}

bool
Variable::calcLayout ()
{
	if (m_type_i)
		m_type = m_type_i->getActualType ();

	return m_type->ensureLayout ();
}

void
Variable::ensureLlvmValue ()
{
	if (m_llvmValue)
		return;

	ASSERT (m_storageKind == StorageKind_Thread);
	m_module->m_variableMgr.allocateTlsVariable (this);
}

//.............................................................................

} // namespace jnc {
