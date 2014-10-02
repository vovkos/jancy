#include "pch.h"
#include "jnc_MulticastLib.h"
#include "jnc_Multicast.h"
#include "jnc_Runtime.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
MulticastLib::mapFunctions (Module* module)
{
	rtl::ConstList <MulticastClassType> mcTypeList = module->m_typeMgr.getMulticastClassTypeList ();
	rtl::Iterator <MulticastClassType> mcType = mcTypeList.getHead ();
	for (; mcType; mcType++)
		mapMulticastMethods (module, *mcType);

	return true;
}

void
MulticastLib::multicastClear (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->clear ();
}

handle_t
MulticastLib::multicastSet (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->setHandler (ptr);
}

handle_t
MulticastLib::multicastSet_t (
	Multicast* multicast,
	void* pf
	)
{
	return ((MulticastImpl*) multicast)->setHandler_t (pf);
}

handle_t
MulticastLib::multicastAdd (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->addHandler (ptr);
}

handle_t
MulticastLib::multicastAdd_t (
	Multicast* multicast,
	void* pf
	)
{
	return ((MulticastImpl*) multicast)->addHandler_t (pf);
}

FunctionPtr
MulticastLib::multicastRemove (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler (handle);
}

void*
MulticastLib::multicastRemove_t (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler_t (handle);
}

FunctionPtr
MulticastLib::multicastGetSnapshot (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->getSnapshot ();
}

void*
MulticastLib::m_multicastMethodTable [FunctionPtrTypeKind__Count] [MulticastMethodKind__Count - 1] =
{
	{
		(void*) multicastClear,
		(void*) multicastSet,
		(void*) multicastAdd,
		(void*) multicastRemove,
		(void*) multicastGetSnapshot,
	},

	{
		(void*) multicastClear,
		(void*) multicastSet,
		(void*) multicastAdd,
		(void*) multicastRemove,
		(void*) multicastGetSnapshot,
	},

	{
		(void*) multicastClear,
		(void*) multicastSet_t,
		(void*) multicastAdd_t,
		(void*) multicastRemove_t,
		(void*) multicastGetSnapshot,
	},
};

void
MulticastLib::mapMulticastMethods (
	Module* module,
	MulticastClassType* multicastType
	)
{
	FunctionPtrTypeKind ptrTypeKind = multicastType->getTargetType ()->getPtrTypeKind ();
	ASSERT (ptrTypeKind < FunctionPtrTypeKind__Count);

	for (size_t i = 0; i < MulticastMethodKind__Count - 1; i++)
	{
		Function* function = multicastType->getMethod ((MulticastMethodKind) i);

		module->mapFunction (
			function->getLlvmFunction (),
			m_multicastMethodTable [ptrTypeKind] [i]
			);
	}
}

//.............................................................................

} // namespace jnc {
