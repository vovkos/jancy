// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_rtl_Recognizer.h"
#include "jnc_rtl_DynamicLib.h"
#include "jnc_rtl_Multicast.h"
#include "jnc_rt_CallSite.h"

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
namespace rtl {
	
//.............................................................................

class CoreLib: public ext::ExtensionLib
{
protected:
	static void* m_multicastMethodTable [ct::FunctionPtrTypeKind__Count] [ct::MulticastMethodKind__Count - 1];

public:
	JNC_BEGIN_LIB_MAP ()
		// dynamic sizeof/countof/casts

		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicSizeOf,       dynamicSizeOf)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicCountOf,      dynamicCountOf)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicCastDataPtr,  dynamicCastDataPtr)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicCastClassPtr, dynamicCastClassPtr)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicCastVariant,  dynamicCastVariant)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_StrengthenClassPtr,  strengthenClassPtr)

		// gc heap

		JNC_MAP_STD_FUNCTION (ct::StdFunc_PrimeStaticClass,         primeStaticClass)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryAllocateClass,         tryAllocateClass)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AllocateClass,            allocateClass)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryAllocateData,          tryAllocateData)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AllocateData,             allocateData)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryAllocateArray,         tryAllocateArray)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AllocateArray,            allocateArray)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CreateDataPtrValidator,   createDataPtrValidator)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_GcSafePoint,              gcSafePoint)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CollectGarbage,           collectGarbage)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AddStaticDestructor,      addStaticDestructor)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AddStaticClassDestructor, addStaticClassDestructor)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_GetTls,                   getTls)

		// runtime checks

		JNC_MAP_STD_FUNCTION (ct::StdFunc_AssertionFailure,             assertionFailure)		
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryCheckDataPtrRangeDirect,   tryCheckDataPtrRangeDirect)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDataPtrRangeDirect,      checkDataPtrRangeDirect)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryCheckDataPtrRangeIndirect, tryCheckDataPtrRangeIndirect)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDataPtrRangeIndirect,    checkDataPtrRangeIndirect)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryCheckNullPtr,              tryCheckNullPtr)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckNullPtr,                 checkNullPtr)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckStackOverflow,           checkStackOverflow)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDivByZero_i32,           checkDivByZero_i32)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDivByZero_i64,           checkDivByZero_i64)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDivByZero_f32,           checkDivByZero_f32)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_CheckDivByZero_f64,           checkDivByZero_f64)
		
		// dynamic libs

		JNC_MAP_STD_FUNCTION (ct::StdFunc_TryLazyGetDynamicLibFunction, tryLazyGetDynamicLibFunction)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_LazyGetDynamicLibFunction,    lazyGetDynamicLibFunction)		
		
		// formating literals

		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_a,    appendFmtLiteral_a)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_p,    appendFmtLiteral_p)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_i32,  appendFmtLiteral_i32)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_ui32, appendFmtLiteral_ui32)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_i64,  appendFmtLiteral_i64)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_ui64, appendFmtLiteral_ui64)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_f,    appendFmtLiteral_f)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AppendFmtLiteral_v,    appendFmtLiteral_v)

		// multicasts

		JNC_MAP_HELPER (mapAllMulticastMethods)

		// std types

		JNC_MAP_STD_TYPE (ct::StdType_Recognizer, Recognizer)
		JNC_MAP_STD_TYPE (ct::StdType_DynamicLib, DynamicLib)	
	JNC_END_LIB_MAP ()

public:
	// dynamic sizeof/countof/casts

	static
	size_t
	dynamicSizeOf (rt::DataPtr ptr)
	{
		return ptr.m_validator ? ptr.m_validator->m_rangeLength : 0;
	}

	static
	size_t
	dynamicCountOf (
		rt::DataPtr ptr,
		ct::Type* type
		);

	static
	rt::DataPtr
	dynamicCastDataPtr (
		rt::DataPtr ptr,
		ct::Type* type
		);

	static
	rt::IfaceHdr*
	dynamicCastClassPtr (
		rt::IfaceHdr* iface,
		ct::ClassType* type
		);

	static
	bool
	dynamicCastVariant (
		rt::Variant variant,
		ct::Type* type,
		void* buffer
		);

	static
	rt::IfaceHdr*
	strengthenClassPtr (rt::IfaceHdr* iface);

	// gc heap

	static
	void
	primeStaticClass (
		rt::Box* box,
		ct::ClassType* type
		);

	static
	rt::IfaceHdr*
	tryAllocateClass (ct::ClassType* type);

	static
	rt::IfaceHdr*
	allocateClass (ct::ClassType* type);

	static
	rt::DataPtr
	tryAllocateData (ct::Type* type);

	static
	rt::DataPtr
	allocateData (ct::Type* type);

	static
	rt::DataPtr
	tryAllocateArray (
		ct::Type* type,
		size_t elementCount
		);

	static
	rt::DataPtr
	allocateArray (
		ct::Type* type,
		size_t elementCount
		);

	static
	rt::DataPtrValidator* 
	createDataPtrValidator (
		rt::Box* box,
		void* rangeBegin,
		size_t rangeLength
		);

	static
	void
	gcSafePoint ();

	static
	void
	collectGarbage ();

	static
	void
	addStaticDestructor (rt::StaticDestructFunc* destructFunc);

	static
	void
	addStaticClassDestructor (
		rt::DestructFunc* destructFunc,
		rt::IfaceHdr* iface
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
		rt::DataPtrValidator* validator
		);

	static
	void 
	checkDataPtrRangeIndirect (
		const void* p,
		size_t size,
		rt::DataPtrValidator* validator
		);

	static
	bool 
	tryCheckNullPtr (
		const void* p,
		ct::TypeKind typeKind
		);

	static
	void
	checkNullPtr (
		const void* p,
		ct::TypeKind typeKind
		);

	static
	void
	checkStackOverflow ();

	static
	void
	checkDivByZero_i32 (int32_t i);

	static
	void
	checkDivByZero_i64 (int64_t i);

	static
	void
	checkDivByZero_f32 (float f);

	static
	void
	checkDivByZero_f64 (double f);

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
		rt::FmtLiteral* fmtLiteral,
		const char* p,
		size_t length
		);

	static
	size_t
	appendFmtLiteral_p (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		rt::DataPtr ptr
		)
	{
		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, (const char*) ptr.m_p, strLen (ptr));
	}

	static
	size_t
	appendFmtLiteral_i32 (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "d", x);
	}

	static
	size_t
	appendFmtLiteral_ui32 (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "u", x);
	}

	static
	size_t
	appendFmtLiteral_i64 (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "lld", x);
	}

	static
	size_t
	appendFmtLiteral_ui64 (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "llu", x);
	}

	static
	size_t
	appendFmtLiteral_f (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		double x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "f", x);
	}

	static
	size_t
	appendFmtLiteral_v (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		rt::Variant variant
		);

	// multicasts

	static
	void
	multicastDestruct (rt::Multicast* multicast);

	static
	void
	multicastClear (rt::Multicast* multicast);

	static
	handle_t
	multicastSet (
		rt::Multicast* multicast,
		rt::FunctionPtr ptr
		);

	static
	handle_t
	multicastSet_t (
		rt::Multicast* multicast,
		void* p
		);

	static
	handle_t
	multicastAdd (
		rt::Multicast* multicast,
		rt::FunctionPtr ptr
		);

	static
	handle_t
	multicastAdd_t (
		rt::Multicast* multicast,
		void* p
		);

	static
	rt::FunctionPtr
	multicastRemove (
		rt::Multicast* multicast,
		handle_t handle
		);

	static
	void*
	multicastRemove_t (
		rt::Multicast* multicast,
		handle_t handle
		);

	static
	rt::FunctionPtr
	multicastGetSnapshot (rt::Multicast* multicast);

protected:
	static
	bool
	mapAllMulticastMethods (ct::Module* runtime);

	static
	void
	mapMulticastMethods (
		ct::Module* module,
		ct::MulticastClassType* multicastType
		);

	static
	void
	prepareFormatString (
		sl::String* formatString,
		const char* fmtSpecifier,
		const char* defaultType
		);

	static
	size_t
	appendFmtLiteralImpl (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		const char* defaultType,
		...
		);

	static
	size_t
	appendFmtLiteralStringImpl (
		rt::FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		const char* p,
		size_t length
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ext::ExtensionLib*
getCoreLib (ext::ExtensionLibHost* host)
{
	// no need to assign slot to corelib
	return sl::getSimpleSingleton <CoreLib> ();
}

//.............................................................................

} // namespace rtl
} // namespace jnc
