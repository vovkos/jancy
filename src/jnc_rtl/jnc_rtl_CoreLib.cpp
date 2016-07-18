#include "pch.h"
#include "jnc_rtl_CoreLib.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_rt_VariantUtils.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//.............................................................................

size_t
dynamicSizeOf (DataPtr ptr)
{
	if (!ptr.m_validator)
		return 0;

	char* p = (char*) ptr.m_p;
	char* end = (char*) ptr.m_validator->m_rangeEnd;
	return p < end ? end - p : 0;
}

size_t
dynamicCountOf (
	DataPtr ptr,
	ct::Type* type
	)
{
	size_t maxSize = dynamicSizeOf (ptr);
	size_t typeSize = type->getSize ();
	return maxSize / (typeSize ? typeSize : 1);
}

DataPtr
dynamicCastDataPtr (
	DataPtr ptr,
	ct::Type* type
	)
{
	if (!ptr.m_validator)
		return g_nullPtr;
	
	Box* box = ptr.m_validator->m_targetBox;
	void* p = (box->m_flags & BoxFlag_StaticData) ? ((StaticDataBox*) box)->m_p : box + 1;
	if (ptr.m_p < p)
		return g_nullPtr;

	ct::Type* srcType = box->m_type;
	while (srcType->getTypeKind () == TypeKind_Array)
	{
		ct::ArrayType* arrayType = (ct::ArrayType*) srcType;
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

	ct::BaseTypeCoord coord;
	bool result = ((ct::StructType*) srcType)->findBaseTypeTraverse (type, &coord);
	if (!result)
		return g_nullPtr;

	ptr.m_p = (char*) p + coord.m_offset;
	return ptr;
}

IfaceHdr*
dynamicCastClassPtr (
	IfaceHdr* iface,
	ct::ClassType* type
	)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ct::ClassType* classType = (ct::ClassType*) iface->m_box->m_type;
	if (classType->cmp (type) == 0)
		return iface;

	ct::BaseTypeCoord coord;
	bool result = classType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return NULL;

	IfaceHdr* iface2 = (IfaceHdr*) ((uchar_t*) (iface->m_box + 1) + coord.m_offset);
	ASSERT (iface2->m_box == iface->m_box);
	return iface2;
}

bool
dynamicCastVariant (
	Variant variant,
	ct::Type* type,
	void* buffer
	)
{
	ct::Module* module = type->getModule ();

	ct::Value opValue (&variant, module->m_typeMgr.getPrimitiveType (TypeKind_Variant));
	ct::CastOperator* castOp = module->m_operatorMgr.getStdCastOperator (ct::StdCast_FromVariant);

	memset (buffer, 0, type->getSize ());
	return castOp->constCast (opValue, type, buffer);
}

IfaceHdr*
strengthenClassPtr (IfaceHdr* iface)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == TypeKind_Class);
	ct::ClassType* classType = (ct::ClassType*) iface->m_box->m_type;

	ct::ClassTypeKind classTypeKind = classType->getClassTypeKind ();
	return classTypeKind == ct::ClassTypeKind_FunctionClosure || classTypeKind == ct::ClassTypeKind_PropertyClosure ?
		((ct::ClosureClassType*) iface->m_box->m_type)->strengthen (iface) :
		(iface->m_box->m_flags & BoxFlag_ClassMark) && !(iface->m_box->m_flags & BoxFlag_Zombie) ? iface : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
primeStaticClass (
	Box* box,
	ct::ClassType* type
	)
{
	primeClass (box, type);
}

IfaceHdr*
tryAllocateClass (ct::ClassType* type)
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	return gcHeap->tryAllocateClass (type);
}

IfaceHdr*
allocateClass (ct::ClassType* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateClass (type);
}

DataPtr
tryAllocateData (ct::Type* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateData (type);
}

DataPtr
allocateData (ct::Type* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateData (type);
}

DataPtr
tryAllocateArray (
	ct::Type* type,
	size_t elementCount
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateArray (type, elementCount);
}

DataPtr
allocateArray (
	ct::Type* type,
	size_t elementCount
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateArray (type, elementCount);
}

DataPtrValidator* 
createDataPtrValidator (
	Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.createDataPtrValidator (box, rangeBegin, rangeLength);
}

void
gcSafePoint ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.safePoint ();
}

void
setGcShadowStackFrameMap (
	GcShadowStackFrame* frame,
	GcShadowStackFrameMap* map,
	bool isOpen
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.setFrameMap (frame, map, isOpen);
}

void
addStaticDestructor (StaticDestructFunc* destructFunc)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticDestructor (destructFunc);
}

void
addStaticClassDestructor (
	DestructFunc* destructFunc,
	IfaceHdr* iface
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticClassDestructor (destructFunc, iface);
}

void*
getTls ()
{
	Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	return tls + 1;
}

void
dynamicThrow()
{
	rt::Runtime::dynamicThrow ();
	ASSERT (false);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Variant
variantUnaryOperator (
	int opKind,
	Variant op
	)
{
	return rt::variantUnaryOperator ((UnOpKind) opKind, op);
}

Variant
variantBinaryOperator (
	int opKind,
	Variant op1,
	Variant op2
	)
{
	return rt::variantBinaryOperator ((BinOpKind) opKind, op1, op2);
}

bool
variantRelationalOperator (
	int opKind,
	Variant op1,
	Variant op2
	)
{
	return rt::variantRelationalOperator ((BinOpKind) opKind, op1, op2);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
assertionFailure (
	const char* fileName,
	int line,
	const char* condition,
	const char* message
	)
{
	sl::String string;
	string.format ("%s(%d): assertion (%s) failed", fileName, line + 1, condition);
	if (message)
		string.appendFormat ("; %s", message);

	rt::Runtime::runtimeError (err::createStringError (string, string.getLength ()));
}

bool 
tryCheckDataPtrRangeDirect (
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

	void* rangeEnd = (char*) rangeBegin + rangeLength;
	if (p < rangeBegin ||  p > rangeEnd)
	{
		err::setFormatStringError ("data pointer %x out of range [%x:%x]", p, rangeBegin, rangeEnd);
		return false;
	}

	return true;
}

void 
checkDataPtrRangeDirect (
	const void* p,
	const void* rangeBegin,
	size_t rangeLength
	)
{
	bool result = tryCheckDataPtrRangeDirect (p, rangeBegin, rangeLength);
	if (!result)
		rt::Runtime::runtimeError (err::getLastError ());
}

bool 
tryCheckDataPtrRangeIndirect (
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	if (!p || !validator)
	{
		err::setStringError ("null data pointer access");
		return false;
	}

	void* end = (char*) p + size;
	if (p < validator->m_rangeBegin || end > validator->m_rangeEnd)
	{
		err::setFormatStringError ("data pointer %x out of range [%x:%x]", p, validator->m_rangeBegin, validator->m_rangeEnd);
		return false;
	}

	return true;
}


void 
checkDataPtrRangeIndirect (
	const void* p,
	size_t size,
	DataPtrValidator* validator
	)
{
	bool result = tryCheckDataPtrRangeIndirect (p, size, validator);
	if (!result)
		rt::Runtime::runtimeError (err::getLastError ());
}

bool 
tryCheckNullPtr (
	const void* p,
	ct::TypeKind typeKind
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
checkNullPtr (
	const void* p,
	ct::TypeKind typeKind
	)
{
	bool result = tryCheckNullPtr (p, typeKind);
	if (!result)
		rt::Runtime::runtimeError (err::getLastError ());
}

void
checkStackOverflow ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->checkStackOverflow ();
}

void
checkDivByZero_i32 (int32_t i)
{
	if (!i)
	{
		err::setStringError ("integer division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
checkDivByZero_i64 (int64_t i)
{
	if (!i)
	{
		err::setStringError ("integer division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
checkDivByZero_f32 (float f)
{
	if (!f)
	{
		err::setStringError ("floating point division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
checkDivByZero_f64 (double f)
{
	if (!f)
	{
		err::setStringError ("floating point division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void* 
tryLazyGetDynamicLibFunction (
	rtl::DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	ASSERT (lib->m_box->m_type->getTypeKind () == TypeKind_Class);
	ct::ClassType* type = (ct::ClassType*) lib->m_box->m_type;

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
lazyGetDynamicLibFunction (
	rtl::DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	void* p = tryLazyGetDynamicLibFunction (lib, index, name);
	if (!p)
		rt::Runtime::runtimeError (err::getLastError ());

	return p;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
appendFmtLiteral_a (
	FmtLiteral* fmtLiteral,
	const char* p,
	size_t length
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t newLength = fmtLiteral->m_length + length;
	if (newLength < 64)
		newLength = 64;

	if (fmtLiteral->m_maxLength < newLength)
	{
		size_t newMaxLength = sl::getMinPower2Ge (newLength);

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

	// adjust validator

	DataPtrValidator* validator = fmtLiteral->m_ptr.m_validator;
	validator->m_rangeEnd = (char*) validator->m_rangeBegin + fmtLiteral->m_length;
	return fmtLiteral->m_length;
}

void
prepareFormatString (
	sl::String* formatString,
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
appendFmtLiteral_v (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	Variant variant
	)
{
	bool result;

	if (!variant.m_type)
		return fmtLiteral->m_length;

	ct::Module* module = variant.m_type->getModule ();
	ct::TypeKind typeKind = variant.m_type->getTypeKind ();
	uint_t typeKindFlags = variant.m_type->getTypeKindFlags ();

	if (typeKindFlags & ct::TypeKindFlag_Integer)
	{
		ct::Value value (&variant, variant.m_type);

		if (variant.m_type->getSize () > 4)
		{
			result = module->m_operatorMgr.castOperator (&value, TypeKind_Int64);
			if (!result)
			{
				ASSERT (false);
				return fmtLiteral->m_length;
			}

			ASSERT (value.getValueKind () == ct::ValueKind_Const);
			int64_t x = *(int64_t*) value.getConstData ();

			return (typeKindFlags & ct::TypeKindFlag_Unsigned) ? 
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

			ASSERT (value.getValueKind () == ct::ValueKind_Const);
			int32_t x = *(int32_t*) value.getConstData ();

			return (typeKindFlags & ct::TypeKindFlag_Unsigned) ? 
				appendFmtLiteral_ui32 (fmtLiteral, fmtSpecifier, x) :
				appendFmtLiteral_i32 (fmtLiteral, fmtSpecifier, x);
		}
	}
	else if (typeKindFlags & ct::TypeKindFlag_Fp)
	{
		return typeKind == TypeKind_Float ? 
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(float*) &variant) :
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(double*) &variant);
	}

	ct::Type* type;
	const void* p;

	if (typeKind != TypeKind_DataRef)
	{
		type = variant.m_type;
		p = &variant;
	}
	else
	{
		type = ((ct::DataPtrType*) variant.m_type)->getTargetType ();
		p = variant.m_dataPtr.m_p;
	}

	if (isCharArrayType (type))
	{
		ct::ArrayType* arrayType = (ct::ArrayType*) type;
		size_t count = arrayType->getElementCount ();
		const char* c = (char*) p;

		// trim zero-termination

		while (count && c [count - 1] == 0)
			count--;

		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, c, count);
	}
	else if (isCharPtrType (type))
	{
		ct::DataPtrType* ptrType = (ct::DataPtrType*) type;
		ct::DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();
		
		if (ptrTypeKind == ct::DataPtrTypeKind_Normal)
			return appendFmtLiteral_p (fmtLiteral, fmtSpecifier, *(DataPtr*) &variant);

		const char* c = *(char**) p;
		size_t length = axl_strlen (c);

		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, c, length);
	}
	else
	{
		sl::String string;
		string.format ("(variant:%s)", type->getTypeString ().cc ());
		
		return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
	}
}

size_t
appendFmtLiteralImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* defaultType,
	...
	)
{
	AXL_VA_DECL (va, defaultType);

	char buffer1 [256];
	sl::String formatString (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	prepareFormatString (&formatString, fmtSpecifier, defaultType);

	char buffer2 [256];
	sl::String string (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	string.format_va (formatString, va);

	return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
}

size_t
appendFmtLiteralStringImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	const char* p,
	size_t length
	)
{
	if (!fmtSpecifier)
		return appendFmtLiteral_a (fmtLiteral, p, length);

	char buffer [256];
	sl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));

	if (p [length] != 0) // ensure zero-terminated
	{
		string.copy (p, length);
		p = string;
	}

	return appendFmtLiteralImpl (fmtLiteral, fmtSpecifier, "s", p);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
mapAllMulticastMethods (ct::Module* module)
{
	sl::ConstList <ct::MulticastClassType> mcTypeList = module->m_typeMgr.getMulticastClassTypeList ();
	sl::Iterator <ct::MulticastClassType> mcType = mcTypeList.getHead ();
	for (; mcType; mcType++)
		mapMulticastMethods (module, *mcType);

	return true;
}

void
multicastDestruct (Multicast* multicast)
{
	((MulticastImpl*) multicast)->destruct ();
}

void
multicastClear (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->clear ();
}

handle_t
multicastSet (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->setHandler (ptr);
}

handle_t
multicastSet_t (
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->setHandler_t (p);
}

handle_t
multicastAdd (
	Multicast* multicast,
	FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->addHandler (ptr);
}

handle_t
multicastAdd_t (
	Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->addHandler_t (p);
}

FunctionPtr
multicastRemove (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler (handle);
}

void*
multicastRemove_t (
	Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler_t (handle);
}

FunctionPtr
multicastGetSnapshot (Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->getSnapshot ();
}

void*
m_multicastMethodTable [ct::FunctionPtrTypeKind__Count] [ct::MulticastMethodKind__Count - 1] =
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
mapMulticastMethods (
	ct::Module* module,
	ct::MulticastClassType* multicastType
	)
{
	ct::FunctionPtrTypeKind ptrTypeKind = multicastType->getTargetType ()->getPtrTypeKind ();
	ASSERT (ptrTypeKind < ct::FunctionPtrTypeKind__Count);

	ct::Function* function = multicastType->getDestructor ();
	module->mapFunction (function, (void*) multicastDestruct);

	for (size_t i = 0; i < ct::MulticastMethodKind__Count - 1; i++)
	{
		function = multicastType->getMethod ((ct::MulticastMethodKind) i);
		module->mapFunction (function, m_multicastMethodTable [ptrTypeKind] [i]);
	}
}

//.............................................................................

} // namespace rtl
} // namespace jnc
