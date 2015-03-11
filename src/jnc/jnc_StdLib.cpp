#include "pch.h"
#include "jnc_StdLib.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

size_t
StdLib::dynamicSizeOf (DataPtr ptr)
{
	size_t maxSize = (char*) ptr.m_rangeEnd > (char*) ptr.m_p ? (char*) ptr.m_rangeEnd - (char*) ptr.m_p : 0;

	if (!ptr.m_object)
		return maxSize;

	#pragma AXL_TODO ("find field pointed to by ptr and calculate sizeof accordingly")
	return maxSize;
}

size_t
StdLib::dynamicCountOf (
	DataPtr ptr,
	Type* type
	)
{
	size_t maxSize = (char*) ptr.m_rangeEnd > (char*) ptr.m_p ? (char*) ptr.m_rangeEnd - (char*) ptr.m_p : 0;
	size_t typeSize = type->getSize ();
	size_t maxCount = maxSize / (typeSize ? typeSize : 1);

	if (!ptr.m_object)
		return maxCount;

	#pragma AXL_TODO ("find field pointed to by ptr and calculate countof accordingly")
	return maxCount;
}

DataPtr
StdLib::dynamicCastDataPtr (
	DataPtr ptr,
	Type* type
	)
{
	if (!ptr.m_object)
		return g_nullPtr;
	
	void* p = (ptr.m_object->m_flags  & (ObjHdrFlag_Stack | ObjHdrFlag_Static)) ?
		((VariableObjHdr*) ptr.m_object)->m_p :
		ptr.m_object + 1;

	if (ptr.m_p < p)
		return g_nullPtr;

	Type* srcType = ptr.m_object->m_type;
	while (srcType->getTypeKind () == TypeKind_Array)
	{
		ArrayType* arrayType = (ArrayType*) srcType;
		srcType = arrayType->getElementType ();
		
		size_t srcTypeSize = srcType->getSize ();
		if (!srcTypeSize)
			srcTypeSize = 1;

		size_t offset = ((char*) ptr.m_p - (char*) p) % srcTypeSize;
		p = (char*) ptr.m_p - offset;
	}

	if (srcType->cmp (type) == 0)
	{
		ptr.m_p = p;
		return ptr;
	}

	#pragma AXL_TODO ("find field pointed to by ptr and do cast accordingly")

	if (srcType->getTypeKind () != TypeKind_Struct)
		return g_nullPtr;

	BaseTypeCoord coord;
	bool result = ((StructType*) srcType)->findBaseTypeTraverse (type, &coord);
	if (!result)
		return g_nullPtr;

	ptr.m_p = (char*) p + coord.m_offset;
	return ptr;
}

IfaceHdr*
StdLib::dynamicCastClassPtr (
	IfaceHdr* p,
	ClassType* type
	)
{
	if (!p)
		return NULL;

	if (p->m_object->m_type->cmp (type) == 0)
		return p;

	BaseTypeCoord coord;
	bool result = p->m_object->m_classType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return NULL;

	IfaceHdr* p2 = (IfaceHdr*) ((uchar_t*) (p->m_object + 1) + coord.m_offset);
	ASSERT (p2->m_object == p->m_object);
	return p2;
}

IfaceHdr*
StdLib::strengthenClassPtr (IfaceHdr* p)
{
	if (!p)
		return NULL;

	ClassTypeKind classTypeKind = p->m_object->m_classType->getClassTypeKind ();
	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ClosureClassType*) p->m_object->m_type)->strengthen (p) :
		(!(p->m_object->m_flags & ObjHdrFlag_Dead)) ? p : NULL;
}

void*
StdLib::gcAllocate (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->gcAllocate (type, elementCount);
}

void*
StdLib::gcTryAllocate (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->gcTryAllocate (type, elementCount);
}

void
StdLib::gcEnter ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);
	runtime->gcEnter ();
}

void
StdLib::gcLeave ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->gcLeave ();
}

void
StdLib::gcPulse ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->gcPulse ();
}

void
StdLib::runGc ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->runGc ();
}

size_t
StdLib::strLen (DataPtr ptr)
{
	char* p = (char*) ptr.m_p;
	if (!p)
		return 0;

	char* p0 = p;
	char* end = (char*) ptr.m_rangeEnd;
	while (*p && p < end)
		p++;

	return p - p0;
}

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
	DataPtr resultPtr = { 0 };

	if (!ptr.m_p)
		return resultPtr;

	resultPtr = ptr;
	resultPtr.m_p = strchr ((char*) ptr.m_p, c);
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

	DataPtr resultPtr = { 0 };

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	if (!p)
		return resultPtr;

	p [length] = 0; // ensure zero-termination just in case

	if (ptr.m_p)
		memcpy (p, ptr.m_p, length);
	else
		memset (p, 0, length);

	resultPtr.m_p = p;
	resultPtr.m_rangeBegin = p;
	resultPtr.m_rangeEnd = p + length;
	resultPtr.m_object = jnc::getStaticObjHdr ();
	return resultPtr;
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
	DataPtr resultPtr = { 0 };

	if (!ptr.m_p)
		return resultPtr;

	resultPtr = ptr;
	resultPtr.m_p = memchr (ptr.m_p, c, size);
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
}

DataPtr
StdLib::memCat (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	DataPtr resultPtr = { 0 };

	size_t totalSize = size1 + size2;
	char* p = (char*) AXL_MEM_ALLOC (totalSize + 1);
	if (!p)
		return resultPtr;

	p [totalSize] = 0; // ensure zero-termination just in case

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);
	else
		memset (p, 0, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);
	else
		memset (p + size1, 0, size2);

	resultPtr.m_p = p;
	resultPtr.m_rangeBegin = p;
	resultPtr.m_rangeEnd = p + totalSize;
	resultPtr.m_object = jnc::getStaticObjHdr ();
	return resultPtr;
}

DataPtr
StdLib::memDup (
	DataPtr ptr,
	size_t size
	)
{
	DataPtr resultPtr = { 0 };

	char* p = (char*) AXL_MEM_ALLOC (size + 1);
	if (!p)
		return resultPtr;

	p [size] = 0; // ensure zero-termination just in case

	if (ptr.m_p)
		memcpy (p, ptr.m_p, size);
	else
		memset (p, 0, size);

	resultPtr.m_p = p;
	resultPtr.m_rangeBegin = p;
	resultPtr.m_rangeEnd = p + size;
	resultPtr.m_object = jnc::getStaticObjHdr ();
	return resultPtr;
}

#if (_AXL_ENV == AXL_ENV_WIN)

intptr_t
StdLib::getCurrentThreadId ()
{
	return ::GetCurrentThreadId ();
}

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
};

DWORD
WINAPI
StdLib::threadProc (PVOID rawContext)
{
	ThreadContext* context = (ThreadContext*) rawContext;
	FunctionPtr ptr = context->m_ptr;
	Runtime* runtime = context->m_runtime;
	AXL_MEM_DELETE (context);

	ScopeThreadRuntime scopeRuntime (runtime);
	getTlsMgr ()->getTls (runtime); // register thread right away

	((void (__cdecl*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	return 0;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext* context = AXL_MEM_NEW (ThreadContext);
	context->m_ptr = ptr;
	context->m_runtime = runtime;

	DWORD threadId;
	HANDLE h = ::CreateThread (NULL, 0, StdLib::threadProc, context, 0, &threadId);
	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

intptr_t
StdLib::getCurrentThreadId ()
{
	return (intptr_t) pthread_self ();
}

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
};

void*
StdLib::threadProc (void* rawContext)
{
	ThreadContext* context = (ThreadContext*) rawContext;
	FunctionPtr ptr = context->m_ptr;
	Runtime* runtime = context->m_runtime;
	AXL_MEM_DELETE (context);

	ScopeThreadRuntime scopeRuntime (runtime);
	getTlsMgr ()->getTls (runtime); // register thread right away

	((void (*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	return NULL;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext* context = AXL_MEM_NEW (ThreadContext);
	context->m_ptr = ptr;
	context->m_runtime = runtime;

	pthread_t thread;
	int result = pthread_create (&thread, NULL, StdLib::threadProc, context);
	return result == 0;
}

#endif

DataPtr
StdLib::getErrorPtr (const err::ErrorData* errorData)
{
	size_t size = errorData->m_size;

	void* p = AXL_MEM_ALLOC (size);
	memcpy (p , errorData, size);

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + size;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

void
StdLib::assertionFailure (
	const char* fileName,
	int line,
	const char* condition,
	const char* message
	)
{
	rtl::String string;
	string.format ("%s(%d): assertion (%s) failed", fileName, line + 1, condition);
	if (message)
		string.appendFormat ("; %s", message);

	Runtime::runtimeError (err::createStringError (string, string.getLength ()));
}

void
StdLib::addStaticDestructor (StaticDestructor* dtor)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->addStaticDestructor (dtor);
}

void
StdLib::addDestructor (
	Destructor* dtor,
	jnc::IfaceHdr* iface
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->addDestructor (dtor, iface);
}

DataPtr
StdLib::format (
	DataPtr formatStringPtr,
	...
	)
{
	AXL_VA_DECL (va, formatStringPtr);

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va ((const char*) formatStringPtr.m_p, va);
	size_t length = string.getLength ();

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	memcpy (p, string.cc (), length);
	p [length] = 0;

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + length + 1;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

void*
StdLib::getTls ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->getTls () + 1;
}

size_t
StdLib::appendFmtLiteral_a (
	FmtLiteral* fmtLiteral,
	const char* p,
	size_t length
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t newLength = fmtLiteral->m_length + length;
	if (newLength < 64)
		newLength = 64;

	if (fmtLiteral->m_maxLength < newLength)
	{
		Module* module = runtime->getFirstModule ();
		ASSERT (module);

		size_t newMaxLength = rtl::getMinPower2Ge (newLength);
		char* p = (char*) runtime->gcAllocate (newMaxLength + 1); // for zero-termination
		memcpy (p, fmtLiteral->m_p, fmtLiteral->m_length);

		fmtLiteral->m_p = p;
		fmtLiteral->m_maxLength = newMaxLength;
	}

	memcpy (fmtLiteral->m_p + fmtLiteral->m_length, p, length);
	fmtLiteral->m_length += length;
	fmtLiteral->m_p [fmtLiteral->m_length] = 0;

	return fmtLiteral->m_length;
}

void
StdLib::prepareFormatString (
	rtl::String* formatString,
	const char* fmtSpecifier,
	char defaultType
	)
{
	if (!fmtSpecifier)
	{
		char formatBuffer [2] = { '%', defaultType };
		formatString->copy (formatBuffer, 2);
		return;
	}

	formatString->clear ();

	if (fmtSpecifier [0] != '%')
		formatString->copy ('%');

	formatString->append (fmtSpecifier);

	size_t length = formatString->getLength ();
	if (!isalpha (formatString->cc () [length - 1]))
		formatString->append (defaultType);
}

size_t
StdLib::appendFmtLiteral_p (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	DataPtr ptr
	)
{
	if (!ptr.m_p)
		return fmtLiteral->m_length;

	char* p = (char*) ptr.m_p;
	while (*p && p < ptr.m_rangeEnd)
		p++;

	if (!fmtSpecifier || !*fmtSpecifier)
	{
		size_t length = p - (char*) ptr.m_p;
		return appendFmtLiteral_a (fmtLiteral, (char*) ptr.m_p, length);
	}

	char buffer1 [256];
	rtl::String formatString (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	prepareFormatString (&formatString, fmtSpecifier, 's');

	char buffer2 [256];
	rtl::String string (ref::BufKind_Stack, buffer2, sizeof (buffer2));

	if (p < ptr.m_rangeEnd) // null terminated
	{
		ASSERT (!*p);
		string.format (formatString, ptr.m_p);
	}
	else
	{
		char buffer3 [256];
		rtl::String nullTermString (ref::BufKind_Stack, buffer3, sizeof (buffer3));
		string.format (formatString, nullTermString.cc ());
	}

	return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
}

size_t
StdLib::appendFmtLiteralImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	char defaultType,
	...
	)
{
	AXL_VA_DECL (va, defaultType);

	char buffer1 [256];
	rtl::String formatString (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	prepareFormatString (&formatString, fmtSpecifier, defaultType);

	char buffer2 [256];
	rtl::String string (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	string.format_va (formatString, va);

	return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
}

size_t
StdLib::appendFmtLiteralStringImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* p,
	size_t length
	)
{
	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));

	if (p [length] != 0) // ensure zero-terminated
	{
		string.copy (p, length);
		p = string;
	}

	return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, 's', p);
}

bool 
StdLib::tryCheckDataPtrRange (
	void* p,
	size_t size,
	void* rangeBegin,
	void* rangeEnd
	)
{
	if (!p)
	{
		err::setStringError ("null data pointer access");
		return false;
	}

	if ((char*) p < (char*) rangeBegin || 
		(char*) p + size > (char*) rangeEnd)
	{
		err::setFormatStringError ("data pointer %x out of range [%x:%x]", p, rangeBegin, rangeEnd);
		return false;
	}

	return true;
}

void 
StdLib::checkDataPtrRange (
	void* p,
	size_t size,
	void* rangeBegin,
	void* rangeEnd
	)
{
	bool result = tryCheckDataPtrRange (p, size, rangeBegin, rangeEnd);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

bool 
StdLib::tryCheckNullPtr (
	void* p,
	TypeKind typeKind
	)
{
	if (p)
		return true;

	switch (typeKind)
	{
	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		err::setStringError ("null class pointer access");
		break;

	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
		err::setStringError ("null function pointer access");
		break;

	case TypeKind_PropertyPtr:
	case TypeKind_PropertyRef:
		err::setStringError ("null property pointer access");
		break;

	default:
		err::setStringError ("null pointer access");
	}

	return false;
}

void
StdLib::checkNullPtr (
	void* p,
	TypeKind typeKind
	)
{
	bool result = tryCheckNullPtr (p, typeKind);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

//.............................................................................

} // namespace jnc {
