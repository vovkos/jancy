// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Runtime.h"
#include "jnc_Api.h"
#include "jnc_MulticastLib.h"

namespace jnc {

//.............................................................................

class CStdLib
{
public:
	JNC_API_BEGIN_LIB ()
		JNC_API_STD_FUNCTION (EStdFunc_RuntimeError, RuntimeError)
		JNC_API_STD_FUNCTION (EStdFunc_DynamicCastClassPtr, DynamicCastClassPtr)
		JNC_API_STD_FUNCTION (EStdFunc_StrengthenClassPtr, StrengthenClassPtr)
		JNC_API_STD_FUNCTION (EStdFunc_GcAllocate, GcAllocate)
		JNC_API_STD_FUNCTION (EStdFunc_GcTryAllocate, GcTryAllocate)
		JNC_API_STD_FUNCTION_FORCED (EStdFunc_GcEnter, GcEnter)
		JNC_API_STD_FUNCTION_FORCED (EStdFunc_GcLeave, GcLeave)
		JNC_API_STD_FUNCTION (EStdFunc_GcPulse, GcPulse)
		JNC_API_STD_FUNCTION (EStdFunc_RunGc, RunGc)
		JNC_API_STD_FUNCTION (EStdFunc_GetCurrentThreadId, GetCurrentThreadId)
		JNC_API_STD_FUNCTION (EStdFunc_CreateThread, CreateThread)
		JNC_API_STD_FUNCTION (EStdFunc_Sleep, Sleep)
		JNC_API_STD_FUNCTION (EStdFunc_GetTimestamp, GetTimestamp)
		JNC_API_STD_FUNCTION (EStdFunc_GetLastError, GetLastError)
		JNC_API_CONST_PROPERTY ("jnc.Error.m_description", GetErrorDescription)
		JNC_API_STD_FUNCTION (EStdFunc_StrLen, StrLen)
		JNC_API_STD_FUNCTION (EStdFunc_MemCpy, MemCpy)
		JNC_API_STD_FUNCTION (EStdFunc_MemCat, MemCat)
		JNC_API_STD_FUNCTION (EStdFunc_Rand, Rand)
		JNC_API_STD_FUNCTION (EStdFunc_Format, Format)
		JNC_API_STD_FUNCTION (EStdFunc_GetTls, GetTls)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_a, AppendFmtLiteral_a)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_p, AppendFmtLiteral_p)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_i32, AppendFmtLiteral_i32)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_ui32, AppendFmtLiteral_ui32)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_i64, AppendFmtLiteral_i64)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_ui64, AppendFmtLiteral_ui64)
		JNC_API_STD_FUNCTION (EStdFunc_AppendFmtLiteral_f, AppendFmtLiteral_f)
		JNC_API_LIB (CMulticastLib)
	JNC_API_END_LIB ()

public:
	static
	void
	RuntimeError (
		int Error,
		void* pCodeAddr,
		void* pDataAddr
		)
	{
		CRuntime::RuntimeError (Error, pCodeAddr, pDataAddr);
	}

	static
	TIfaceHdr*
	DynamicCastClassPtr (
		TIfaceHdr* p,
		CClassType* pType
		);

	static
	TIfaceHdr*
	StrengthenClassPtr (TIfaceHdr* p);

	static
	void*
	GcAllocate (
		CType* pType,
		size_t ElementCount
		);

	static
	void*
	GcTryAllocate (
		CType* pType,
		size_t ElementCount
		);

	static
	void
	GcEnter ();

	static
	void
	GcLeave ();

	static
	void
	GcPulse ();

	static
	void
	RunGc ();

	static
	intptr_t
	GetCurrentThreadId ();

	static
	bool
	CreateThread (TFunctionPtr Ptr);

	static
	void
	Sleep (uint32_t MsCount)
	{
		g::Sleep (MsCount);
	}

	static
	uint64_t
	GetTimestamp ()
	{
		return g::GetTimestamp ();
	}

	static
	TDataPtr
	GetLastError ();

	static
	TDataPtr
	GetErrorDescription (TDataPtr Error);

	static
	TDataPtr
	Format (
		TDataPtr FormatString,
		...
		);

	static
	size_t
	StrLen (TDataPtr Ptr);

	static
	void
	MemCpy (
		TDataPtr DstPtr,
		TDataPtr SrcPtr,
		size_t Size
		);

	static
	TDataPtr
	MemCat (
		TDataPtr Ptr1,
		size_t Size1,
		TDataPtr Ptr2,
		size_t Size2
		);

	static
	int
	Rand ()
	{
		return rand ();
	}

	static
	void*
	GetTls ();

	static
	size_t
	AppendFmtLiteral_a (
		TFmtLiteral* pFmtLiteral,
		const char* p,
		size_t Length
		);

	static
	size_t
	AppendFmtLiteral_p (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		TDataPtr Ptr
		);

	static
	size_t
	AppendFmtLiteral_i32 (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		int32_t x
		)
	{
		return AppendFmtLiteralImpl (pFmtLiteral, pFmtSpecifier, 'd', x);
	}

	static
	size_t
	AppendFmtLiteral_ui32 (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		uint32_t x
		)
	{
		return AppendFmtLiteralImpl (pFmtLiteral, pFmtSpecifier, 'u', x);
	}

	static
	size_t
	AppendFmtLiteral_i64 (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		int64_t x
		)
	{
		return AppendFmtLiteralImpl (pFmtLiteral, pFmtSpecifier, 'd', x);
	}

	static
	size_t
	AppendFmtLiteral_ui64 (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		uint64_t x
		)
	{
		return AppendFmtLiteralImpl (pFmtLiteral, pFmtSpecifier, 'u', x);
	}

	static
	size_t
	AppendFmtLiteral_f (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		double x
		)
	{
		return AppendFmtLiteralImpl (pFmtLiteral, pFmtSpecifier, 'f', x);
	}

protected:
#if (_AXL_ENV == AXL_ENV_WIN)
	static
	DWORD
	WINAPI
	ThreadProc (PVOID pContext);
#elif (_AXL_ENV == AXL_ENV_POSIX)
	static
	void*
	ThreadProc (void* pContext);
#endif

	static
	void
	PrepareFormatString (
		rtl::CString* pFormatString,
		const char* pFmtSpecifier,
		char DefaultType
		);

	static
	size_t
	AppendFmtLiteralImpl (
		TFmtLiteral* pFmtLiteral,
		const char* pFmtSpecifier,
		char DefaultType,
		...
		);
};

//.............................................................................

} // namespace jnc {
