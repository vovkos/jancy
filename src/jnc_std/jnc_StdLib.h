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

namespace jnc {

//.............................................................................

class StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_STD_FUNCTION (StdFunc_GetCurrentThreadId, getCurrentThreadId)
		JNC_STD_FUNCTION (StdFunc_CreateThread, createThread)
		JNC_STD_FUNCTION (StdFunc_Sleep, sleep)
		JNC_STD_FUNCTION (StdFunc_GetTimestamp, getTimestamp)
		JNC_STD_FUNCTION (StdFunc_Throw, forceThrow)
		JNC_STD_FUNCTION (StdFunc_GetLastError, getLastError)
		JNC_STD_FUNCTION (StdFunc_SetPosixError, setPosixError)
		JNC_STD_FUNCTION (StdFunc_SetStringError, setStringError)
		
		JNC_STD_FUNCTION (StdFunc_StrLen, strLen)
		JNC_STD_FUNCTION (StdFunc_StrCmp, strCmp)
		JNC_STD_FUNCTION (StdFunc_StriCmp, striCmp)
		JNC_STD_FUNCTION (StdFunc_StrChr, strChr)
		JNC_STD_FUNCTION (StdFunc_StrCat, strCat)
		JNC_STD_FUNCTION (StdFunc_StrDup, strDup)
		JNC_STD_FUNCTION (StdFunc_MemCmp, memCmp)
		JNC_STD_FUNCTION (StdFunc_MemChr, memChr)
		JNC_STD_FUNCTION (StdFunc_MemCpy, memCpy)
		JNC_STD_FUNCTION (StdFunc_MemSet, memSet)
		JNC_STD_FUNCTION (StdFunc_MemCat, memCat)
		JNC_STD_FUNCTION (StdFunc_MemDup, memDup)
		JNC_STD_FUNCTION (StdFunc_Rand, rand)
		JNC_STD_FUNCTION (StdFunc_Atoi, atoi)
		JNC_STD_FUNCTION (StdFunc_Format, format)
		
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
	JNC_END_LIB ()

public:
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
	sleep (uint32_t msCount);

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
};

//.............................................................................

DataPtr
strDup (
	const char* p,
	size_t length = -1
	);

DataPtr
memDup (
	const void* p,
	size_t size
	);

//.............................................................................

} // namespace jnc {
