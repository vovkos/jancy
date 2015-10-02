// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Error.h"
#include "jnc_Buffer.h"
#include "jnc_String.h"
#include "jnc_List.h"
#include "jnc_HashTable.h"

#include "jnc_globals.jnc.cpp"
#include "jnc_Error.jnc.cpp"
#include "jnc_Buffer.jnc.cpp"
#include "jnc_String.jnc.cpp"
#include "jnc_List.jnc.cpp"
#include "jnc_HashTable.jnc.cpp"

namespace jnc {

//.............................................................................

class StdLib: public ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("jnc.getCurrentThreadId", getCurrentThreadId)
		JNC_MAP_FUNCTION ("jnc.createThread",       createThread)
		JNC_MAP_FUNCTION ("jnc.sleep",              sleep)
		JNC_MAP_FUNCTION ("jnc.getTimestamp",       getTimestamp)
		JNC_MAP_FUNCTION ("jnc.throw",              forceThrow)
		JNC_MAP_FUNCTION ("jnc.getLastError",       getLastError)
		JNC_MAP_FUNCTION ("jnc.setPosixError",      setPosixError)
		JNC_MAP_FUNCTION ("jnc.setStringError",     setStringError)
		JNC_MAP_FUNCTION ("jnc.collectGarbage",     collectGarbage)
		JNC_MAP_FUNCTION ("jnc.format",             format)
		
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
		
/*

#if (_AXL_ENV == AXL_ENV_POSIX)
		source = getStdFunctionSource (func);
		ASSERT (source->m_p);

		function = parseStdFunction (
			source->m_stdNamespace,
			source->m_p,
			source->m_length
			);

		ASSERT (!function->m_llvmFunction);
		function->m_tag += "_jnc"; // as to avoid mapping conflicts
		break;
#endif

 */
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

	JNC_BEGIN_LIB_FORCED_EXPORT ()
		JNC_LIB_FORCED_SOURCE_FILE ("jnc_globals.jnc", g_jnc_globalsSrc)
		JNC_LIB_FORCED_SOURCE_FILE ("jnc_Error.jnc",   g_jnc_ErrorSrc)
	JNC_END_LIB_FORCED_EXPORT ()

	JNC_BEGIN_LIB_SOURCE_FILE_TABLE ()
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_Buffer.jnc",    g_jnc_BufferSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_String.jnc",    g_jnc_StringSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_List.jnc",      g_jnc_ListSrc)
		JNC_LIB_SOURCE_FILE_TABLE_ENTRY ("jnc_HashTable.jnc", g_jnc_HashTableSrc)
	JNC_END_LIB_SOURCE_FILE_TABLE ()

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
	void
	collectGarbage ();

	static
	DataPtr
	format (
		DataPtr formatString,
		...
		);

	static
	size_t
	strLen (DataPtr ptr)
	{
		return jnc::strLen (ptr);
	}

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ExtensionLib* 
getStdLib (ExtensionLibSlotDb* slotDb);

//.............................................................................

} // namespace jnc {
