#include "pch.h"
#include "jnc_ScheduleLauncherFunction.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
CScheduleLauncherFunction::Compile ()
{
	bool Result;

	size_t ArgCount = m_pType->GetArgArray ().GetCount ();

	char Buffer [256];
	rtl::CArrayT <CValue> ArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgValueArray.SetCount (ArgCount);

	m_pModule->m_FunctionMgr.InternalPrologue (this, ArgValueArray, ArgCount);

	CValue ScheduleValue;
	Result = m_pModule->m_OperatorMgr.MemberOperator (ArgValueArray [1], "schedule", &ScheduleValue);
	if (!Result)
		return false;

	CValue FunctionPtrValue = ArgValueArray [0];
	if (ArgCount > 2)
	{
		rtl::CBoxListT <CValue> ArgList;
		for (size_t i = 2; i < ArgCount; i++)
			ArgList.InsertTail (ArgValueArray [i]);

		Result = m_pModule->m_OperatorMgr.ClosureOperator (&FunctionPtrValue, &ArgList);
		if (!Result)
			return false;
	}

	Result = m_pModule->m_OperatorMgr.CallOperator (ScheduleValue, FunctionPtrValue);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
