#include "pch.h"
#include "jnc_CoreLib.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

size_t
CoreLib::dynamicCountOf (
	DataPtr ptr,
	Type* type
	)
{
	size_t maxSize = ptr.m_validator ? ptr.m_validator->m_rangeLength : 0;
	size_t typeSize = type->getSize ();
	return maxSize / (typeSize ? typeSize : 1);
}

DataPtr
CoreLib::dynamicCastDataPtr (
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
CoreLib::dynamicCastClassPtr (
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
CoreLib::dynamicCastVariant (
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
CoreLib::strengthenClassPtr (IfaceHdr* iface)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) iface->m_box->m_type;

	ClassTypeKind classTypeKind = classType->getClassTypeKind ();
	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ClosureClassType*) iface->m_box->m_type)->strengthen (iface) :
		(iface->m_box->m_flags & BoxFlag_ClassMark) && !(iface->m_box->m_flags & BoxFlag_Zombie) ? iface : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CoreLib::primeStaticClass (
	Box* box,
	ClassType* type
	)
{
	prime (box, type);
}

IfaceHdr*
CoreLib::tryAllocateClass (ClassType* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateClass (type);
}

IfaceHdr*
CoreLib::allocateClass (ClassType* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateClass (type);
}

DataPtr
CoreLib::tryAllocateData (Type* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateData (type);
}

DataPtr
CoreLib::allocateData (Type* type)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateData (type);
}

DataPtr
CoreLib::tryAllocateArray (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateArray (type, elementCount);
}

DataPtr
CoreLib::allocateArray (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateArray (type, elementCount);
}

DataPtrValidator* 
CoreLib::createDataPtrValidator (
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
CoreLib::gcSafePoint ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.safePoint ();
}

void
CoreLib::addStaticDestructor (StaticDestructFunc* destructFunc)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticDestructor (destructFunc);
}

void
CoreLib::addStaticClassDestructor (
	DestructFunc* destructFunc,
	IfaceHdr* iface
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticClassDestructor (destructFunc, iface);
}

void*
CoreLib::getTls ()
{
	Tls* tls = getCurrentThreadTls ();
	ASSERT (tls);

	return tls + 1;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CoreLib::assertionFailure (
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

bool 
CoreLib::tryCheckDataPtrRangeDirect (
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
CoreLib::checkDataPtrRangeDirect (
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
CoreLib::tryCheckDataPtrRangeIndirect (
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
CoreLib::checkDataPtrRangeIndirect (
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
CoreLib::tryCheckNullPtr (
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
CoreLib::checkNullPtr (
	const void* p,
	TypeKind typeKind
	)
{
	bool result = tryCheckNullPtr (p, typeKind);
	if (!result)
		Runtime::runtimeError (err::getLastError ());
}

void
CoreLib::checkStackOverflow ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->checkStackOverflow ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void* 
CoreLib::tryLazyGetDynamicLibFunction (
	DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	ASSERT (lib->m_box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* type = (ClassType*) lib->m_box->m_type;

	if (!lib->m_handle)
	{
		err::setFormatStringError ("dynamiclib '%s' is not loaded yet", type->getQualifiedName ().cc ());
		return NULL;
	}

	size_t librarySize = type->getIfaceStructType ()->getSize ();
	size_t functionCount = (librarySize - sizeof (DynamicLib)) / sizeof (void*);

	if (index >= functionCount)
	{
		err::setFormatStringError ("index #%d out of range for dynamiclib '%s'", index, type->getQualifiedName ().cc ());
		return NULL;
	}

	void** functionTable = (void**) (lib + 1);
	if (functionTable [index])
		return functionTable [index];

	void* function = lib->getFunctionImpl (name);
	if (!function)
		return NULL;

	functionTable [index] = function;
	return function;
}

void* 
CoreLib::lazyGetDynamicLibFunction (
	DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	void* p = tryLazyGetDynamicLibFunction (lib, index, name);
	if (!p)
		Runtime::runtimeError (err::getLastError ());

	return p;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
CoreLib::appendFmtLiteral_a (
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
CoreLib::prepareFormatString (
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
CoreLib::appendFmtLiteral_v (
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

	char defaultString [] = "(variant)";
	return appendFmtLiteral_a (fmtLiteral, defaultString, lengthof (defaultString));
}

size_t
CoreLib::appendFmtLiteralImpl (
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
CoreLib::appendFmtLiteralStringImpl (
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CoreLib::mapAllMulticastMethods (Module* module)
{
	rtl::ConstList <MulticastClassType> mcTypeList = module->m_typeMgr.getMulticastClassTypeList ();
	rtl::Iterator <MulticastClassType> mcType = mcTypeList.getHead ();
	for (; mcType; mcType++)
		mapMulticastMethods (module, *mcType);

	return true;
}

void
CoreLib::multicastDestruct (Multicast* multicast)
{
	((MulticastImpl*) multicast)->~MulticastImpl ();
}

void
CoreLib::multicastClear (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->clear ();
}

handle_t
CoreLib::multicastSet (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->setHandler (ptr);
}

handle_t
CoreLib::multicastSet_t (
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->setHandler_t (p);
}

handle_t
CoreLib::multicastAdd (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->addHandler (ptr);
}

handle_t
CoreLib::multicastAdd_t (
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->addHandler_t (p);
}

FunctionPtr
CoreLib::multicastRemove (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler (handle);
}

void*
CoreLib::multicastRemove_t (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler_t (handle);
}

FunctionPtr
CoreLib::multicastGetSnapshot (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->getSnapshot ();
}

void*
CoreLib::m_multicastMethodTable [FunctionPtrTypeKind__Count] [MulticastMethodKind__Count - 1] =
{
	{
		(void*) multicastClear,
		(void*) multicastSet,
		(void*) multicastAdd,
		(void*) multicastRemove,
		(void*) multicastGetSnapshot,
	},

	{
		(void*) multicastClear,
		(void*) multicastSet,
		(void*) multicastAdd,
		(void*) multicastRemove,
		(void*) multicastGetSnapshot,
	},

	{
		(void*) multicastClear,
		(void*) multicastSet_t,
		(void*) multicastAdd_t,
		(void*) multicastRemove_t,
		(void*) multicastGetSnapshot,
	},
};

void
CoreLib::mapMulticastMethods (
	Module* module,
	MulticastClassType* multicastType
	)
{
	FunctionPtrTypeKind ptrTypeKind = multicastType->getTargetType ()->getPtrTypeKind ();
	ASSERT (ptrTypeKind < FunctionPtrTypeKind__Count);

	Function* function = multicastType->getDestructor ();
	module->mapFunction (function, (void*) multicastDestruct);

	for (size_t i = 0; i < MulticastMethodKind__Count - 1; i++)
	{
		function = multicastType->getMethod ((MulticastMethodKind) i);
		module->mapFunction (function, m_multicastMethodTable [ptrTypeKind] [i]);
	}
}

//.............................................................................

} // namespace jnc {
