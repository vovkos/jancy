#include "pch.h"
#include "jnc_sys_Thread.h"

namespace jnc {
namespace sys {

//.............................................................................

bool
AXL_CDECL
Thread::start (rt::FunctionPtr ptr)
{
	bool result;

	if (m_thread.isOpen ())
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	if (!ptr.m_p)
	{
		err::setError (err::SystemErrorCode_InvalidParameter);
		return false;
	}

	m_threadFuncPtr = ptr;
	result = m_thread.start ();
	if (!result)
	{
		m_threadFuncPtr = rt::g_nullFunctionPtr;
		return false;
	}

	return true;
}

bool
AXL_CDECL
Thread::wait (uint_t timeout)
{
	bool result;

	rt::enterWaitRegion (m_runtime);
	result = m_thread.wait (timeout);
	rt::leaveWaitRegion (m_runtime);

	return result;
}

void
AXL_CDECL
Thread::waitAndClose (uint_t timeout)
{
	rt::enterWaitRegion (m_runtime);
	m_thread.waitAndClose (timeout);
	rt::leaveWaitRegion (m_runtime);


	m_threadFuncPtr = rt::g_nullFunctionPtr;
}

//.............................................................................

} // namespace sys
} // namespace jnc
