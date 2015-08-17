// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_MulticastClassType.h"
#include "jnc_Value.h"

namespace jnc {

class MulticastClassType;

//.............................................................................

class MulticastLib
{
protected:
	static void* m_multicastMethodTable [FunctionPtrTypeKind__Count] [MulticastMethodKind__Count - 1];

public:
	static
	bool
	mapFunctions (Module* runtime);

	static
	void
	multicastDestruct (Multicast* multicast);

	static
	void
	multicastClear (Multicast* multicast);

	static
	handle_t
	multicastSet (
		Multicast* multicast,
		FunctionPtr ptr
		);

	static
	handle_t
	multicastSet_t (
		Multicast* multicast,
		void* p
		);

	static
	handle_t
	multicastAdd (
		Multicast* multicast,
		FunctionPtr ptr
		);

	static
	handle_t
	multicastAdd_t (
		Multicast* multicast,
		void* p
		);

	static
	FunctionPtr
	multicastRemove (
		Multicast* multicast,
		handle_t handle
		);

	static
	void*
	multicastRemove_t (
		Multicast* multicast,
		handle_t handle
		);

	static
	FunctionPtr
	multicastGetSnapshot (Multicast* multicast);

protected:
	static
	void
	mapMulticastMethods (
		Module* module,
		MulticastClassType* multicastType
		);
};

//.............................................................................

} // namespace jnc {
