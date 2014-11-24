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
	m_scopeLevel = 0;
	m_tlsField = NULL;
	m_llvmValue = NULL;
	m_llvmAllocValue = NULL;
	m_llvmObjHdrValue = NULL;
}

Value
Variable::getObjHdr ()
{
	if (!m_llvmObjHdrValue)
	{
		m_module->m_variableMgr.allocateVariableObjHdr (this);
		ASSERT (m_llvmObjHdrValue);
	}

	return Value (m_llvmObjHdrValue, m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr));
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
