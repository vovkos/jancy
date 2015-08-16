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

	rtl::ConstList <McSnapshotClassType> mcSnaphotTypeList = module->m_typeMgr.getMcSnapshotClassTypeList ();
	rtl::Iterator <McSnapshotClassType> mcSnapshotType = mcSnaphotTypeList.getHead ();
	for (; mcSnapshotType; mcSnapshotType++)
		mapMcSnapshotMethods (module, *mcSnapshotType);

	return true;
}

void
MulticastLib::multicastDestruct (Multicast* multicast)
{
	((MulticastImpl*) multicast)->~MulticastImpl ();
}

void
MulticastLib::mcSnapshotDestruct (McSnapshot* mcSnapshot)
{
	((McSnapshotImpl*) mcSnapshot)->~McSnapshotImpl ();
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
	void* p
	)
{
	return ((MulticastImpl*) multicast)->setHandler_t (p);
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
	void* p
	)
{
	return ((MulticastImpl*) multicast)->addHandler_t (p);
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

	Function* function = multicastType->getDestructor ();
	module->mapFunction (function, (void*) multicastDestruct);

	for (size_t i = 0; i < MulticastMethodKind__Count - 1; i++)
	{
		function = multicastType->getMethod ((MulticastMethodKind) i);
		module->mapFunction (function, m_multicastMethodTable [ptrTypeKind] [i]);
	}
}

void
MulticastLib::mapMcSnapshotMethods (
	Module* module,
	McSnapshotClassType* mcSnapshotType
	)
{
	Function* function = mcSnapshotType->getDestructor ();
	module->mapFunction (function, (void*) mcSnapshotDestruct);
}

//.............................................................................

} // namespace jnc {
