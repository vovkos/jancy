#include "pch.h"
#include "jnc_ScheduleLauncherFunction.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
ScheduleLauncherFunction::compile ()
{
	bool result;

	size_t argCount = m_type->getArgArray ().getCount ();

	char buffer [256];
	rtl::Array <Value> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argValueArray.setCount (argCount);

	m_module->m_functionMgr.internalPrologue (this, argValueArray, argCount);

	Value scheduleValue;
	result = m_module->m_operatorMgr.memberOperator (argValueArray [1], "schedule", &scheduleValue);
	if (!result)
		return false;

	Value functionPtrValue = argValueArray [0];
	if (argCount > 2)
	{
		rtl::BoxList <Value> argList;
		for (size_t i = 2; i < argCount; i++)
			argList.insertTail (argValueArray [i]);

		result = m_module->m_operatorMgr.closureOperator (&functionPtrValue, &argList);
		if (!result)
			return false;
	}

	result = m_module->m_operatorMgr.callOperator (scheduleValue, functionPtrValue);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
