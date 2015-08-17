#include "pch.h"
#include "jnc_MulticastClassType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

MulticastClassType::MulticastClassType ()
{
	m_classTypeKind = ClassTypeKind_Multicast;
	m_targetType = NULL;
	m_snapshotType = NULL;
	m_eventClassPtrTypeTuple = NULL;
	memset (m_fieldArray, 0, sizeof (m_fieldArray));
	memset (m_methodArray, 0, sizeof (m_methodArray));
}

void
MulticastClassType::prepareTypeString ()
{
	m_typeString = m_targetType->getTypeModifierString ();
	m_typeString.appendFormat ("multicast %s", m_targetType->getTargetType ()->getArgString ().cc ());
}

bool
MulticastClassType::compileCallMethod ()
{
	bool result;

	Function* function = m_methodArray [MulticastMethodKind_Call];

	size_t argCount = function->getType ()->getArgArray ().getCount ();

	char buffer [256];
	rtl::Array <Value> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argValueArray.setCount (argCount);

	m_module->m_functionMgr.internalPrologue (function, argValueArray, argCount);

	Function* getSnapshot = m_methodArray [MulticastMethodKind_GetSnapshot];

	Value snapshotValue;
	result = m_module->m_operatorMgr.callOperator (getSnapshot, argValueArray [0], &snapshotValue);
	if (!result)
		return false;

	rtl::BoxList <Value> argList;
	for (size_t i = 1; i < argCount; i++)
		argList.insertTail (argValueArray [i]);

	m_module->m_operatorMgr.callOperator (snapshotValue, &argList);

	m_module->m_controlFlowMgr.ret ();

	m_module->m_functionMgr.internalEpilogue ();

	return true;
}

//.............................................................................
	
} // namespace jnc {
