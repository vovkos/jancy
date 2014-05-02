// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_MulticastClassType.h"
#include "jnc_Value.h"

namespace jnc {

class CMulticastClassType;

//.............................................................................

class CMulticastLib
{
protected:
	static void* m_MulticastMethodTable [EFunctionPtrType__Count] [EMulticastMethod__Count - 1];

public:
	static
	bool
	Export (CModule* pRuntime);

	static
	void
	MulticastClear (TMulticast* pMulticast);

	static
	handle_t
	MulticastSet (
		TMulticast* pMulticast,
		TFunctionPtr Ptr
		);

	static
	handle_t
	MulticastSet_t (
		TMulticast* pMulticast,
		void* pf
		);

	static
	handle_t
	MulticastAdd (
		TMulticast* pMulticast,
		TFunctionPtr Ptr
		);

	static
	handle_t
	MulticastAdd_t (
		TMulticast* pMulticast,
		void* pf
		);

	static
	TFunctionPtr
	MulticastRemove (
		TMulticast* pMulticast,
		handle_t Handle
		);

	static
	void*
	MulticastRemove_t (
		TMulticast* pMulticast,
		handle_t Handle
		);

	static
	TFunctionPtr
	MulticastGetSnapshot (TMulticast* pMulticast);

protected:
	static
	void
	MapMulticastMethods (
		CModule* pModule,
		CMulticastClassType* pMulticastType
		);

	static
	void
	ExportMulticastMethods (
		CModule* pModule,
		llvm::ExecutionEngine* pLlvmExecutionEngine,
		CMulticastClassType* pMulticastType
		);
};

//.............................................................................

} // namespace jnc {
