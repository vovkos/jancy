#include "pch.h"
#include "jnc_MulticastLib.h"
#include "jnc_Multicast.h"
#include "jnc_Runtime.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
CMulticastLib::Export (CRuntime* pRuntime)
{
	CModule* pModule = pRuntime->GetModule ();
	EJit JitKind = pRuntime->GetJitKind ();

	rtl::CConstListT <CMulticastClassType> McTypeList = pModule->m_TypeMgr.GetMulticastClassTypeList ();
	rtl::CIteratorT <CMulticastClassType> McType = McTypeList.GetHead ();
	for (; McType; McType++)
		MapMulticastMethods (pRuntime, *McType);

	return true;
}

void
CMulticastLib::MulticastClear (TMulticast* pMulticast)
{
	return ((CMulticast*) pMulticast)->Clear ();
}

handle_t
CMulticastLib::MulticastSet (
	TMulticast* pMulticast,
	TFunctionPtr Ptr
	)
{
	return ((CMulticast*) pMulticast)->SetHandler (Ptr);
}

handle_t
CMulticastLib::MulticastSet_t (
	TMulticast* pMulticast,
	void* pf
	)
{
	return ((CMulticast*) pMulticast)->SetHandler_t (pf);
}

handle_t
CMulticastLib::MulticastAdd (
	TMulticast* pMulticast,
	TFunctionPtr Ptr
	)
{
	return ((CMulticast*) pMulticast)->AddHandler (Ptr);
}

handle_t
CMulticastLib::MulticastAdd_t (
	TMulticast* pMulticast,
	void* pf
	)
{
	return ((CMulticast*) pMulticast)->AddHandler_t (pf);
}

TFunctionPtr
CMulticastLib::MulticastRemove (
	TMulticast* pMulticast,
	handle_t Handle
	)
{
	return ((CMulticast*) pMulticast)->RemoveHandler (Handle);
}

void*
CMulticastLib::MulticastRemove_t (
	TMulticast* pMulticast,
	handle_t Handle
	)
{
	return ((CMulticast*) pMulticast)->RemoveHandler_t (Handle);
}

TFunctionPtr
CMulticastLib::MulticastGetSnapshot (TMulticast* pMulticast)
{
	return ((CMulticast*) pMulticast)->GetSnapshot ();
}

void*
CMulticastLib::m_MulticastMethodTable [EFunctionPtrType__Count] [EMulticastMethod__Count - 1] =
{
	{
		(void*) MulticastClear,
		(void*) MulticastSet,
		(void*) MulticastAdd,
		(void*) MulticastRemove,
		(void*) MulticastGetSnapshot,
	},

	{
		(void*) MulticastClear,
		(void*) MulticastSet,
		(void*) MulticastAdd,
		(void*) MulticastRemove,
		(void*) MulticastGetSnapshot,
	},

	{
		(void*) MulticastClear,
		(void*) MulticastSet_t,
		(void*) MulticastAdd_t,
		(void*) MulticastRemove_t,
		(void*) MulticastGetSnapshot,
	},
};

void
CMulticastLib::MapMulticastMethods (
	CRuntime* pRuntime,
	CMulticastClassType* pMulticastType
	)
{
	EFunctionPtrType PtrTypeKind = pMulticastType->GetTargetType ()->GetPtrTypeKind ();
	ASSERT (PtrTypeKind < EFunctionPtrType__Count);

	for (size_t i = 0; i < EMulticastMethod__Count - 1; i++)
	{
		CFunction* pFunction = pMulticastType->GetMethod ((EMulticastMethod) i);

		pRuntime->MapFunction (
			pFunction->GetLlvmFunction (),
			m_MulticastMethodTable [PtrTypeKind] [i]
			);
	}
}

//.............................................................................

} // namespace jnc {
