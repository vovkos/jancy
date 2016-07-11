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
		JNC_MAP_STD_FUNCTION (ct::StdFunc_SetGcShadowStackFrameMap, setGcShadowStackFrameMap)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AddStaticDestructor,      addStaticDestructor)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_AddStaticClassDestructor, addStaticClassDestructor)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_GetTls,                   getTls)

		// variant operators

		JNC_MAP_STD_FUNCTION (ct::StdFunc_VariantUnaryOperator,      variantUnaryOperator)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_VariantBinaryOperator,     variantBinaryOperator)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_VariantRelationalOperator, variantRelationalOperator)

		// exceptions

		JNC_MAP_STD_FUNCTION (ct::StdFunc_SetJmp,       ::setjmp)
		JNC_MAP_STD_FUNCTION (ct::StdFunc_DynamicThrow, dynamicThrow)

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
	dynamicSizeOf (DataPtr ptr);

	static
	size_t
	dynamicCountOf (
		DataPtr ptr,
		ct::Type* type
		);

	static
	DataPtr
	dynamicCastDataPtr (
		DataPtr ptr,
		ct::Type* type
		);

	static
	IfaceHdr*
	dynamicCastClassPtr (
		IfaceHdr* iface,
		ct::ClassType* type
		);

	static
	bool
	dynamicCastVariant (
		Variant variant,
		ct::Type* type,
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
		ct::ClassType* type
		);

	static
	IfaceHdr*
	tryAllocateClass (ct::ClassType* type);

	static
	IfaceHdr*
	allocateClass (ct::ClassType* type);

	static
	DataPtr
	tryAllocateData (ct::Type* type);

	static
	DataPtr
	allocateData (ct::Type* type);

	static
	DataPtr
	tryAllocateArray (
		ct::Type* type,
		size_t elementCount
		);

	static
	DataPtr
	allocateArray (
		ct::Type* type,
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
	setGcShadowStackFrameMap (
		GcShadowStackFrame* frame,
		GcShadowStackFrameMap* map,
		bool isOpen
		);

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

	// exceptions

	static
	void
	dynamicThrow();

	// variant operators

	static
	Variant
	variantUnaryOperator (
		int opKind,
		Variant op
		);

	static
	Variant
	variantBinaryOperator (
		int opKind,
		Variant op1,
		Variant op2
		);

	static
	bool
	variantRelationalOperator (
		int opKind,
		Variant op1,
		Variant op2
		);

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
		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, (const char*) ptr.m_p, rt::strLen (ptr));
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
ext::ExtensionLib*
getCoreLib (ext::ExtensionLibHost* host)
{
	// no need to assign slot to corelib
	return sl::getSimpleSingleton <CoreLib> ();
}

//.............................................................................

} // namespace rtl
} // namespace jnc
