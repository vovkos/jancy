#include "pch.h"
#include "jnc_StdLib.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

size_t
StdLib::dynamicCountOf (
	DataPtr ptr,
	Type* type
	)
{
	size_t maxSize = ptr.m_validator ? ptr.m_validator->m_rangeLength : 0;
	size_t typeSize = type->getSize ();
	return maxSize / (typeSize ? typeSize : 1);
}

DataPtr
StdLib::dynamicCastDataPtr (
	DataPtr ptr,
	Type* type
	)
{
	if (!ptr.m_validator)
		return g_nullPtr;
	
	Box* box = ptr.m_validator->m_targetBox;
	void* p = (box->m_flags & BoxFlag_StaticData) ? ((StaticDataBox*) box)->m_p : box + 1;
	if (ptr.m_p < p)
		return g_nullPtr;

	Type* srcType = box->m_type;
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
	IfaceHdr* iface,
	ClassType* type
	)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) iface->m_box->m_type;
	if (classType->cmp (type) == 0)
		return iface;

	BaseTypeCoord coord;
	bool result = classType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return NULL;

	IfaceHdr* iface2 = (IfaceHdr*) ((uchar_t*) (iface->m_box + 1) + coord.m_offset);
	ASSERT (iface2->m_box == iface->m_box);
	return iface2;
}

bool
StdLib::dynamicCastVariant (
	Variant variant,
	Type* type,
	void* buffer
	)
{
	Module* module = type->getModule ();

	Value opValue (&variant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	CastOperator* castOp = module->m_operatorMgr.getStdCastOperator (StdCast_FromVariant);

	memset (buffer, 0, type->getSize ());
	return castOp->constCast (opValue, type, buffer);
}

IfaceHdr*
StdLib::strengthenClassPtr (IfaceHdr* iface)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) iface->m_box->m_type;

	ClassTypeKind classTypeKind = classType->getClassTypeKind ();
	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ClosureClassType*) iface->m_box->m_type)->strengthen (iface) :
		(iface->m_box->m_flags & BoxFlag_ClassMark) ? iface : NULL;
}

void
StdLib::primeStaticClass (
	Box* box,
	ClassType* type
	)
{
	prime (box, type);
}

IfaceHdr*
StdLib::tryAllocateClass (ClassType* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateClass (type);
}

IfaceHdr*
StdLib::allocateClass (ClassType* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateClass (type);
}

DataPtr
StdLib::tryAllocateData (Type* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateData (type);
}

DataPtr
StdLib::allocateData (Type* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateData (type);
}

DataPtr
StdLib::tryAllocateArray (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateArray (type, elementCount);
}

DataPtr
StdLib::allocateArray (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateArray (type, elementCount);
}

DataPtrValidator* 
StdLib::createDataPtrValidator (
	Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.createDataPtrValidator (box, rangeBegin, rangeLength);
}

void
StdLib::collectGarbage ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.collect ();
}

size_t
StdLib::strLen (DataPtr ptr)
{
	if (!ptr.m_validator || ptr.m_p < ptr.m_validator->m_rangeBegin)
		return 0;

	char* p0 = (char*) ptr.m_p;
	char* end = (char*) ptr.m_validator->m_rangeBegin + ptr.m_validator->m_rangeLength;

	char* p = p0;
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

	return jnc::strDup ((const char*) ptr.m_p, length);
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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t totalSize = size1 + size2;
	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (totalSize);
	if (!resultPtr.m_p)
		return g_nullPtr;

	char* p = (char*) resultPtr.m_p;

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);
	else
		memset (p, 0, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);
	else
		memset (p + size1, 0, size2);

	return resultPtr;
}

DataPtr
StdLib::memDup (
	DataPtr ptr,
	size_t size
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
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

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
	mt::Event m_threadStartedEvent;
};

#if (_AXL_ENV == AXL_ENV_WIN)

DWORD
WINAPI
StdLib::threadFunc (PVOID context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	jnc::FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN (context->m_runtime);
	context->m_threadStartedEvent.signal ();
	
	((void (__cdecl*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END ();

	return 0;
}

void
StdLib::sleep (uint32_t msCount)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.enterWaitRegion ();
	g::sleep (msCount);
	runtime->m_gcHeap.leaveWaitRegion ();
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	DWORD threadId;
	HANDLE h = ::CreateThread (NULL, 0, StdLib::threadFunc, &context, 0, &threadId);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

void*
StdLib::threadFunc (void* context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	jnc::FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN (context->m_runtime);
	context->m_threadStartedEvent.signal ();
	
	((void (__cdecl*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END ();

	return NULL;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	pthread_t thread;
	int result = pthread_create (&thread, NULL, StdLib::threadFunc, &context);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return result == 0;
}

#endif

DataPtr
StdLib::getErrorPtr (const err::ErrorData* errorData)
{
	size_t size = errorData->m_size;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, errorData, size);
	return resultPtr;
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
StdLib::addStaticDestructor (StaticDestructFunc* destructFunc)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticDestructor (destructFunc);
}

void
StdLib::addStaticClassDestructor (
	DestructFunc* destructFunc,
	jnc::IfaceHdr* iface
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticClassDestructor (destructFunc, iface);
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

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, string.cc (), length);
	return resultPtr;
}

void*
StdLib::getTls ()
{
	Tls* tls = getCurrentThreadTls ();
	ASSERT (tls);

	return tls + 1;
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
		size_t newMaxLength = rtl::getMinPower2Ge (newLength);

		DataPtr ptr = runtime->m_gcHeap.tryAllocateBuffer (newMaxLength + 1);
		if (!ptr.m_p)
			return fmtLiteral->m_length;

		if (fmtLiteral->m_length)
			memcpy (ptr.m_p, fmtLiteral->m_ptr.m_p, fmtLiteral->m_length);

		fmtLiteral->m_ptr = ptr;
		fmtLiteral->m_maxLength = newMaxLength;
	}

	char* dst = (char*) fmtLiteral->m_ptr.m_p;
	memcpy (dst + fmtLiteral->m_length, p, length);
	fmtLiteral->m_length += length;
	dst [fmtLiteral->m_length] = 0;

	fmtLiteral->m_ptr.m_validator->m_rangeLength = fmtLiteral->m_length; // adjust validator
	return fmtLiteral->m_length;
}

void
StdLib::prepareFormatString (
	rtl::String* formatString,
	const char* fmtSpecifier,
	const char* defaultType
	)
{
	if (!fmtSpecifier)
	{
		formatString->copy ('%');
		formatString->append (defaultType);
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
StdLib::appendFmtLiteral_v (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	Variant variant
	)
{
	bool result;

	if (!variant.m_type)
		return fmtLiteral->m_length;

	Module* module = variant.m_type->getModule ();
	TypeKind typeKind = variant.m_type->getTypeKind ();
	uint_t typeKindFlags = variant.m_type->getTypeKindFlags ();

	if (typeKindFlags & TypeKindFlag_Integer)
	{
		Value value (&variant, variant.m_type);

		if (variant.m_type->getSize () > 4)
		{
			result = module->m_operatorMgr.castOperator (&value, TypeKind_Int64);
			if (!result)
			{
				ASSERT (false);
				return fmtLiteral->m_length;
			}

			ASSERT (value.getValueKind () == ValueKind_Const);
			int64_t x = *(int64_t*) value.getConstData ();

			return (typeKindFlags & TypeKindFlag_Unsigned) ? 
				appendFmtLiteral_ui64 (fmtLiteral, fmtSpecifier, x) :
				appendFmtLiteral_i64 (fmtLiteral, fmtSpecifier, x);
		}
		else
		{
			result = module->m_operatorMgr.castOperator (&value, TypeKind_Int32);
			if (!result)
			{
				ASSERT (false);
				return fmtLiteral->m_length;
			}

			ASSERT (value.getValueKind () == ValueKind_Const);
			int32_t x = *(int32_t*) value.getConstData ();

			return (typeKindFlags & TypeKindFlag_Unsigned) ? 
				appendFmtLiteral_ui32 (fmtLiteral, fmtSpecifier, x) :
				appendFmtLiteral_i32 (fmtLiteral, fmtSpecifier, x);
		}
	}
	else if (typeKindFlags & TypeKindFlag_Fp)
	{
		return typeKind == TypeKind_Float ? 
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(float*) &variant) :
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(double*) &variant);
	}
	else if (isCharArrayType (variant.m_type))
	{
		ArrayType* arrayType = (ArrayType*) variant.m_type;
		size_t count = arrayType->getElementCount ();
		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, (char*) &variant, count);
	}
	else if (isCharPtrType (variant.m_type))
	{
		DataPtrType* ptrType = (DataPtrType*) variant.m_type;
		DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();
		
		if (ptrTypeKind == DataPtrTypeKind_Normal)
			return appendFmtLiteral_p (fmtLiteral, fmtSpecifier, *(DataPtr*) &variant);

		const char* p = (const char*) variant.m_p;
		size_t length = strlen (p);

		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, p, length);
	}
	else
	{
		StdType stdType = variant.m_type->getStdType ();
		switch (stdType)
		{
		case StdType_String:
		case StdType_BufferRef:
		case StdType_ConstBuffer:
			return appendFmtLiteral_s (fmtLiteral, fmtSpecifier, *(String*) &variant);

		case StdType_StringRef:
		case StdType_ConstBufferRef:
			return appendFmtLiteral_sr (fmtLiteral, fmtSpecifier, *(StringRef*) &variant);
		}
	}

	char defaultString [] = "(variant)";
	return appendFmtLiteral_a (fmtLiteral, defaultString, lengthof (defaultString));
}

size_t
StdLib::appendFmtLiteralImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* defaultType,
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
	if (!fmtSpecifier)
		return appendFmtLiteral_a (fmtLiteral, p, length);

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));

	if (p [length] != 0) // ensure zero-terminated
	{
		string.copy (p, length);
		p = string;
	}

	return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "s", p);
}

bool 
StdLib::tryCheckDataPtrRangeDirect (
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
	)
{
	if (!p)
	{
		err::setStringError ("null data pointer access");
		return false;
	}

	void* rangeEnd = rangeEnd = (char*) rangeBegin + rangeLength;
	if (p < rangeBegin ||  p > rangeEnd)
	{
		err::setFormatStringError ("data pointer %x out of range [%x:%x]", p, rangeBegin, rangeEnd);
		return false;
	}

	return true;
}

void 
StdLib::checkDataPtrRangeDirect (
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
	)
{
	bool result = tryCheckDataPtrRangeDirect (p, rangeBegin, rangeLength);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

bool 
StdLib::tryCheckDataPtrRangeIndirect (
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	if (!validator)
	{
		err::setStringError ("null data pointer access");
		return false;
	}

	if (validator->m_rangeLength < size)
	{
		err::setFormatStringError (
			"data pointer [%x:%x] out of range [%x:%x]", 
			p, 
			(char*) p + size,
			validator->m_rangeBegin, 
			(char*) validator->m_rangeBegin + validator->m_rangeLength
			);

		return false;
	}

	return tryCheckDataPtrRangeDirect (
		p, 
		validator->m_rangeBegin, 
		validator->m_rangeLength - size
		);
}


void 
StdLib::checkDataPtrRangeIndirect (
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	bool result = tryCheckDataPtrRangeIndirect (p, size, validator);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

bool 
StdLib::tryCheckNullPtr (
	const void* p,
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
	const void* p,
	TypeKind typeKind
	)
{
	bool result = tryCheckNullPtr (p, typeKind);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

void
StdLib::checkStackOverflow ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->checkStackOverflow ();
}

void* 
StdLib::tryLazyGetLibraryFunction (
	Library* library,
	size_t index,
	const char* name
	)
{
	ASSERT (library->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* type = (ClassType*) library->m_box->m_type;

	if (!library->m_handle)
	{
		err::setFormatStringError ("library '%s' is not loaded yet", type->getQualifiedName ().cc ());
		return NULL;
	}

	size_t librarySize = type->getIfaceStructType ()->getSize ();
	size_t functionCount = (librarySize - sizeof (Library)) / sizeof (void*);

	if (index >= functionCount)
	{
		err::setFormatStringError ("index #%d out of range for library '%s'", index, type->getQualifiedName ().cc ());
		return NULL;
	}

	void** functionTable = (void**) (library + 1);
	if (functionTable [index])
		return functionTable [index];

	void* function = library->getFunctionImpl (name);
	if (!function)
		return NULL;

	functionTable [index] = function;
	return function;
}

void* 
StdLib::lazyGetLibraryFunction (
	Library* library,
	size_t index,
	const char* name
	)
{
	void* p = tryLazyGetLibraryFunction (library, index, name);
	if (!p)
		Runtime::runtimeError (err::getLastError ());

	return p;
}

//.............................................................................

DataPtr
strDup (
	const char* p,
	size_t length
	)
{
	if (length == -1)
		length = p ? strlen (p) : 0;

	if (!length)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, length);

	return resultPtr;
}

DataPtr
memDup (
	const void* p,
	size_t size
	)
{
	if (!size)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, size);

	return resultPtr;
}


//.............................................................................

} // namespace jnc {
