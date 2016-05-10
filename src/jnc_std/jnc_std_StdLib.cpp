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
getStdLib (ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	sl::callOnce (initStdLib, host, &onceFlag);
	return sl::getSimpleSingleton <std::StdLib> ();
}

//.............................................................................

} // namespace ext

namespace std {

//.............................................................................

int
StdLib::strCmp (
	rt::DataPtr ptr1,
	rt::DataPtr ptr2
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
	rt::DataPtr ptr1,
	rt::DataPtr ptr2
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return 
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		_stricmp ((char*) ptr1.m_p, (char*) ptr2.m_p);
}

rt::DataPtr 
StdLib::strChr (
	rt::DataPtr ptr,
	int c
	)
{
	if (!ptr.m_p)
		return rt::g_nullPtr;

	char* p = strchr ((char*) ptr.m_p, c);
	if (!p)
		return rt::g_nullPtr;

	rt::DataPtr resultPtr;
	resultPtr.m_p = 
	resultPtr.m_validator = ptr.m_validator;
	return resultPtr;
}

rt::DataPtr 
StdLib::strStr (
	rt::DataPtr ptr1,
	rt::DataPtr ptr2
	)
{
	if (!ptr1.m_p)
		return rt::g_nullPtr;

	if (!ptr2.m_p)
		return ptr1;

	char* p = strstr ((char*) ptr1.m_p, (char*) ptr2.m_p);
	if (!p)
		return rt::g_nullPtr;

	rt::DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr1.m_validator;
	return resultPtr;
}

rt::DataPtr
StdLib::strCat (
	rt::DataPtr ptr1,
	rt::DataPtr ptr2
	)
{
	size_t length1 = strLen (ptr1);
	size_t length2 = strLen (ptr2);

	return memCat (ptr1, length1, ptr2, length2 + 1);
}

rt::DataPtr
StdLib::strDup (
	rt::DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return rt::strDup ((const char*) ptr.m_p, length);
}

int
StdLib::memCmp (
	rt::DataPtr ptr1,
	rt::DataPtr ptr2,
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

rt::DataPtr 
StdLib::memChr (
	rt::DataPtr ptr,
	int c,
	size_t size
	)
{
	if (!ptr.m_p)
		return rt::g_nullPtr;

	void* p = memchr (ptr.m_p, c, size);
	if (!p)
		return rt::g_nullPtr;

	rt::DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr.m_validator;
	return resultPtr;
}

rt::DataPtr 
StdLib::memMem (
	rt::DataPtr ptr1,
	size_t size1,
	rt::DataPtr ptr2,
	size_t size2
	)
{
	if (!ptr1.m_p)
		return rt::g_nullPtr;

	if (!ptr2.m_p)
		return ptr1;

	void* p = sl::memMem (ptr1.m_p, size1, ptr2.m_p, size2);
	if (!p)
		return rt::g_nullPtr;

	rt::DataPtr resultPtr;
	resultPtr.m_p = p;
	resultPtr.m_validator = ptr1.m_validator;
	return resultPtr;
}

void
StdLib::memCpy (
	rt::DataPtr dstPtr,
	rt::DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memcpy (dstPtr.m_p, srcPtr.m_p, size);
}

void
StdLib::memSet (
	rt::DataPtr ptr,
	int c,
	size_t size
	)
{
	if (ptr.m_p)
		memset (ptr.m_p, c, size);
}

rt::DataPtr
StdLib::memCat (
	rt::DataPtr ptr1,
	size_t size1,
	rt::DataPtr ptr2,
	size_t size2
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t totalSize = size1 + size2;
	rt::DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (totalSize);
	if (!resultPtr.m_p)
		return rt::g_nullPtr;

	char* p = (char*) resultPtr.m_p;

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);

	return resultPtr;
}

rt::DataPtr
StdLib::memDup (
	rt::DataPtr ptr,
	size_t size
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return rt::g_nullPtr;

	if (ptr.m_p)
		memcpy (resultPtr.m_p, ptr.m_p, size);
	else
		memset (resultPtr.m_p, 0, size);

	return resultPtr;
}

rt::DataPtr
StdLib::getErrorPtr (const err::ErrorHdr* error)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (error->m_size);
	if (!resultPtr.m_p)
		return rt::g_nullPtr;

	memcpy (resultPtr.m_p, error, error->m_size);
	return resultPtr;
}

rt::DataPtr
StdLib::format (
	rt::DataPtr formatStringPtr,
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

	rt::DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return rt::g_nullPtr;

	memcpy (resultPtr.m_p, string.cc (), length);
	return resultPtr;
}

//.............................................................................

} // namespace std
} // namespace jnc
