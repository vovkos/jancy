#include "pch.h"
#include "jnc_rtl_Multicast.h"
#include "jnc_rtl_CoreLib.h"

namespace jnc {
namespace rtl {

//.............................................................................

void
MulticastImpl::destruct ()
{
	if (m_handleTable)
	{
		AXL_MEM_DELETE ((sl::HandleTable <size_t>*) m_handleTable);
		m_handleTable = NULL;
	}

	m_count = 0;
}

void
MulticastImpl::clear ()
{
	if (m_handleTable)
		((sl::HandleTable <size_t>*) m_handleTable)->clear ();

	m_count = 0;
}

handle_t
MulticastImpl::setHandler (rt::FunctionPtr ptr)
{
	if (ptr.m_p)
		return setHandlerImpl (ptr);

	clear ();
	return NULL;
}

handle_t
MulticastImpl::setHandler_t (void* p)
{
	if (p)
		return setHandlerImpl (p);

	clear ();
	return NULL;
}

sl::HandleTable <size_t>*
MulticastImpl::getHandleTable ()
{
	if (m_handleTable)
		return (sl::HandleTable <size_t>*) m_handleTable;

	sl::HandleTable <size_t>* handleTable = AXL_MEM_NEW (sl::HandleTable <size_t>);
	m_handleTable = handleTable;
	return handleTable;
}

bool
MulticastImpl::setCount (
	size_t count,
	size_t ptrSize
	)
{
	if (m_maxCount >= count)
	{
		m_count = count;
		return true;
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ASSERT (isClassType (m_box->m_type, ct::ClassTypeKind_Multicast));
	ct::MulticastClassType* multicastType = (ct::MulticastClassType*) m_box->m_type;

	size_t maxCount = sl::getMinPower2Ge (count);
	rt::DataPtr ptr = runtime->m_gcHeap.tryAllocateArray (multicastType->getTargetType (), maxCount);
	if (!ptr.m_p)
		return false;

	if (m_count)
		memcpy (ptr.m_p, m_ptr.m_p, m_count * ptrSize);

	m_ptr = ptr;
	m_count = count;
	m_maxCount = maxCount;
	return true;
}

rt::FunctionPtr
MulticastImpl::getSnapshot ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::ScopedNoCollectRegion noCollectRegion (runtime, false);

	ASSERT (isClassType (m_box->m_type, ct::ClassTypeKind_Multicast));
	ct::MulticastClassType* multicastType = (ct::MulticastClassType*) m_box->m_type;
	ct::McSnapshotClassType* snapshotType = multicastType->getSnapshotType ();
	ct::FunctionPtrType* targetType = multicastType->getTargetType ();
	rt::McSnapshot* snapshot = (rt::McSnapshot*) runtime->m_gcHeap.allocateClass (snapshotType);

	rt::FunctionPtr resultPtr;
	resultPtr.m_p = snapshotType->getMethod (ct::McSnapshotMethodKind_Call)->getMachineCode ();
	resultPtr.m_closure = snapshot;

	if (!m_count)
		return resultPtr;

	snapshot->m_ptr = runtime->m_gcHeap.tryAllocateArray (targetType, m_count);
	if (!snapshot->m_ptr.m_p)
		return resultPtr;

	size_t targetTypeSize = targetType->getSize ();

	if (multicastType->getTargetType ()->getPtrTypeKind () != ct::FunctionPtrTypeKind_Weak)
	{
		snapshot->m_count = m_count;
		memcpy (snapshot->m_ptr.m_p, m_ptr.m_p, m_count * targetTypeSize);
		return resultPtr;
	}

	rt::FunctionPtr* dstPtr = (rt::FunctionPtr*) snapshot->m_ptr.m_p;
	rt::FunctionPtr* srcPtr = (rt::FunctionPtr*) m_ptr.m_p;
	rt::FunctionPtr* srcPtrEnd = srcPtr + m_count;

	size_t aliveCount = 0;
	for (; srcPtr < srcPtrEnd; srcPtr++)
	{
		if (CoreLib::strengthenClassPtr (srcPtr->m_closure))
		{
			*dstPtr = *srcPtr;
			dstPtr++;
			aliveCount++;
		}
	}

	if (aliveCount != m_count) // remove dead pointers from multicast
	{
		size_t oldSize = m_count * targetTypeSize;
		size_t aliveSize = aliveCount * targetTypeSize;

		memcpy (m_ptr.m_p, snapshot->m_ptr.m_p, aliveSize);
		memset ((char*) m_ptr.m_p + aliveSize, 0, oldSize - aliveSize);

		m_count = aliveCount;
	}

	snapshot->m_count = aliveCount;

	return resultPtr;
}

//.............................................................................

} // namespace rtl
} // namespace jnc
