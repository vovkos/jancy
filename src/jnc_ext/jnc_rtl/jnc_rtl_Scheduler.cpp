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
#include "jnc_rtl_Scheduler.h"
#include "jnc_rtl_Promise.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_CLASS_TYPE(
	SchedulerImpl,
	"jnc.Scheduler",
	sl::g_nullGuid,
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SchedulerImpl)
	JNC_MAP_FUNCTION("asyncWait", &SchedulerImpl::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Promise*
JNC_CDECL
SchedulerImpl::asyncWait()
{
	Runtime* runtime = getCurrentThreadRuntime();
	ASSERT(runtime);

	Module* module = runtime->getModule();
	ClassType* promisifierType = (ClassType*) module->m_typeMgr.getStdType(StdType_Promisifier);
	Function* promisifierConstructor = promisifierType->getConstructor();
	ASSERT(promisifierConstructor);

	Promisifier* promisifier;

	JNC_BEGIN_CALL_SITE(runtime)

	promisifier = (Promisifier*)runtime->getGcHeap()->allocateClass(promisifierType);
	((ConstructFunc*)promisifierConstructor->getMachineCode())(promisifier);

	FunctionPtr completeFuncPtr;
	completeFuncPtr.m_p = jnc_pvoid_cast(&Promisifier::complete_0);
	completeFuncPtr.m_closure = promisifier;
	((SchedulerVtable*) m_ifaceHdr.m_vtable)->m_scheduleFunc(this, completeFuncPtr);

	JNC_END_CALL_SITE();

	return promisifier;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
