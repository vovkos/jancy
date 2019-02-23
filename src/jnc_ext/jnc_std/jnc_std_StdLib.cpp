//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

#include "jnc_std_Guid.h"
#include "jnc_std_Error.h"
#include "jnc_std_String.h"
#include "jnc_std_Buffer.h"
#include "jnc_std_Array.h"
#include "jnc_std_List.h"
#include "jnc_std_HashTable.h"
#include "jnc_std_RbTree.h"

#include "std_globals.jnc.cpp"
#include "std_Guid.jnc.cpp"
#include "std_Error.jnc.cpp"
#include "std_String.jnc.cpp"
#include "std_Buffer.jnc.cpp"
#include "std_Array.jnc.cpp"
#include "std_List.jnc.cpp"
#include "std_MapEntry.jnc.cpp"
#include "std_HashTable.jnc.cpp"
#include "std_RbTree.jnc.cpp"

namespace jnc {
namespace std {

//..............................................................................

size_t
stdGets (
	void* buffer,
	size_t size
	)
{
	char* p = (char*) buffer;
	fgets (p, size, stdin);
	return strnlen (p, size);
}

size_t
stdPrintOut (
	const void* p,
	size_t size
	)
{
	return fwrite (p, size, 1, stdout);
}

size_t
stdPrintErr (
	const void* p,
	size_t size
	)
{
	return fwrite (p, size, 1, stderr);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

jnc_StdLib_StdInputFunc* g_getsFunc = stdGets;
jnc_StdLib_StdOutputFunc* g_printOutFunc = stdPrintOut;
jnc_StdLib_StdOutputFunc* g_printErrFunc = stdPrintErr;

//..............................................................................

int
atoi (DataPtr ptr)
{
	return ptr.m_p ? ::atoi ((char*) ptr.m_p) : 0;
}

long
strtol (
	DataPtr ptr,
	DataPtr endPtr,
	int radix
	)
{
	long result = 0;
	char* end = (char*) ptr.m_p;
	if (ptr.m_p)
		result = ::strtol ((char*) ptr.m_p, &end, radix);

	if (endPtr.m_p)
	{
		((DataPtr*) endPtr.m_p)->m_p = end;
		((DataPtr*) endPtr.m_p)->m_validator = ptr.m_validator;
	}

	return result;
}

uint32_t
toUpper (uint32_t c)
{
	return enc::utfToUpperCase (c);
}

uint32_t
toLower (uint32_t c)
{
	return enc::utfToLowerCase (c);
}

DataPtr
getErrorPtr (const err::ErrorHdr* error)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr resultPtr = gcHeap->tryAllocateBuffer (error->m_size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, error, error->m_size);
	return resultPtr;
}

DataPtr
getLastError ()
{
	return getErrorPtr (err::getLastError ());
}

void
setErrno (int code)
{
	err::setErrno (code);
}

void
setError_0 (DataPtr errorPtr)
{
	err::setError ((const err::ErrorHdr*) errorPtr.m_p);
}

void
setError_1 (DataPtr stringPtr)
{
	err::setError ((const char*) stringPtr.m_p);
}

int
memCmp (
	DataPtr ptr1,
	DataPtr ptr2,
	size_t size
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		memcmp (ptr1.m_p, ptr2.m_p, size);
}

DataPtr
memChr (
	DataPtr ptr,
	int c,
	size_t size
	)
{
	if (!ptr.m_p)
		return g_nullPtr;

	void* p = memchr (ptr.m_p, c, size);
	if (!p)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr.m_validator;
	return resultPtr;
}

DataPtr
memMem (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	if (!ptr1.m_p)
		return g_nullPtr;

	if (!ptr2.m_p)
		return ptr1;

	void* p = sl::memMem (ptr1.m_p, size1, ptr2.m_p, size2);
	if (!p)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr1.m_validator;
	return resultPtr;
}

void
memCpy (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memcpy (dstPtr.m_p, srcPtr.m_p, size);
}

void
memMove (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memmove (dstPtr.m_p, srcPtr.m_p, size);
}

void
memSet (
	DataPtr ptr,
	int c,
	size_t size
	)
{
	if (ptr.m_p)
		memset (ptr.m_p, c, size);
}

DataPtr
memCat (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	size_t totalSize = size1 + size2;
	DataPtr resultPtr = gcHeap->tryAllocateBuffer (totalSize);
	if (!resultPtr.m_p)
		return g_nullPtr;

	char* p = (char*) resultPtr.m_p;

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);

	return resultPtr;
}

DataPtr
memDup (
	DataPtr ptr,
	size_t size
	)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr resultPtr = gcHeap->tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (ptr.m_p)
		memcpy (resultPtr.m_p, ptr.m_p, size);
	else
		memset (resultPtr.m_p, 0, size);

	return resultPtr;
}

size_t
memDjb2 (
	DataPtr ptr,
	size_t size
	)
{
	return sl::djb2 (ptr.m_p, size);
}

int
strCmp (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		strcmp ((char*) ptr1.m_p, (char*) ptr2.m_p);
}

int
strnCmp (
	DataPtr ptr1,
	DataPtr ptr2,
	size_t length
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		strncmp ((char*) ptr1.m_p, (char*) ptr2.m_p, length);
}

int
striCmp (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		_stricmp ((char*) ptr1.m_p, (char*) ptr2.m_p);
}

int
strniCmp (
	DataPtr ptr1,
	DataPtr ptr2,
	size_t length
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		_strnicmp ((char*) ptr1.m_p, (char*) ptr2.m_p, length);
}

DataPtr
strChr (
	DataPtr ptr,
	char c
	)
{
	if (!ptr.m_p)
		return g_nullPtr;

	char* p = strchr ((char*) ptr.m_p, c);
	if (!p)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p =
	resultPtr.m_validator = ptr.m_validator;
	return resultPtr;
}

DataPtr
striChr (
	DataPtr ptr,
	char c
	)
{
	if (!ptr.m_p)
		return g_nullPtr;

	size_t length = strLen (ptr);

	sl::TextBoyerMooreFind find;
	find.setPattern (enc::CharCodecKind_Ascii, &c, 1, sl::TextBoyerMooreFlag_CaseInsensitive);
	size_t offset = find.find (enc::CharCodecKind_Ascii, ptr.m_p, length);
	if (offset == -1)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p = (char*) ptr.m_p + offset;
	resultPtr.m_validator = ptr.m_validator;
	return resultPtr;
}

DataPtr
strStr (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (!ptr1.m_p)
		return g_nullPtr;

	if (!ptr2.m_p)
		return ptr1;

	char* p = strstr ((char*) ptr1.m_p, (char*) ptr2.m_p);
	if (!p)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr1.m_validator;
	return resultPtr;
}

DataPtr
striStr (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (!ptr1.m_p)
		return g_nullPtr;

	if (!ptr2.m_p)
		return ptr1;

	size_t length1 = strLen (ptr1);
	size_t length2 = strLen (ptr2);

	sl::TextBoyerMooreFind find;
	find.setPattern (enc::CharCodecKind_Ascii, ptr2.m_p, length2, sl::TextBoyerMooreFlag_CaseInsensitive);
	size_t offset = find.find (enc::CharCodecKind_Ascii, ptr1.m_p, length1);
	if (offset == -1)
		return g_nullPtr;

	DataPtr resultPtr;
	resultPtr.m_p = (char*) ptr1.m_p + offset;
	resultPtr.m_validator = ptr1.m_validator;
	return resultPtr;
}

void
strCpy (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (!dstPtr.m_validator)
	{
		err::setError ("null data pointer access");
		dynamicThrow ();
	}

	size_t dstLength = dstPtr.m_p < dstPtr.m_validator->m_rangeEnd ? (char*) dstPtr.m_validator->m_rangeEnd - (char*) dstPtr.m_p : 0;
	size_t srcLength = strLen (srcPtr);

	if (dstLength <= srcLength)
	{
		memcpy (dstPtr.m_p, srcPtr.m_p, dstLength);
	}
	else
	{
		memcpy (dstPtr.m_p, srcPtr.m_p, srcLength);
		((char*) dstPtr.m_p) [srcLength] = 0;
	}
}

DataPtr
strCat (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	size_t length1 = strLen (ptr1);
	size_t length2 = strLen (ptr2);

	return memCat (ptr1, length1, ptr2, length2 + 1);
}

DataPtr
strDup (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return jnc::strDup ((const char*) ptr.m_p, length);
}

size_t
strDjb2 (DataPtr ptr)
{
	size_t length = strLen (ptr);
	return sl::djb2 (ptr.m_p, length);
}

size_t
striDjb2 (DataPtr ptr)
{
	size_t length = strLen (ptr);
	return sl::djb2_op (::tolower, (char*) ptr.m_p, length);
}

DataPtr
format (
	DataPtr formatStringPtr,
	...
	)
{
	AXL_VA_DECL (va, formatStringPtr);

	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va ((const char*) formatStringPtr.m_p, va);
	size_t length = string.getLength ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr resultPtr = gcHeap->tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, string.sz (), length);
	return resultPtr;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
gets ()
{
	char buffer [1024];
	size_t length = g_getsFunc (buffer, sizeof (buffer));
	return jnc::strDup (buffer, length);
}

size_t
print (DataPtr ptr)
{
	size_t length = strLen (ptr);
	return g_printOutFunc (ptr.m_p, length);
}

size_t
print_u (const char* s)
{
	return s ? g_printOutFunc (s, strlen (s)) : 0;
}

size_t
perror (DataPtr ptr)
{
	size_t length = strLen (ptr);
	return g_printErrFunc (ptr.m_p, length);
}

int
printf (
	const char* formatString,
	...
	)
{
	AXL_VA_DECL (va, formatString);

	sl::String string = sl::formatString_va (formatString, va);
	return g_printOutFunc (string.cp (), string.getLength ());
}

void
resetDynamicLayout (DataPtr ptr)
{
	if (!ptr.m_validator)
		return;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->resetDynamicLayout (ptr.m_validator->m_targetBox);
}

//..............................................................................

} // namespace std
} // namespace jnc


using namespace jnc::std;

JNC_DEFINE_LIB (
	jnc_StdLib,
	g_stdLibGuid,
	"StdLib",
	"Jancy standard extension library"
	)

JNC_BEGIN_LIB_FUNCTION_MAP (jnc_StdLib)
	JNC_MAP_FUNCTION ("std.getLastError",       getLastError)
	JNC_MAP_FUNCTION ("std.setErrno",           setErrno)
	JNC_MAP_FUNCTION ("std.setError",           setError_0)
	JNC_MAP_OVERLOAD (setError_1)
	JNC_MAP_FUNCTION ("std.format",             format)
	JNC_MAP_FUNCTION ("std.resetDynamicLayout", resetDynamicLayout)

	JNC_MAP_FUNCTION ("strlen",   jnc::strLen)
	JNC_MAP_FUNCTION ("strcmp",   strCmp)
	JNC_MAP_FUNCTION ("strncmp",  strnCmp)
	JNC_MAP_FUNCTION ("stricmp",  striCmp)
	JNC_MAP_FUNCTION ("strnicmp", strniCmp)
	JNC_MAP_FUNCTION ("strchr",   strChr)
	JNC_MAP_FUNCTION ("strichr",  striChr)
	JNC_MAP_FUNCTION ("strstr",   strStr)
	JNC_MAP_FUNCTION ("stristr",  striStr)
	JNC_MAP_FUNCTION ("strcpy",   strCpy)
	JNC_MAP_FUNCTION ("strcat",   strCat)
	JNC_MAP_FUNCTION ("strdup",   strDup)
	JNC_MAP_FUNCTION ("strdjb2",  strDjb2)
	JNC_MAP_FUNCTION ("stridjb2", striDjb2)
	JNC_MAP_FUNCTION ("memcmp",   memCmp)
	JNC_MAP_FUNCTION ("memchr",   memChr)
	JNC_MAP_FUNCTION ("memmem",   memMem)
	JNC_MAP_FUNCTION ("memcpy",   memCpy)
	JNC_MAP_FUNCTION ("memmove",  memMove)
	JNC_MAP_FUNCTION ("memset",   memSet)
	JNC_MAP_FUNCTION ("memcat",   memCat)
	JNC_MAP_FUNCTION ("memdup",   memDup)
	JNC_MAP_FUNCTION ("memdjb2",  memDjb2)
	JNC_MAP_FUNCTION ("rand",     ::rand)
	JNC_MAP_FUNCTION ("atoi",     jnc::std::atoi)
	JNC_MAP_FUNCTION ("strtol",   jnc::std::strtol)
	JNC_MAP_FUNCTION ("toupper",  toUpper)
	JNC_MAP_FUNCTION ("tolower",  toLower)
	JNC_MAP_FUNCTION ("gets",     jnc::std::gets)
	JNC_MAP_FUNCTION ("print",    jnc::std::print)
	JNC_MAP_FUNCTION ("print_u",  print_u)
	JNC_MAP_FUNCTION ("perror",   jnc::std::perror)
	JNC_MAP_FUNCTION ("printf",   jnc::std::printf)

	JNC_MAP_TYPE (Guid)
	JNC_MAP_TYPE (Error)
	JNC_MAP_TYPE (StringBuilder)
	JNC_MAP_TYPE (Buffer)
	JNC_MAP_TYPE (Array)
	JNC_MAP_TYPE (ListEntry)
	JNC_MAP_TYPE (List)
	JNC_MAP_TYPE (MapEntry)
	JNC_MAP_TYPE (HashTable)
	JNC_MAP_TYPE (RbTree)
JNC_END_LIB_FUNCTION_MAP ()

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (jnc_StdLib)
	JNC_LIB_SOURCE_FILE ("std_globals.jnc",   g_std_globalsSrc)
	JNC_LIB_SOURCE_FILE ("std_Guid.jnc",      g_std_GuidSrc)
	JNC_LIB_SOURCE_FILE ("std_Error.jnc",     g_std_ErrorSrc)
	JNC_LIB_SOURCE_FILE ("std_String.jnc",    g_std_StringSrc)
	JNC_LIB_SOURCE_FILE ("std_Buffer.jnc",    g_std_BufferSrc)
	JNC_LIB_SOURCE_FILE ("std_Array.jnc",     g_std_ArraySrc)
	JNC_LIB_SOURCE_FILE ("std_List.jnc",      g_std_ListSrc)
	JNC_LIB_SOURCE_FILE ("std_MapEntry.jnc",  g_std_MapEntrySrc)
	JNC_LIB_SOURCE_FILE ("std_HashTable.jnc", g_std_HashTableSrc)
	JNC_LIB_SOURCE_FILE ("std_RbTree.jnc",    g_std_RbTreeSrc)

	JNC_LIB_IMPORT ("std_globals.jnc")
	JNC_LIB_IMPORT ("std_Error.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (jnc_StdLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (HashTable)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (RbTree)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_StdLib_setStdIo (
	jnc_StdLib_StdInputFunc* getsFunc,
	jnc_StdLib_StdOutputFunc* printOutFunc,
	jnc_StdLib_StdOutputFunc* printErrFunc
	)
{
	g_getsFunc = getsFunc ? getsFunc : stdGets;
	g_printOutFunc = printOutFunc ? printOutFunc : stdPrintOut;
	g_printErrFunc = printErrFunc ? printErrFunc : stdPrintErr;
}

//..............................................................................
