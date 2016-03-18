#include "pch.h"
#include "jnc_rtl_CoreLib.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace rtl {

//.............................................................................

size_t
CoreLib::dynamicCountOf (
	rt::DataPtr ptr,
	ct::Type* type
	)
{
	size_t maxSize = ptr.m_validator ? ptr.m_validator->m_rangeLength : 0;
	size_t typeSize = type->getSize ();
	return maxSize / (typeSize ? typeSize : 1);
}

rt::DataPtr
CoreLib::dynamicCastDataPtr (
	rt::DataPtr ptr,
	ct::Type* type
	)
{
	if (!ptr.m_validator)
		return rt::g_nullPtr;
	
	rt::Box* box = ptr.m_validator->m_targetBox;
	void* p = (box->m_flags & rt::BoxFlag_StaticData) ? ((rt::StaticDataBox*) box)->m_p : box + 1;
	if (ptr.m_p < p)
		return rt::g_nullPtr;

	ct::Type* srcType = box->m_type;
	while (srcType->getTypeKind () == ct::TypeKind_Array)
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

	if (srcType->getTypeKind () != ct::TypeKind_Struct)
		return rt::g_nullPtr;

	ct::BaseTypeCoord coord;
	bool result = ((ct::StructType*) srcType)->findBaseTypeTraverse (type, &coord);
	if (!result)
		return rt::g_nullPtr;

	ptr.m_p = (char*) p + coord.m_offset;
	return ptr;
}

rt::IfaceHdr*
CoreLib::dynamicCastClassPtr (
	rt::IfaceHdr* iface,
	ct::ClassType* type
	)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == ct::TypeKind_Class);
	ct::ClassType* classType = (ct::ClassType*) iface->m_box->m_type;
	if (classType->cmp (type) == 0)
		return iface;

	ct::BaseTypeCoord coord;
	bool result = classType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return NULL;

	rt::IfaceHdr* iface2 = (rt::IfaceHdr*) ((uchar_t*) (iface->m_box + 1) + coord.m_offset);
	ASSERT (iface2->m_box == iface->m_box);
	return iface2;
}

bool
CoreLib::dynamicCastVariant (
	rt::Variant variant,
	ct::Type* type,
	void* buffer
	)
{
	ct::Module* module = type->getModule ();

	ct::Value opValue (&variant, module->m_typeMgr.getPrimitiveType (ct::TypeKind_Variant));
	ct::CastOperator* castOp = module->m_operatorMgr.getStdCastOperator (ct::StdCast_FromVariant);

	memset (buffer, 0, type->getSize ());
	return castOp->constCast (opValue, type, buffer);
}

rt::IfaceHdr*
CoreLib::strengthenClassPtr (rt::IfaceHdr* iface)
{
	if (!iface)
		return NULL;

	ASSERT (iface->m_box->m_type->getTypeKind () == ct::TypeKind_Class);
	ct::ClassType* classType = (ct::ClassType*) iface->m_box->m_type;

	ct::ClassTypeKind classTypeKind = classType->getClassTypeKind ();
	return classTypeKind == ct::ClassTypeKind_FunctionClosure || classTypeKind == ct::ClassTypeKind_PropertyClosure ?
		((ct::ClosureClassType*) iface->m_box->m_type)->strengthen (iface) :
		(iface->m_box->m_flags & rt::BoxFlag_ClassMark) && !(iface->m_box->m_flags & rt::BoxFlag_Zombie) ? iface : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CoreLib::primeStaticClass (
	rt::Box* box,
	ct::ClassType* type
	)
{
	primeClass (box, type);
}

rt::IfaceHdr*
CoreLib::tryAllocateClass (ct::ClassType* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateClass (type);
}

rt::IfaceHdr*
CoreLib::allocateClass (ct::ClassType* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateClass (type);
}

rt::DataPtr
CoreLib::tryAllocateData (ct::Type* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateData (type);
}

rt::DataPtr
CoreLib::allocateData (ct::Type* type)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateData (type);
}

rt::DataPtr
CoreLib::tryAllocateArray (
	ct::Type* type,
	size_t elementCount
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.tryAllocateArray (type, elementCount);
}

rt::DataPtr
CoreLib::allocateArray (
	ct::Type* type,
	size_t elementCount
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.allocateArray (type, elementCount);
}

rt::DataPtrValidator* 
CoreLib::createDataPtrValidator (
	rt::Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->m_gcHeap.createDataPtrValidator (box, rangeBegin, rangeLength);
}

void
CoreLib::gcSafePoint ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.safePoint ();
}

void
CoreLib::collectGarbage ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.collect ();
}

void
CoreLib::setGcShadowStackFrameMap (
	rt::GcShadowStackFrame* frame,
	rt::GcShadowStackFrameMap* map,
	bool isOpen
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.setFrameMap (frame, map, isOpen);
}

void
CoreLib::addStaticDestructor (rt::StaticDestructFunc* destructFunc)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticDestructor (destructFunc);
}

void
CoreLib::addStaticClassDestructor (
	rt::DestructFunc* destructFunc,
	rt::IfaceHdr* iface
	)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.addStaticClassDestructor (destructFunc, iface);
}

void*
CoreLib::getTls ()
{
	rt::Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	return tls + 1;
}

void
CoreLib::dynamicThrow()
{
	rt::Runtime::dynamicThrow ();
	ASSERT (false);
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
	sl::String string;
	string.format ("%s(%d): assertion (%s) failed", fileName, line + 1, condition);
	if (message)
		string.appendFormat ("; %s", message);

	rt::Runtime::runtimeError (err::createStringError (string, string.getLength ()));
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
		rt::Runtime::runtimeError (err::getLastError ());
}

bool 
CoreLib::tryCheckDataPtrRangeIndirect (
	const void* p,
	size_t size,
	rt::DataPtrValidator* validator
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
	rt::DataPtrValidator* validator
	)
{
	bool result = tryCheckDataPtrRangeIndirect (p, size, validator);
	if (!result)
		rt::Runtime::runtimeError (err::getLastError ());
}

bool 
CoreLib::tryCheckNullPtr (
	const void* p,
	ct::TypeKind typeKind
	)
{
	if (p)
		return true;

	switch (typeKind)
	{
	case ct::TypeKind_ClassPtr:
	case ct::TypeKind_ClassRef:
		err::setStringError ("null class pointer access");
		break;

	case ct::TypeKind_FunctionPtr:
	case ct::TypeKind_FunctionRef:
		err::setStringError ("null function pointer access");
		break;

	case ct::TypeKind_PropertyPtr:
	case ct::TypeKind_PropertyRef:
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
	ct::TypeKind typeKind
	)
{
	bool result = tryCheckNullPtr (p, typeKind);
	if (!result)
		rt::Runtime::runtimeError (err::getLastError ());
}

void
CoreLib::checkStackOverflow ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->checkStackOverflow ();
}

void
CoreLib::checkDivByZero_i32 (int32_t i)
{
	if (!i)
	{
		err::setStringError ("integer division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
CoreLib::checkDivByZero_i64 (int64_t i)
{
	if (!i)
	{
		err::setStringError ("integer division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
CoreLib::checkDivByZero_f32 (float f)
{
	if (!f)
	{
		err::setStringError ("floating point division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

void
CoreLib::checkDivByZero_f64 (double f)
{
	if (!f)
	{
		err::setStringError ("floating point division by zero");
		rt::Runtime::runtimeError (err::getLastError ());
	}
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void* 
CoreLib::tryLazyGetDynamicLibFunction (
	rtl::DynamicLib* lib,
	size_t index,
	const char* name
	)
{
	ASSERT (lib->m_box->m_type->getTypeKind () == ct::TypeKind_Class);
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
CoreLib::lazyGetDynamicLibFunction (
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
CoreLib::appendFmtLiteral_a (
	rt::FmtLiteral* fmtLiteral,
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

		rt::DataPtr ptr = runtime->m_gcHeap.tryAllocateBuffer (newMaxLength + 1);
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
CoreLib::appendFmtLiteral_v (
	rt::FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	rt::Variant variant
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
			result = module->m_operatorMgr.castOperator (&value, ct::TypeKind_Int64);
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
			result = module->m_operatorMgr.castOperator (&value, ct::TypeKind_Int32);
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
		return typeKind == ct::TypeKind_Float ? 
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(float*) &variant) :
			appendFmtLiteral_f (fmtLiteral, fmtSpecifier, *(double*) &variant);
	}

	ct::Type* type;
	const void* p;

	if (typeKind != ct::TypeKind_DataRef)
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
		const char* c = (const char*) p;

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
			return appendFmtLiteral_p (fmtLiteral, fmtSpecifier, *(rt::DataPtr*) &variant);

		const char* p = *(char**) p;
		size_t length = strlen (p);

		return appendFmtLiteralStringImpl (fmtLiteral, fmtSpecifier, p, length);
	}
	else
	{
		sl::String string;
		string.format ("(variant:%s)", type->getTypeString ().cc ());
		
		return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
	}
}

size_t
CoreLib::appendFmtLiteralImpl (
	rt::FmtLiteral* fmtLiteral,
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
CoreLib::appendFmtLiteralStringImpl (
	rt::FmtLiteral* fmtLiteral,
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
CoreLib::mapAllMulticastMethods (ct::Module* module)
{
	sl::ConstList <ct::MulticastClassType> mcTypeList = module->m_typeMgr.getMulticastClassTypeList ();
	sl::Iterator <ct::MulticastClassType> mcType = mcTypeList.getHead ();
	for (; mcType; mcType++)
		mapMulticastMethods (module, *mcType);

	return true;
}

void
CoreLib::multicastDestruct (rt::Multicast* multicast)
{
	((MulticastImpl*) multicast)->destruct ();
}

void
CoreLib::multicastClear (rt::Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->clear ();
}

handle_t
CoreLib::multicastSet (
	rt::Multicast* multicast,
	rt::FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->setHandler (ptr);
}

handle_t
CoreLib::multicastSet_t (
	rt::Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->setHandler_t (p);
}

handle_t
CoreLib::multicastAdd (
	rt::Multicast* multicast,
	rt::FunctionPtr ptr
	)
{
	return ((MulticastImpl*) multicast)->addHandler (ptr);
}

handle_t
CoreLib::multicastAdd_t (
	rt::Multicast* multicast,
	void* p
	)
{
	return ((MulticastImpl*) multicast)->addHandler_t (p);
}

rt::FunctionPtr
CoreLib::multicastRemove (
	rt::Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler (handle);
}

void*
CoreLib::multicastRemove_t (
	rt::Multicast* multicast,
	handle_t handle
	)
{
	return ((MulticastImpl*) multicast)->removeHandler_t (handle);
}

rt::FunctionPtr
CoreLib::multicastGetSnapshot (rt::Multicast* multicast)
{
	return ((MulticastImpl*) multicast)->getSnapshot ();
}

void*
CoreLib::m_multicastMethodTable [ct::FunctionPtrTypeKind__Count] [ct::MulticastMethodKind__Count - 1] =
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
