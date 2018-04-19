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
#include "jnc_rtl_Multicast.h"
#include "jnc_Type.h"

#ifdef _JNC_CORE
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

#include "jnc_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

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

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	MulticastClassType* type = (MulticastClassType*) m_ifaceHdr.m_box->m_type;
	ASSERT (isClassType (type, ClassTypeKind_Multicast));

	FunctionPtrType* targetType = type->getTargetType ();
	size_t maxCount = sl::getHiBit (count);
	DataPtr ptr = gcHeap->allocateArray (targetType, maxCount);

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
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	MulticastClassType* type = (MulticastClassType*) m_ifaceHdr.m_box->m_type;
	ASSERT (isClassType (type, ClassTypeKind_Multicast));

	ScopedNoCollectRegion noCollectRegion (gcHeap, false);

	McSnapshotClassType* snapshotType = type->getSnapshotType ();
	FunctionPtrType* targetType = type->getTargetType ();
	McSnapshot* snapshot = (McSnapshot*) gcHeap->allocateClass (snapshotType);

	FunctionPtr resultPtr;
	resultPtr.m_p = snapshotType->getMethod (McSnapshotMethodKind_Call)->getMachineCode ();
	resultPtr.m_closure = (IfaceHdr*) snapshot;

	if (!m_count)
		return resultPtr;

	snapshot->m_ptr = gcHeap->allocateArray (targetType, m_count);

	if (targetType->getPtrTypeKind () != FunctionPtrTypeKind_Weak)
	{
		size_t targetTypeSize = targetType->getSize ();
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
		if (strengthenClassPtr (srcPtr->m_closure))
		{
			*dstPtr = *srcPtr;
			dstPtr++;
			aliveCount++;
		}
	}

	if (aliveCount != m_count) // remove dead pointers from multicast
	{
		size_t oldSize = m_count * sizeof (FunctionPtr);
		size_t aliveSize = aliveCount * sizeof (FunctionPtr);

		memcpy (m_ptr.m_p, snapshot->m_ptr.m_p, aliveSize);
		memset ((char*) m_ptr.m_p + aliveSize, 0, oldSize - aliveSize);

		m_count = aliveCount;
	}

	snapshot->m_count = aliveCount;

	return resultPtr;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
