// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Runtime.h"
#include "jnc_ExtensionLib.h"
#include "jnc_Recognizer.h"
#include "jnc_DynamicLib.h"
#include "jnc_Multicast.h"

#define JNC_MAP_STD_FUNCTION(stdFuncKind, proc) \
	if (module->m_functionMgr.isStdFunctionUsed (stdFuncKind)) \
	{ \
		function = module->m_functionMgr.getStdFunction (stdFuncKind); \
		ASSERT (function); \
		JNC_MAP (function, proc); \
	}

#define JNC_MAP_STD_TYPE(stdType, Type) \
	if (module->m_typeMgr.isStdTypeUsed (stdType)) \
	{ \
		JNC_MAP_TYPE (Type); \
	}

namespace jnc {
	
//.............................................................................

class CoreLib: public ExtensionLib
{
protected:
	static void* m_multicastMethodTable [FunctionPtrTypeKind__Count] [MulticastMethodKind__Count - 1];

public:
	JNC_BEGIN_LIB_MAP ()
		// dynamic sizeof/countof/casts

		JNC_MAP_STD_FUNCTION (StdFunc_DynamicSizeOf, dynamicSizeOf)
		JNC_MAP_STD_FUNCTION (StdFunc_DynamicCountOf, dynamicCountOf)
		JNC_MAP_STD_FUNCTION (StdFunc_DynamicCastDataPtr, dynamicCastDataPtr)
		JNC_MAP_STD_FUNCTION (StdFunc_DynamicCastClassPtr, dynamicCastClassPtr)
		JNC_MAP_STD_FUNCTION (StdFunc_DynamicCastVariant, dynamicCastVariant)
		JNC_MAP_STD_FUNCTION (StdFunc_StrengthenClassPtr, strengthenClassPtr)

		// gc heap

		JNC_MAP_STD_FUNCTION (StdFunc_PrimeStaticClass, primeStaticClass)
		JNC_MAP_STD_FUNCTION (StdFunc_TryAllocateClass, tryAllocateClass)
		JNC_MAP_STD_FUNCTION (StdFunc_AllocateClass, allocateClass)
		JNC_MAP_STD_FUNCTION (StdFunc_TryAllocateData, tryAllocateData)
		JNC_MAP_STD_FUNCTION (StdFunc_AllocateData, allocateData)
		JNC_MAP_STD_FUNCTION (StdFunc_TryAllocateArray, tryAllocateArray)
		JNC_MAP_STD_FUNCTION (StdFunc_AllocateArray, allocateArray)
		JNC_MAP_STD_FUNCTION (StdFunc_CreateDataPtrValidator, createDataPtrValidator)
		JNC_MAP_STD_FUNCTION (StdFunc_GcSafePoint, gcSafePoint)
		JNC_MAP_STD_FUNCTION (StdFunc_AddStaticDestructor, addStaticDestructor)
		JNC_MAP_STD_FUNCTION (StdFunc_AddStaticClassDestructor, addStaticClassDestructor)
		JNC_MAP_STD_FUNCTION (StdFunc_GetTls, getTls)

		// runtime checks

		JNC_MAP_STD_FUNCTION (StdFunc_AssertionFailure, assertionFailure)		
		JNC_MAP_STD_FUNCTION (StdFunc_TryCheckDataPtrRangeDirect, tryCheckDataPtrRangeDirect)
		JNC_MAP_STD_FUNCTION (StdFunc_CheckDataPtrRangeDirect, checkDataPtrRangeDirect)
		JNC_MAP_STD_FUNCTION (StdFunc_TryCheckDataPtrRangeIndirect, tryCheckDataPtrRangeIndirect)
		JNC_MAP_STD_FUNCTION (StdFunc_CheckDataPtrRangeIndirect, checkDataPtrRangeIndirect)
		JNC_MAP_STD_FUNCTION (StdFunc_TryCheckNullPtr, tryCheckNullPtr)
		JNC_MAP_STD_FUNCTION (StdFunc_CheckNullPtr, checkNullPtr)
		JNC_MAP_STD_FUNCTION (StdFunc_CheckStackOverflow, checkStackOverflow)
		
		// dynamic libs

		JNC_MAP_STD_FUNCTION (StdFunc_TryLazyGetDynamicLibFunction, tryLazyGetDynamicLibFunction)
		JNC_MAP_STD_FUNCTION (StdFunc_LazyGetDynamicLibFunction, lazyGetDynamicLibFunction)		
		
		// formating literals

		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_a, appendFmtLiteral_a)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_p, appendFmtLiteral_p)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_i32, appendFmtLiteral_i32)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_ui32, appendFmtLiteral_ui32)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_i64, appendFmtLiteral_i64)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_ui64, appendFmtLiteral_ui64)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_f, appendFmtLiteral_f)
		JNC_MAP_STD_FUNCTION (StdFunc_AppendFmtLiteral_v, appendFmtLiteral_v)

		// multicasts

		JNC_MAP_HELPER (mapAllMulticastMethods)

		// std types

		JNC_MAP_STD_TYPE (StdType_Recognizer, Recognizer)
		JNC_MAP_STD_TYPE (StdType_DynamicLib, DynamicLib)	
	JNC_END_LIB_MAP ()

public:
	// dynamic sizeof/countof/casts

	static
	size_t
	dynamicSizeOf (DataPtr ptr)
	{
		return ptr.m_validator ? ptr.m_validator->m_rangeLength : 0;
	}

	static
	size_t
	dynamicCountOf (
		DataPtr ptr,
		Type* type
		);

	static
	DataPtr
	dynamicCastDataPtr (
		DataPtr ptr,
		Type* type
		);

	static
	IfaceHdr*
	dynamicCastClassPtr (
		IfaceHdr* iface,
		ClassType* type
		);

	static
	bool
	dynamicCastVariant (
		Variant variant,
		Type* type,
		void* buffer
		);

	static
	IfaceHdr*
	strengthenClassPtr (IfaceHdr* iface);

	// gc heap

	static
	void
	primeStaticClass (
		Box* box,
		ClassType* type
		);

	static
	IfaceHdr*
	tryAllocateClass (ClassType* type);

	static
	IfaceHdr*
	allocateClass (ClassType* type);

	static
	DataPtr
	tryAllocateData (Type* type);

	static
	DataPtr
	allocateData (Type* type);

	static
	DataPtr
	tryAllocateArray (
		Type* type,
		size_t elementCount
		);

	static
	DataPtr
	allocateArray (
		Type* type,
		size_t elementCount
		);

	static
	DataPtrValidator* 
	createDataPtrValidator (
		Box* box,
		void* rangeBegin,
		size_t rangeLength
		);

	static
	void
	gcSafePoint ();

	static
	void
	addStaticDestructor (StaticDestructFunc* destructFunc);

	static
	void
	addStaticClassDestructor (
		DestructFunc* destructFunc,
		IfaceHdr* iface
		);

	static
	void*
	getTls ();

	// runtime checks

	static
	void
	assertionFailure (
		const char* fileName,
		int line,
		const char* condition,
		const char* message
		);

	static
	bool 
	tryCheckDataPtrRangeDirect (
		const void* p,
		const void* rangeBegin,
		size_t rangeLength
		);

	static
	void 
	checkDataPtrRangeDirect (
		const void* p,
		const void* rangeBegin,
		size_t rangeLength
		);

	static
	bool 
	tryCheckDataPtrRangeIndirect (
		const void* p,
		size_t size,
		DataPtrValidator* validator
		);

	static
	void 
	checkDataPtrRangeIndirect (
		const void* p,
		size_t size,
		DataPtrValidator* validator
		);

	static
	bool 
	tryCheckNullPtr (
		const void* p,
		TypeKind typeKind
		);

	static
	void
	checkNullPtr (
		const void* p,
		TypeKind typeKind
		);

	static
	void
	checkStackOverflow ();

	// dynamic libs

	static
	void* 
	tryLazyGetDynamicLibFunction (
		DynamicLib* lib,
		size_t index,
		const char* name
		);

	static
	void* 
	lazyGetDynamicLibFunction (
		DynamicLib* lib,
		size_t index,
		const char* name
		);


	// formatting literals

	static
	size_t
	appendFmtLiteral_a (
		FmtLiteral* fmtLiteral,
		const char* p,
		size_t length
		);

	static
	size_t
	appendFmtLiteral_p (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		DataPtr ptr
		)
	{
		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, (const char*) ptr.m_p, strLen (ptr));
	}

	static
	size_t
	appendFmtLiteral_i32 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "d", x);
	}

	static
	size_t
	appendFmtLiteral_ui32 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "u", x);
	}

	static
	size_t
	appendFmtLiteral_i64 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "lld", x);
	}

	static
	size_t
	appendFmtLiteral_ui64 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "llu", x);
	}

	static
	size_t
	appendFmtLiteral_f (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		double x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "f", x);
	}

	static
	size_t
	appendFmtLiteral_v (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		Variant variant
		);

	// multicasts

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
	bool
	mapAllMulticastMethods (Module* runtime);

	static
	void
	mapMulticastMethods (
		Module* module,
		MulticastClassType* multicastType
		);

	static
	void
	prepareFormatString (
		rtl::String* formatString,
		const char* fmtSpecifier,
		const char* defaultType
		);

	static
	size_t
	appendFmtLiteralImpl (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		const char* defaultType,
		...
		);

	static
	size_t
	appendFmtLiteralStringImpl (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		const char* p,
		size_t length
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ExtensionLib*
getCoreLib (ExtensionLibSlotDb* slotDb)
{
	// no need to assign slot to corelib
	return rtl::getSimpleSingleton <CoreLib> ();
}

//.............................................................................

} // namespace jnc {
