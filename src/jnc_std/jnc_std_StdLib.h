// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_std_Error.h"
#include "jnc_std_Buffer.h"
#include "jnc_std_String.h"
#include "jnc_std_List.h"
#include "jnc_std_HashTable.h"

#include "jnc_std_globals.jnc.cpp"
#include "jnc_std_Error.jnc.cpp"
#include "jnc_std_Buffer.jnc.cpp"
#include "jnc_std_String.jnc.cpp"
#include "jnc_std_List.jnc.cpp"
#include "jnc_std_HashTable.jnc.cpp"

namespace jnc {
namespace std {

//.............................................................................

class StdLib: public ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("std.getCurrentThreadId", getCurrentThreadId)
		JNC_MAP_FUNCTION ("std.createThread",       createThread)
		JNC_MAP_FUNCTION ("std.getTimestamp",       getTimestamp)
		JNC_MAP_FUNCTION ("std.sleep",              sleep)
		JNC_MAP_FUNCTION ("std.getLastError",       getLastError)
		JNC_MAP_FUNCTION ("std.setPosixError",      setPosixError)
		JNC_MAP_FUNCTION ("std.setStringError",     setStringError)
		JNC_MAP_FUNCTION ("std.format",             format)
		
		JNC_MAP_FUNCTION ("strlen",  strLen)
		JNC_MAP_FUNCTION ("strcmp",  strCmp)
		JNC_MAP_FUNCTION ("stricmp", striCmp)
		JNC_MAP_FUNCTION ("strchr",  strChr)
		JNC_MAP_FUNCTION ("strcat",  strCat)
		JNC_MAP_FUNCTION ("strdup",  strDup)
		JNC_MAP_FUNCTION ("memcmp",  memCmp)
		JNC_MAP_FUNCTION ("memchr",  memChr)
		JNC_MAP_FUNCTION ("memcpy",  memCpy)
		JNC_MAP_FUNCTION ("memset",  memSet)
		JNC_MAP_FUNCTION ("memcat",  memCat)
		JNC_MAP_FUNCTION ("memdup",  memDup)
		JNC_MAP_FUNCTION ("rand",    rand)
		JNC_MAP_FUNCTION ("atoi",    atoi)

		JNC_MAP_TYPE (Error)
		JNC_MAP_TYPE (ConstBuffer)
		JNC_MAP_TYPE (ConstBufferRef)
		JNC_MAP_TYPE (BufferRef)
		JNC_MAP_TYPE (Buffer)
		JNC_MAP_TYPE (String)
		JNC_MAP_TYPE (StringRef)
		JNC_MAP_TYPE (StringBuilder)
		JNC_MAP_TYPE (StringHashTable)
		JNC_MAP_TYPE (VariantHashTable)
		JNC_MAP_TYPE (List)
		JNC_MAP_TYPE (ListEntry)
	JNC_END_LIB_MAP ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE ("std_Error.jnc",     g_std_ErrorSrc)
		JNC_LIB_SOURCE_FILE ("std_Buffer.jnc",    g_std_BufferSrc)
		JNC_LIB_SOURCE_FILE ("std_String.jnc",    g_std_StringSrc)
		JNC_LIB_SOURCE_FILE ("std_List.jnc",      g_std_ListSrc)
		JNC_LIB_SOURCE_FILE ("std_HashTable.jnc", g_std_HashTableSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()

	JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE ()
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (StringHashTable)
		JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (VariantHashTable)
	JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

	JNC_BEGIN_LIB_FORCED_EXPORT ()
		JNC_LIB_FORCED_SOURCE_FILE ("std_globals.jnc", g_std_globalsSrc)
		JNC_LIB_FORCED_SOURCE_FILE ("std_Error.jnc",   g_std_ErrorSrc)
	JNC_END_LIB_FORCED_EXPORT ()

public:
	static
	intptr_t
	getCurrentThreadId ()
	{
		return (intptr_t) sys::getCurrentThreadId ();
	}

	static
	bool
	createThread (rt::FunctionPtr ptr);

	static
	uint64_t
	getTimestamp ()
	{
		return sys::getTimestamp ();
	}

	static
	void
	sleep (uint32_t msCount);

	static
	rt::DataPtr
	getLastError ()
	{
		return getErrorPtr (err::getLastError ());
	}

	static
	rt::DataPtr
	setPosixError (int code)
	{
		return getErrorPtr (err::setErrno (code));
	}

	static
	rt::DataPtr
	setStringError (rt::DataPtr stringPtr)
	{
		return getErrorPtr (err::setStringError ((const char*) stringPtr.m_p));
	}

	static
	rt::DataPtr
	format (
		rt::DataPtr formatString,
		...
		);

	static
	size_t
	strLen (rt::DataPtr ptr)
	{
		return rt::strLen (ptr);
	}

	static
	int
	strCmp (
		rt::DataPtr ptr1,
		rt::DataPtr ptr2
		);

	static
	int
	striCmp (
		rt::DataPtr ptr1,
		rt::DataPtr ptr2
		);

	static
	rt::DataPtr 
	strChr (
		rt::DataPtr ptr,
		int c
		);

	static
	rt::DataPtr 
	strCat (
		rt::DataPtr ptr1,
		rt::DataPtr ptr2
		);

	static
	rt::DataPtr 
	strDup (
		rt::DataPtr ptr,
		size_t length
		);

	static
	int
	memCmp (
		rt::DataPtr ptr1,
		rt::DataPtr ptr2,
		size_t size
		);

	static
	rt::DataPtr 
	memChr (
		rt::DataPtr ptr,
		int c,
		size_t size
		);

	static
	void
	memCpy (
		rt::DataPtr dstPtr,
		rt::DataPtr srcPtr,
		size_t size
		);

	static
	void
	memSet (
		rt::DataPtr ptr,
		int c,
		size_t size
		);

	static
	rt::DataPtr
	memCat (
		rt::DataPtr ptr1,
		size_t size1,
		rt::DataPtr ptr2,
		size_t size2
		);

	static
	rt::DataPtr
	memDup (
		rt::DataPtr ptr,
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
	atoi (rt::DataPtr ptr)
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
	rt::DataPtr
	getErrorPtr (const err::ErrorHdr* error);
};

//.............................................................................

} // namespace std
} // namespace jnc
