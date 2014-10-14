// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Runtime.h"
#include "jnc_Api.h"
#include "jnc_String.h"
#include "jnc_Buffer.h"
#include "jnc_MulticastLib.h"

namespace jnc {

//.............................................................................

class StdLib
{
public:
	JNC_API_BEGIN_LIB ()
		JNC_API_STD_FUNCTION (StdFunction_RuntimeError, runtimeError)
		JNC_API_STD_FUNCTION (StdFunction_DynamicCastClassPtr, dynamicCastClassPtr)
		JNC_API_STD_FUNCTION (StdFunction_StrengthenClassPtr, strengthenClassPtr)
		JNC_API_STD_FUNCTION (StdFunction_GcAllocate, gcAllocate)
		JNC_API_STD_FUNCTION (StdFunction_GcTryAllocate, gcTryAllocate)
		JNC_API_STD_FUNCTION_FORCED (StdFunction_GcEnter, gcEnter)
		JNC_API_STD_FUNCTION_FORCED (StdFunction_GcLeave, gcLeave)
		JNC_API_STD_FUNCTION (StdFunction_GcPulse, gcPulse)
		JNC_API_STD_FUNCTION (StdFunction_RunGc, runGc)
		JNC_API_STD_FUNCTION (StdFunction_GetCurrentThreadId, getCurrentThreadId)
		JNC_API_STD_FUNCTION (StdFunction_CreateThread, createThread)
		JNC_API_STD_FUNCTION (StdFunction_Sleep, sleep)
		JNC_API_STD_FUNCTION (StdFunction_GetTimestamp, getTimestamp)
		JNC_API_STD_FUNCTION (StdFunction_GetLastError, getLastError)
		JNC_API_CONST_PROPERTY ("jnc.Error.m_description", getErrorDescription)
		JNC_API_STD_FUNCTION (StdFunction_StrLen, strLen)
		JNC_API_STD_FUNCTION (StdFunction_MemCpy, memCpy)
		JNC_API_STD_FUNCTION (StdFunction_MemCat, memCat)
		JNC_API_STD_FUNCTION (StdFunction_Rand, rand)
		JNC_API_STD_FUNCTION (StdFunction_Atoi, atoi)
		JNC_API_STD_FUNCTION (StdFunction_Format, format)
		JNC_API_STD_FUNCTION (StdFunction_GetTls, getTls)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_a, appendFmtLiteral_a)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_p, appendFmtLiteral_p)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_i32, appendFmtLiteral_i32)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_ui32, appendFmtLiteral_ui32)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_i64, appendFmtLiteral_i64)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_ui64, appendFmtLiteral_ui64)
		JNC_API_STD_FUNCTION (StdFunction_AppendFmtLiteral_f, appendFmtLiteral_f)
		JNC_API_STD_TYPE (StdType_String, String)
		JNC_API_STD_TYPE (StdType_StringRef, StringRef)
		JNC_API_STD_TYPE (StdType_StringBuilder, StringBuilder)
		JNC_API_STD_TYPE (StdType_ConstBuffer, ConstBuffer)
		JNC_API_STD_TYPE (StdType_ConstBufferRef, ConstBufferRef)
		JNC_API_STD_TYPE (StdType_BufferRef, BufferRef)
		JNC_API_STD_TYPE (StdType_Buffer, Buffer)
		JNC_API_LIB (MulticastLib)
	JNC_API_END_LIB ()

public:
	static
	void
	runtimeError (
		int error,
		void* codeAddr,
		void* dataAddr
		)
	{
		Runtime::runtimeError (error, codeAddr, dataAddr);
	}

	static
	IfaceHdr*
	dynamicCastClassPtr (
		IfaceHdr* p,
		ClassType* type
		);

	static
	IfaceHdr*
	strengthenClassPtr (IfaceHdr* p);

	static
	void*
	gcAllocate (
		Type* type,
		size_t elementCount
		);

	static
	void*
	gcTryAllocate (
		Type* type,
		size_t elementCount
		);

	static
	void
	gcEnter ();

	static
	void
	gcLeave ();

	static
	void
	gcPulse ();

	static
	void
	runGc ();

	static
	intptr_t
	getCurrentThreadId ();

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
	DataPtr
	getLastError ();

	static
	DataPtr
	getErrorDescription (DataPtr error);

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
	void
	memCpy (
		DataPtr dstPtr,
		DataPtr srcPtr,
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
		);

	static
	size_t
	appendFmtLiteral_i32 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 'd', x);
	}

	static
	size_t
	appendFmtLiteral_ui32 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint32_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 'u', x);
	}

	static
	size_t
	appendFmtLiteral_i64 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		int64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 'd', x);
	}

	static
	size_t
	appendFmtLiteral_ui64 (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		uint64_t x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 'u', x);
	}

	static
	size_t
	appendFmtLiteral_f (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		double x
		)
	{
		return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 'f', x);
	}

protected:
#if (_AXL_ENV == AXL_ENV_WIN)
	static
	DWORD
	WINAPI
	threadProc (PVOID context);
#elif (_AXL_ENV == AXL_ENV_POSIX)
	static
	void*
	threadProc (void* context);
#endif

	static
	void
	prepareFormatString (
		rtl::String* formatString,
		const char* fmtSpecifier,
		char defaultType
		);

	static
	size_t
	appendFmtLiteralImpl (
		FmtLiteral* fmtLiteral,
		const char* fmtSpecifier,
		char defaultType,
		...
		);
};

//.............................................................................

} // namespace jnc {
