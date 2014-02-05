#include "pch.h"
#include "jnc_Multicast.h"
#include "jnc_Runtime.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

void
CMulticast::Clear ()
{
	m_Count = 0;
	if (m_pHandleTable)
		((rtl::CHandleTableT <size_t>*) m_pHandleTable)->Clear ();
}

handle_t
CMulticast::SetHandler (TFunctionPtr Ptr)
{
	if (Ptr.m_pf)
		return SetHandlerImpl (Ptr);

	Clear ();
	return NULL;
}

handle_t
CMulticast::SetHandler_t (void* pf)
{
	if (pf)
		return SetHandlerImpl (pf);

	Clear ();
	return NULL;
}

rtl::CHandleTableT <size_t>*
CMulticast::GetHandleTable ()
{
	if (m_pHandleTable)
		return (rtl::CHandleTableT <size_t>*) m_pHandleTable;

	rtl::CHandleTableT <size_t>* pHandleTable = AXL_MEM_NEW (rtl::CHandleTableT <size_t>);
	m_pHandleTable = pHandleTable;
	return pHandleTable;
}

bool
CMulticast::SetCount (
	size_t Count,
	size_t PtrSize
	)
{
	if (m_MaxCount >= Count)
	{
		m_Count = Count;
		return true;
	}

	size_t MaxCount = rtl::GetMinPower2Ge (Count);

	void* p = AXL_MEM_ALLOC (MaxCount * PtrSize);
	if (!p)
		return false;

	if (m_pPtrArray)
		memcpy (p, m_pPtrArray, m_Count * PtrSize);

	m_pPtrArray = p;
	m_MaxCount = MaxCount;
	m_Count = Count;
	return true;
}

struct TMcSnapshotObject:
	TObjHdr,
	TMcSnapshot
{
};

TFunctionPtr
CMulticast::GetSnapshot ()
{
	CRuntime* pRuntime = GetCurrentThreadRuntime ();
	ASSERT (pRuntime);

	ASSERT (m_pObject->m_pClassType->GetClassTypeKind () == EClassType_Multicast);
	CMulticastClassType* pMulticastType = (CMulticastClassType*) m_pObject->m_pType;
	CMcSnapshotClassType* pSnapshotType = pMulticastType->GetSnapshotType ();

//	TMcSnapshotObject* pSnapshot = (TMcSnapshotObject*) AXL_MEM_NEW (pRuntime->GcAllocate (pSnapshotType);
	TMcSnapshotObject* pSnapshot = AXL_MEM_NEW (TMcSnapshotObject);
	pSnapshot->m_ScopeLevel = 0;
	pSnapshot->m_pRoot = pSnapshot;
	pSnapshot->m_pType = pSnapshotType;
	pSnapshot->m_pObject = pSnapshot;
	pSnapshot->m_Flags = 0;

	size_t Size = pMulticastType->GetTargetType ()->GetSize () * m_Count;
	if (Size)
	{
		pSnapshot->m_pPtrArray = AXL_MEM_ALLOC (Size);

		if (pMulticastType->GetTargetType ()->GetPtrTypeKind () == EFunctionPtrType_Weak)
		{
			TFunctionPtr* pDstPtr = (TFunctionPtr*) pSnapshot->m_pPtrArray;
			TFunctionPtr* pSrcPtr = (TFunctionPtr*) m_pPtrArray;
			TFunctionPtr* pSrcPtrEnd = pSrcPtr + m_Count;

			size_t AliveCount = 0;
			for (; pSrcPtr < pSrcPtrEnd; pSrcPtr++)
			{
				if (CStdLib::StrengthenClassPtr (pSrcPtr->m_pClosure))
				{
					*pDstPtr = *pSrcPtr;
					pDstPtr++;
					AliveCount++;
				}
			}

			pSnapshot->m_Count = AliveCount;
		}
		else
		{
			pSnapshot->m_Count = m_Count;
			memcpy (pSnapshot->m_pPtrArray, m_pPtrArray, Size);
		}
	}

	TFunctionPtr Ptr = { 0 };
	Ptr.m_pClosure = pSnapshot;
	Ptr.m_pf = pSnapshotType->GetMethod (EMcSnapshotMethod_Call)->GetMachineCode ();

	ASSERT (Ptr.m_pf);
	return Ptr;
}

//.............................................................................

} // namespace jnc
