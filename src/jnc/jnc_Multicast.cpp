#include "pch.h"
#include "jnc_Multicast.h"
#include "jnc_Runtime.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

MulticastImpl::~MulticastImpl ()
{
	dbg::trace ("MulticastImpl::~MulticastImpl (%x)\n", this);

	if (m_handleTable)
		AXL_MEM_DELETE ((rtl::HandleTable <size_t>*) m_handleTable);

	if (m_ptrArray)
		AXL_MEM_FREE (m_ptrArray);

	m_handleTable = NULL;
	m_ptrArray = NULL;
	m_count = 0;
	m_maxCount = 0;
}

void
MulticastImpl::clear ()
{
	m_count = 0;
	if (m_handleTable)
		((rtl::HandleTable <size_t>*) m_handleTable)->clear ();
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

	size_t maxCount = rtl::getMinPower2Ge (count);

	void* p = AXL_MEM_ALLOC (maxCount * ptrSize);
	if (!p)
		return false;

	if (m_ptrArray)
	{
		memcpy (p, m_ptrArray, m_count * ptrSize);
		AXL_MEM_FREE (m_ptrArray);
	}

	m_ptrArray = p;
	m_maxCount = maxCount;
	m_count = count;
	return true;
}

FunctionPtr
MulticastImpl::getSnapshot ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ASSERT (isClassType (m_box->m_type, ClassTypeKind_Multicast));
	MulticastClassType* multicastType = (MulticastClassType*) m_box->m_type;
	McSnapshotClassType* snapshotType = multicastType->getSnapshotType ();
	McSnapshot* snapshot = (McSnapshot*) runtime->m_gcHeap.allocateClass (snapshotType);

	size_t size = multicastType->getTargetType ()->getSize () * m_count;
	if (size)
	{
		snapshot->m_ptrArray = AXL_MEM_ALLOC (size);

		if (multicastType->getTargetType ()->getPtrTypeKind () == FunctionPtrTypeKind_Weak)
		{
			FunctionPtr* dstPtr = (FunctionPtr*) snapshot->m_ptrArray;
			FunctionPtr* srcPtr = (FunctionPtr*) m_ptrArray;
			FunctionPtr* srcPtrEnd = srcPtr + m_count;

			size_t aliveCount = 0;
			for (; srcPtr < srcPtrEnd; srcPtr++)
			{
				if (StdLib::strengthenClassPtr (srcPtr->m_closure))
				{
					*dstPtr = *srcPtr;
					dstPtr++;
					aliveCount++;
				}
			}

			snapshot->m_count = aliveCount;
		}
		else
		{
			snapshot->m_count = m_count;
			memcpy (snapshot->m_ptrArray, m_ptrArray, size);
		}
	}

	FunctionPtr ptr = { 0 };
	ptr.m_p = snapshotType->getMethod (McSnapshotMethodKind_Call)->getMachineCode ();
	ptr.m_closure = snapshot;

	ASSERT (ptr.m_p);
	return ptr;
}

//.............................................................................

McSnapshotImpl::~McSnapshotImpl ()
{
	dbg::trace ("McSnapshotImpl::~McSnapshotImpl (%x)\n", this);

	if (m_ptrArray)
		AXL_MEM_FREE (m_ptrArray);

	m_ptrArray = NULL;
	m_count = 0;
}

//.............................................................................

} // namespace jnc
