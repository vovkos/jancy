#include "pch.h"
#include "jnc_std_StdLib.h"

namespace jnc {
namespace ext {

//.............................................................................

void
initStdLib (ExtensionLibHost* host)
{
	std::g_stdLibCacheSlot = host->getLibCacheSlot (std::g_stdLibGuid);
}

ExtensionLib*
getStdLib ()
{
	return sl::getSimpleSingleton <std::StdLib> ();
}

//.............................................................................

} // namespace ext

namespace std {

//.............................................................................

int
StdLib::strCmp (
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
StdLib::striCmp (
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

DataPtr 
StdLib::strChr (
	DataPtr ptr,
	int c
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
StdLib::strStr (
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
StdLib::strCat (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	size_t length1 = strLen (ptr1);
	size_t length2 = strLen (ptr2);

	return memCat (ptr1, length1, ptr2, length2 + 1);
}

DataPtr
StdLib::strDup (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return rt::strDup ((const char*) ptr.m_p, length);
}

int
StdLib::memCmp (
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
StdLib::memChr (
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
StdLib::memMem (
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
StdLib::memCpy (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memcpy (dstPtr.m_p, srcPtr.m_p, size);
}

void
StdLib::memSet (
	DataPtr ptr,
	int c,
	size_t size
	)
{
	if (ptr.m_p)
		memset (ptr.m_p, c, size);
}

DataPtr
StdLib::memCat (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t totalSize = size1 + size2;
	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (totalSize);
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
StdLib::memDup (
	DataPtr ptr,
	size_t size
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (ptr.m_p)
		memcpy (resultPtr.m_p, ptr.m_p, size);
	else
		memset (resultPtr.m_p, 0, size);

	return resultPtr;
}

DataPtr
StdLib::getErrorPtr (const err::ErrorHdr* error)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (error->m_size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, error, error->m_size);
	return resultPtr;
}

DataPtr
StdLib::format (
	DataPtr formatStringPtr,
	...
	)
{
	AXL_VA_DECL (va, formatStringPtr);

	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va ((const char*) formatStringPtr.m_p, va);
	size_t length = string.getLength ();

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, string.cc (), length);
	return resultPtr;
}

void
StdLib::collectGarbage ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.collect ();
}

//.............................................................................

} // namespace std
} // namespace jnc
