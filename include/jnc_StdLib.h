// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Runtime.h"
#include "jnc_Api.h"
#include "jnc_Error.h"
#include "jnc_String.h"
#include "jnc_HashTable.h"
#include "jnc_List.h"
#include "jnc_Buffer.h"
#include "jnc_Recognizer.h"
#include "jnc_Library.h"
#include "jnc_MulticastLib.h"

namespace jnc {

//.............................................................................

class StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_STD_FUNCTION (StdFunction_DynamicSizeOf, dynamicSizeOf)
		JNC_STD_FUNCTION (StdFunction_DynamicCountOf, dynamicCountOf)
		JNC_STD_FUNCTION (StdFunction_DynamicCastDataPtr, dynamicCastDataPtr)
		JNC_STD_FUNCTION (StdFunction_DynamicCastClassPtr, dynamicCastClassPtr)
		JNC_STD_FUNCTION (StdFunction_DynamicCastVariant, dynamicCastVariant)
		JNC_STD_FUNCTION (StdFunction_StrengthenClassPtr, strengthenClassPtr)

		JNC_STD_FUNCTION (StdFunction_AllocateClass, allocateClass)
		JNC_STD_FUNCTION (StdFunction_TryAllocateClass, tryAllocateClass)
		JNC_STD_FUNCTION (StdFunction_AllocateData, allocateData)
		JNC_STD_FUNCTION (StdFunction_TryAllocateData, tryAllocateData)
		JNC_STD_FUNCTION (StdFunction_AllocateArray, allocateArray)
		JNC_STD_FUNCTION (StdFunction_TryAllocateArray, tryAllocateArray)
		JNC_STD_FUNCTION (StdFunction_GcSafePoint, gcSafePoint)		
		JNC_STD_FUNCTION (StdFunction_CollectGarbage, collectGarbage)

		JNC_STD_FUNCTION (StdFunction_GetCurrentThreadId, getCurrentThreadId)
		JNC_STD_FUNCTION (StdFunction_CreateThread, createThread)
		JNC_STD_FUNCTION (StdFunction_Sleep, sleep)
		JNC_STD_FUNCTION (StdFunction_GetTimestamp, getTimestamp)
		JNC_STD_FUNCTION (StdFunction_Throw, forceThrow)
		JNC_STD_FUNCTION (StdFunction_GetLastError, getLastError)
		JNC_STD_FUNCTION (StdFunction_SetPosixError, setPosixError)
		JNC_STD_FUNCTION (StdFunction_SetStringError, setStringError)
		JNC_STD_FUNCTION (StdFunction_AssertionFailure, assertionFailure)
		JNC_STD_FUNCTION (StdFunction_AddStaticDestructor, addStaticDestructor)
		JNC_STD_FUNCTION (StdFunction_AddStaticClassDestructor, addStaticClassDestructor)
		JNC_STD_FUNCTION (StdFunction_StrLen, strLen)
		JNC_STD_FUNCTION (StdFunction_StrCmp, strCmp)
		JNC_STD_FUNCTION (StdFunction_StriCmp, striCmp)
		JNC_STD_FUNCTION (StdFunction_StrChr, strChr)
		JNC_STD_FUNCTION (StdFunction_StrCat, strCat)
		JNC_STD_FUNCTION (StdFunction_StrDup, strDup)
		JNC_STD_FUNCTION (StdFunction_MemCmp, memCmp)
		JNC_STD_FUNCTION (StdFunction_MemChr, memChr)
		JNC_STD_FUNCTION (StdFunction_MemCpy, memCpy)
		JNC_STD_FUNCTION (StdFunction_MemSet, memSet)
		JNC_STD_FUNCTION (StdFunction_MemCat, memCat)
		JNC_STD_FUNCTION (StdFunction_MemDup, memDup)
		JNC_STD_FUNCTION (StdFunction_Rand, rand)
		JNC_STD_FUNCTION (StdFunction_Atoi, atoi)
		JNC_STD_FUNCTION (StdFunction_Format, format)
		JNC_STD_FUNCTION (StdFunction_GetTls, getTls)

		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_a, appendFmtLiteral_a)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_p, appendFmtLiteral_p)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_i32, appendFmtLiteral_i32)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_ui32, appendFmtLiteral_ui32)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_i64, appendFmtLiteral_i64)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_ui64, appendFmtLiteral_ui64)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_f, appendFmtLiteral_f)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_v, appendFmtLiteral_v)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_s, appendFmtLiteral_s)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_sr, appendFmtLiteral_sr)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_cb, appendFmtLiteral_s)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_cbr, appendFmtLiteral_sr)
		JNC_STD_FUNCTION (StdFunction_AppendFmtLiteral_br, appendFmtLiteral_s)

		JNC_STD_FUNCTION (StdFunction_TryCheckDataPtrRangeDirect, tryCheckDataPtrRangeDirect)
		JNC_STD_FUNCTION (StdFunction_CheckDataPtrRangeDirect, checkDataPtrRangeDirect)
		JNC_STD_FUNCTION (StdFunction_TryCheckDataPtrRangeIndirect, tryCheckDataPtrRangeIndirect)
		JNC_STD_FUNCTION (StdFunction_CheckDataPtrRangeIndirect, checkDataPtrRangeIndirect)
		JNC_STD_FUNCTION (StdFunction_TryCheckNullPtr, tryCheckNullPtr)
		JNC_STD_FUNCTION (StdFunction_CheckNullPtr, checkNullPtr)
		JNC_STD_FUNCTION (StdFunction_TryLazyGetLibraryFunction, tryLazyGetLibraryFunction)
		JNC_STD_FUNCTION (StdFunction_LazyGetLibraryFunction, lazyGetLibraryFunction)
		
		JNC_STD_TYPE (StdType_Error, Error)
		JNC_STD_TYPE (StdType_String, String)
		JNC_STD_TYPE (StdType_StringRef, StringRef)
		JNC_STD_TYPE (StdType_StringBuilder, StringBuilder)
		JNC_STD_TYPE (StdType_StringHashTable, StringHashTable)
		JNC_STD_TYPE (StdType_VariantHashTable, VariantHashTable)
		JNC_STD_TYPE (StdType_ListEntry, ListEntry)
		JNC_STD_TYPE (StdType_List, List)
		JNC_STD_TYPE (StdType_ConstBuffer, ConstBuffer)
		JNC_STD_TYPE (StdType_ConstBufferRef, ConstBufferRef)
		JNC_STD_TYPE (StdType_BufferRef, BufferRef)
		JNC_STD_TYPE (StdType_Buffer, Buffer)
		JNC_STD_TYPE (StdType_Recognizer, Recognizer)
		JNC_STD_TYPE (StdType_Library, Library)
		JNC_LIB (MulticastLib)
	JNC_END_LIB ()

public:
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

	static
	Box*
	tryAllocateClass (ClassType* type);

	static
	Box*
	allocateClass (ClassType* type);

	static
	DataBox*
	tryAllocateData (Type* type);

	static
	DataBox*
	allocateData (Type* type);

	static
	DynamicArrayBox*
	tryAllocateArray (
	Type* type,
		size_t elementCount
		);

	static
	DynamicArrayBox*
	allocateArray (
		Type* type,
		size_t elementCount
		);

	static
	void
	gcSafePoint ();

	static
	void
	collectGarbage ();

	static
	intptr_t
	getCurrentThreadId ()
	{
		return (intptr_t) mt::getCurrentThreadId ();
	}

	static
	bool
	createThread (FunctionPtr ptr);

	static
	void
	sleep (uint32_t msCount)
	{
		g::sleep (msCount);
	}

	static
	uint64_t
	getTimestamp ()
	{
		return g::getTimestamp ();
	}

	static
	bool
	forceThrow ()
	{
		return false;
	}

	static
	DataPtr
	getLastError ()
	{
		return getErrorPtr (err::getLastError ());
	}

	static
	DataPtr
	setPosixError (int code)
	{
		return getErrorPtr (err::setErrno (code));
	}

	static
	DataPtr
	setStringError (DataPtr stringPtr)
	{
		return getErrorPtr (err::setStringError ((const char*) stringPtr.m_p));
	}

	static
	void
	assertionFailure (
		const char* fileName,
		int line,
		const char* condition,
		const char* message
		);

	static
	void
	addStaticDestructor (StaticDestructFunc* destructFunc);

	static
	void
	addStaticClassDestructor (
		DestructFunc* destructFunc,
		jnc::IfaceHdr* iface
		);

	static
	DataPtr
	format (
		DataPtr formatString,
		...
		);

	static
	size_t
	strLen (DataPtr ptr);

	static
	int
	strCmp (
		DataPtr ptr1,
		DataPtr ptr2
		);

	static
	int
	striCmp (
		DataPtr ptr1,
		DataPtr ptr2
		);

	static
	DataPtr 
	strChr (
		DataPtr ptr,
		int c
		);

	static
	DataPtr 
	strCat (
		DataPtr ptr1,
		DataPtr ptr2
		);

	static
	DataPtr 
	strDup (
		DataPtr ptr,
		size_t length
		);

	static
	int
	memCmp (
		DataPtr ptr1,
		DataPtr ptr2,
		size_t size
		);

	static
	DataPtr 
	memChr (
		DataPtr ptr,
		int c,
		size_t size
		);

	static
	void
	memCpy (
		DataPtr dstPtr,
		DataPtr srcPtr,
		size_t size
		);

	static
	void
	memSet (
		DataPtr ptr,
		int c,
		size_t size
		);

	static
	DataPtr
	memCat (
		DataPtr ptr1,
		size_t size1,
		DataPtr ptr2,
		size_t size2
		);

	static
	DataPtr
	memDup (
		DataPtr ptr,
		size_t size
		);

	static
	int
	rand ()
	{
		return ::rand ();
	}

	static
	int
	atoi (DataPtr ptr)
	{
		return ptr.m_p ? ::atoi ((char*) ptr.m_p) : 0;
	}

	static
	void*
	getTls ();

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

	static
	size_t
	appendFmtLiteral_s (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		String string
		)
	{
		return appendFmtLiteralStringImpl (
			fmtLiteral, 
			fmtSpecifier, 
			(const char*) string.m_ptr.m_p, 
			string.m_length
			);
	}

	static
	size_t
	appendFmtLiteral_sr (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		StringRef stringRef
		)
	{
		return appendFmtLiteralStringImpl (
			fmtLiteral, 
			fmtSpecifier, 
			(const char*) stringRef.m_ptr.m_p, 
			stringRef.m_length
			);
	}

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
	void* 
	tryLazyGetLibraryFunction (
		Library* library,
		size_t index,
		const char* name
		);

	static
	void* 
	lazyGetLibraryFunction (
		Library* library,
		size_t index,
		const char* name
		);

protected:
#if (_AXL_ENV == AXL_ENV_WIN)
	static
	DWORD
	WINAPI
	threadFunc (PVOID context);
#elif (_AXL_ENV == AXL_ENV_POSIX)
	static
	void*
	threadFunc (void* context);
#endif

	static
	DataPtr
	getErrorPtr (const err::ErrorData* errorData);

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

//.............................................................................

DataPtr
strDup (
	const char* p,
	size_t length = -1
	);

//.............................................................................

} // namespace jnc {
