#include "pch.h"
#include "jnc_Multicast.h"
#include "jnc_CoreLib.h"

namespace jnc {

//.............................................................................

MulticastImpl::~MulticastImpl ()
{
	if (m_handleTable)
	{
		AXL_MEM_DELETE ((rtl::HandleTable <size_t>*) m_handleTable);
		m_handleTable = NULL;
	}

	m_count = 0;
}

void
MulticastImpl::clear ()
{
	if (m_handleTable)
		((rtl::HandleTable <size_t>*) m_handleTable)->clear ();

	m_count = 0;
}

handle_t
MulticastImpl::setHandler (FunctionPtr ptr)
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

rtl::HandleTable <size_t>*
MulticastImpl::getHandleTable ()
{
	if (m_handleTable)
		return (rtl::HandleTable <size_t>*) m_handleTable;

	rtl::HandleTable <size_t>* handleTable = AXL_MEM_NEW (rtl::HandleTable <size_t>);
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

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ASSERT (isClassType (m_box->m_type, ClassTypeKind_Multicast));
	MulticastClassType* multicastType = (MulticastClassType*) m_box->m_type;

	size_t maxCount = rtl::getMinPower2Ge (count);
	DataPtr ptr = runtime->m_gcHeap.tryAllocateArray (multicastType->getTargetType (), maxCount);
	if (!ptr.m_p)
		return false;

	if (m_count)
		memcpy (ptr.m_p, m_ptr.m_p, m_count * ptrSize);

	m_ptr = ptr;
	m_count = count;
	m_maxCount = maxCount;
	return true;
}

FunctionPtr
MulticastImpl::getSnapshot ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ScopedNoCollectRegion noCollectRegion (&runtime->m_gcHeap, false);

	ASSERT (isClassType (m_box->m_type, ClassTypeKind_Multicast));
	MulticastClassType* multicastType = (MulticastClassType*) m_box->m_type;
	McSnapshotClassType* snapshotType = multicastType->getSnapshotType ();
	FunctionPtrType* targetType = multicastType->getTargetType ();
	McSnapshot* snapshot = (McSnapshot*) runtime->m_gcHeap.allocateClass (snapshotType);

	FunctionPtr resultPtr;
	resultPtr.m_p = snapshotType->getMethod (McSnapshotMethodKind_Call)->getMachineCode ();
	resultPtr.m_closure = snapshot;

	if (!m_count)
		return resultPtr;

	snapshot->m_ptr = runtime->m_gcHeap.tryAllocateArray (targetType, m_count);
	if (!snapshot->m_ptr.m_p)
		return resultPtr;

	size_t targetTypeSize = targetType->getSize ();

	if (multicastType->getTargetType ()->getPtrTypeKind () != FunctionPtrTypeKind_Weak)
	{
		snapshot->m_count = m_count;
		memcpy (snapshot->m_ptr.m_p, m_ptr.m_p, m_count * targetTypeSize);
		return resultPtr;
	}

	FunctionPtr* dstPtr = (FunctionPtr*) snapshot->m_ptr.m_p;
	FunctionPtr* srcPtr = (FunctionPtr*) m_ptr.m_p;
	FunctionPtr* srcPtrEnd = srcPtr + m_count;

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

} // namespace jnc
